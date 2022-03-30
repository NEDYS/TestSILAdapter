/* -*-c++-*- OpenRTI - Copyright (C) 2009-2013 Mathias Froehlich
 *
 * This file is part of OpenRTI.
 *
 * OpenRTI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * OpenRTI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with OpenRTI.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef OpenRTI_FederateHandleLowerBoundTimeStampMap_h
#define OpenRTI_FederateHandleLowerBoundTimeStampMap_h

#include <map>

#include "Handle.h"
#include "LogStream.h"

namespace OpenRTI {

template<typename T>
class OPENRTI_LOCAL FederateHandleLowerBoundTimeStampMap {
public:
  typedef T LogicalTime;
  typedef std::pair<LogicalTime, int> LogicalTimePair;

  /// Insert a new federate handle with the initial logical time pair
  void insert(const FederateHandle& federateHandle, const LogicalTime& logicalTime, const LogicalTime& nextMessageTime, const Unsigned& commitId, const Unsigned& beforeOwnCommitId)
  {
    ScopeLock lock(_mutex);
    // Store that in the time map, so that we cannot advance more than that until this federate
    // commits a new lbts in the rti. This will usually happen in immediate response to that sent message.
    typename LogicalTimeFederateCountMap::iterator i = _timeAdvanceLogicalTimeFederateCountMap.insert(logicalTime);
    typename LogicalTimeFederateCountMap::iterator j = _nextMessageLogicalTimeFederateCountMap.insert(nextMessageTime);
    OpenRTIAssert(_federateHandleCommitMap.find(federateHandle) == _federateHandleCommitMap.end());
    _federateHandleCommitMap.insert(typename FederateHandleCommitMap::value_type(federateHandle, Commit(i, j, commitId, beforeOwnCommitId)));
  }

  /// Erase a federate handle including its logical time bounds
  /// Returns true if this unleaches a new logical time
  bool erase(const FederateHandle& federateHandle)
  {
    ScopeLock lock(_mutex);
    // find the (last) commit done by the given federate
    typename FederateHandleCommitMap::iterator i;
    i = _federateHandleCommitMap.find(federateHandle);
    // OpenRTIAssert(i != _federateHandleCommitMap.end());
    if (i == _federateHandleCommitMap.end())
      return false;

    typename LogicalTimeFederateCountMap::iterator j = i->second._timeAdvanceCommit;
    typename LogicalTimeFederateCountMap::iterator k = i->second._nextMessageCommit;
    _federateHandleCommitMap.erase(i);
    bool isFirstLogicalTime;
    isFirstLogicalTime = _timeAdvanceLogicalTimeFederateCountMap.erase(j);
    isFirstLogicalTime = _nextMessageLogicalTimeFederateCountMap.erase(k) || isFirstLogicalTime;
    return isFirstLogicalTime;
  }

  /// Returns true if this unleaches a new logical time
  std::pair<bool, bool> commit(const FederateHandle& federateHandle, const LogicalTime& logicalTime, const LowerBoundTimeStampCommitType& commitType, const Unsigned& commitId)
  {
    ScopeLock lock(_mutex);
    // Once we receive these commits, the federate must have registered as some regulating, this must be here
    OpenRTIAssert(!_timeAdvanceLogicalTimeFederateCountMap.empty());
    OpenRTIAssert(!_nextMessageLogicalTimeFederateCountMap.empty());
    OpenRTIAssert(!_federateHandleCommitMap.empty());

    // find the (last) commit done by the given federate
    // Note each federate has exactly one commit entry.
    typename FederateHandleCommitMap::iterator i;
    i = _federateHandleCommitMap.find(federateHandle);
    OpenRTIAssert(i != _federateHandleCommitMap.end());
    Commit& commit = i->second;

    // The iterators contained in the commit entry point into the time advance count map, or the next message commit map, or both.
    bool isFirstLogicalTime;
    if (commitType & TimeAdvanceCommit) {
      // 'move' this federate's count in the _timeAdvanceLogicalTimeFederateCountMap
      std::pair<typename LogicalTimeFederateCountMap::iterator, bool> iteratorPair = _timeAdvanceLogicalTimeFederateCountMap.move(commit._timeAdvanceCommit, logicalTime);
      commit._timeAdvanceCommit = iteratorPair.first;
      isFirstLogicalTime = iteratorPair.second;
    } else {
      isFirstLogicalTime = false;
    }

    if (commitType & NextMessageCommit) {
      commit._nextMessageCommit = _nextMessageLogicalTimeFederateCountMap.move(commit._nextMessageCommit, logicalTime).first;
    }

    // Forcefully clear this
#ifndef EXPERIMENTAL_NO_LOCKED_BY_NEXT_MESSAGE
    bool nextMessageMode = commit.isInNextMessageMode();
    if (!nextMessageMode)
      commit._federateIsLocked = false;
#endif

    bool commmitIdChangedAndNextMessageMode;
    if (commit._commitId != commitId) {
#ifndef EXPERIMENTAL_NO_LOCKED_BY_NEXT_MESSAGE
      OpenRTIAssert(nextMessageMode);
#endif
      commit._commitId = commitId;
      commmitIdChangedAndNextMessageMode = true;
    } else {
      commmitIdChangedAndNextMessageMode = false;
    }

    OpenRTIAssert(!_timeAdvanceLogicalTimeFederateCountMap.empty());
    OpenRTIAssert(!_nextMessageLogicalTimeFederateCountMap.empty());
    OpenRTIAssert(!_federateHandleCommitMap.empty());
    OpenRTIAssert(commit._timeAdvanceCommit->first <= commit._nextMessageCommit->first);
    OpenRTIAssert(_timeAdvanceLogicalTimeFederateCountMap.begin()->first <= _nextMessageLogicalTimeFederateCountMap.begin()->first);

    return std::pair<bool, bool>(isFirstLogicalTime, commmitIdChangedAndNextMessageMode);
  }

  // O(1)
  bool canAdvanceTo(const LogicalTimePair& logicalTimePair) const
  {
    ScopeLock lock(_mutex);
    if (_empty())
      return true;
    if (0 < logicalTimePair.second)
      return logicalTimePair.first < _timeAdvanceLogicalTimeFederateCountMap.begin()->first;
    else
      return logicalTimePair.first <= _timeAdvanceLogicalTimeFederateCountMap.begin()->first;
  }

  // O(1)
  bool canAdvanceToNextMessage(const LogicalTimePair& logicalTimePair) const
  {
    ScopeLock lock(_mutex);
    if (_empty())
      return true;
    if (0 < logicalTimePair.second)
      return logicalTimePair.first < _nextMessageLogicalTimeFederateCountMap.begin()->first;
    else
      return logicalTimePair.first <= _nextMessageLogicalTimeFederateCountMap.begin()->first;
  }

  // O(1)
  bool empty() const
  {
    ScopeLock lock(_mutex);
    OpenRTIAssert(_timeAdvanceLogicalTimeFederateCountMap.empty() == _federateHandleCommitMap.empty());
    OpenRTIAssert(_nextMessageLogicalTimeFederateCountMap.empty() == _federateHandleCommitMap.empty());
    return _timeAdvanceLogicalTimeFederateCountMap.empty();
  }

  // O(1)
  const LogicalTime& getGALT() const
  {
    ScopeLock lock(_mutex);
    OpenRTIAssert(!_timeAdvanceLogicalTimeFederateCountMap.empty());
    OpenRTIAssert(!_nextMessageLogicalTimeFederateCountMap.empty());
    OpenRTIAssert(!_federateHandleCommitMap.empty());
    return _timeAdvanceLogicalTimeFederateCountMap.begin()->first;
  }

  // O(1)
  const LogicalTime& getNextMessageGALT() const
  {
    ScopeLock lock(_mutex);
    OpenRTIAssert(!_timeAdvanceLogicalTimeFederateCountMap.empty());
    OpenRTIAssert(!_nextMessageLogicalTimeFederateCountMap.empty());
    OpenRTIAssert(!_federateHandleCommitMap.empty());
    return _nextMessageLogicalTimeFederateCountMap.begin()->first;
  }

  // O(log(n))
  bool getFederateIsInNextMessageModeForAssert(const FederateHandle& federateHandle) const
  {
    ScopeLock lock(_mutex);
    typename FederateHandleCommitMap::const_iterator i = _federateHandleCommitMap.find(federateHandle);
    if (i == _federateHandleCommitMap.end())
      return false;
    return i->second.isInNextMessageMode();
  }

  // O(log(n))
#ifndef EXPERIMENTAL_NO_LBTS_RESPONSE_MESSAGE
  void setFederateWaitCommitId(const FederateHandle& federateHandle, const Unsigned& commitId)
  {
    ScopeLock lock(_mutex);
    typename FederateHandleCommitMap::iterator i = _federateHandleCommitMap.find(federateHandle);
    if (i == _federateHandleCommitMap.end())
      return;
    i->second._federateIsWaitingForCommitId = commitId;
  }
#endif

// O(log(n))
#ifndef EXPERIMENTAL_NO_LOCKED_BY_NEXT_MESSAGE
  void setFederateLockedByNextMessage(const FederateHandle& federateHandle, bool locked)
  {
    ScopeLock lock(_mutex);
    typename FederateHandleCommitMap::iterator i = _federateHandleCommitMap.find(federateHandle);
    if (i == _federateHandleCommitMap.end())
      return;
    i->second._federateIsLocked = locked;
  }
#endif

  // O(n)
#ifndef EXPERIMENTAL_NO_LOCKED_BY_NEXT_MESSAGE
  bool getLockedByNextMessage(const Unsigned& commitId) const
  {
    ScopeLock lock(_mutex);
    if (!getConstrainedByNextMessage())
      return false;
    // Hmm, can we work on counts to summarize that
    for (typename FederateHandleCommitMap::const_iterator i = _federateHandleCommitMap.begin();
         i != _federateHandleCommitMap.end(); ++i) {
      if (_nextMessageLogicalTimeFederateCountMap.begin()->first <= i->second._timeAdvanceCommit->first)
        continue;
      if (!i->second.isInNextMessageMode())
        continue;
      if (i->second._federateIsWaitingForCommitId != commitId)
        return false;
    }
    return true;
  }
#endif

  // O(n)
  bool getIsSaveToAdvanceToNextMessage(const Unsigned& commitId) const
  {
    ScopeLock lock(_mutex);
    if (!getConstrainedByNextMessage())
      return false;
    // Hmm, can we work on counts to summarize that
    for (typename FederateHandleCommitMap::const_iterator i = _federateHandleCommitMap.begin();
         i != _federateHandleCommitMap.end(); ++i) {
      if (_nextMessageLogicalTimeFederateCountMap.begin()->first <= i->second._timeAdvanceCommit->first)
        continue;
      if (!i->second.isInNextMessageMode())
        continue;
      if (i->second._federateIsWaitingForCommitId != commitId)
        return false;
      if (!i->second._federateIsLocked)
        return false;
    }
    return true;
  }

  // O(n)
  // return list of federates (with commits) currently executing NMR
  void getNextMessageFederateHandleList(std::list<std::pair<FederateHandle, Unsigned> >& federateHandleCommitIdList)
  {
    ScopeLock lock(_mutex);
    for (typename FederateHandleCommitMap::const_iterator i = _federateHandleCommitMap.begin();
         i != _federateHandleCommitMap.end(); ++i) {
      if (!i->second.isInNextMessageMode())
        continue;
      federateHandleCommitIdList.push_back(std::pair<FederateHandle, Unsigned>(i->first, i->second._commitId));
    }
  }

private:
  // O(1)
  // Is there a TAR pending, with a smaller time than any NMR? Has priority.
  bool getConstrainedByNextMessage() const
  {
    if (_empty())
      return false;
    return _timeAdvanceLogicalTimeFederateCountMap.begin()->first < _nextMessageLogicalTimeFederateCountMap.begin()->first;
  }
  bool _empty() const
  {
    OpenRTIAssert(_timeAdvanceLogicalTimeFederateCountMap.empty() == _federateHandleCommitMap.empty());
    OpenRTIAssert(_nextMessageLogicalTimeFederateCountMap.empty() == _federateHandleCommitMap.empty());
    return _timeAdvanceLogicalTimeFederateCountMap.empty();
  }

  mutable Mutex _mutex;

  typedef FederateHandle::value_type FederateCountType;

  // Mapping from LogicalTimes to number of federates who committed that logical time.
  // Entries where the count drops to zero get removed.
  // The front of this list is the minimum time, which some (but maybe not all federates) committed to.
  // This central data structure actually implements the conservative approach in simulation time synchronization,
  // and is used to actually find out whether this federate's time may advance to a certain point.
  class OPENRTI_LOCAL LogicalTimeFederateCountMap {
  public:
    typedef typename std::map<LogicalTime, FederateCountType>::iterator iterator;
    typedef typename std::map<LogicalTime, FederateCountType>::const_iterator const_iterator;
    typedef typename std::map<LogicalTime, FederateCountType>::value_type value_type;

    bool empty() const
    { return _countMap.empty(); }

    iterator begin()
    { return _countMap.begin(); }
    const_iterator begin() const
    { return _countMap.begin(); }

    iterator end()
    { return _countMap.end(); }
    const_iterator end() const
    { return _countMap.end(); }

    iterator find(const LogicalTime& logicalTime)
    { return _countMap.find(logicalTime); }
    const_iterator find(const LogicalTime& logicalTime) const
    { return _countMap.find(logicalTime); }

    iterator insert(const LogicalTime& logicalTime)
    {
      // Only inserts if the entry is new
      iterator i = _countMap.insert(value_type(logicalTime, 0)).first;
      // Increment the reference count for this entry
      ++(i->second);
      return i;
    }

    // 'Move' a count pointing to oldEntryIter to another logical time.
    // The logical time pointed to by oldEntryIter may get removed, if it's count drops to zero.
    std::pair<iterator, bool> move(iterator oldEntryIter, const LogicalTime& logicalTime)
    {
      // Only inserts if the entry is new
      iterator newEntryIter = _countMap.insert(value_type(logicalTime, 0)).first;
      // If we get back the same iterator, we must have gotten the same entry again
      if (newEntryIter == oldEntryIter)
        return std::pair<iterator, bool>(oldEntryIter, false);

      // Change the reference count on the new entry
      ++(newEntryIter->second);

      // And if at the old entry the reference count does drop to zero ...
      if (0 != --(oldEntryIter->second))
        return std::pair<iterator, bool>(newEntryIter, false); // old entry remains

      // 0 == oldEntryIter->second:
      // ... see if this unleaches a new time ...
      bool isFirstLogicalTime = (oldEntryIter == _countMap.begin());

      // .. and finally erase this entry.
      _countMap.erase(oldEntryIter);

      return std::pair<iterator, bool>(newEntryIter, isFirstLogicalTime);
    }

    bool erase(iterator i)
    {
      if (0 != --(i->second))
        return false;
      bool isFirstLogicalTime = (i == _countMap.begin());
      _countMap.erase(i);
      return isFirstLogicalTime;
    }

  private:
    std::map<LogicalTime, FederateCountType> _countMap;
  };

  LogicalTimeFederateCountMap _timeAdvanceLogicalTimeFederateCountMap;
  LogicalTimeFederateCountMap _nextMessageLogicalTimeFederateCountMap;

  /// Hmm, want a list of federates to check that are in next event mode. So that we do not need to check everything in turn?!

  typedef typename LogicalTimeFederateCountMap::iterator LogicalTimeMapIterator;

  struct Commit {
    Commit(const LogicalTimeMapIterator& timeAdvanceCommit, const LogicalTimeMapIterator& nextMessageCommit, Unsigned commitId, Unsigned beforeOwnCommitId)
      : _timeAdvanceCommit(timeAdvanceCommit)
      , _nextMessageCommit(nextMessageCommit)
      , _commitId(commitId)
#ifndef EXPERIMENTAL_NO_LBTS_RESPONSE_MESSAGE
      , _federateIsWaitingForCommitId(beforeOwnCommitId)
#endif
#ifndef EXPERIMENTAL_NO_LOCKED_BY_NEXT_MESSAGE
      , _federateIsLocked(false)
#endif
    {
      OpenRTIAssert(_timeAdvanceCommit->first <= _nextMessageCommit->first);
    }

    // if there are entries in the nextMessageCommit map with lower times than in the time advance commit map,
    // s.o. is still processing nextMessage requests.
    bool isInNextMessageMode() const
    {
      OpenRTIAssert(_timeAdvanceCommit->first <= _nextMessageCommit->first);
      return _timeAdvanceCommit->first != _nextMessageCommit->first;
    }
    // Points to the logical time sorted map of time advance commits
    LogicalTimeMapIterator _timeAdvanceCommit;
    // Points to the logical time sorted map of next message commits
    LogicalTimeMapIterator _nextMessageCommit;
    // The commit id of this federate we have seen last.
    Unsigned _commitId;
    // This compares against our own commit id, if they are the same we know that the appropriate federate has received our current commit
#ifndef EXPERIMENTAL_NO_LBTS_RESPONSE_MESSAGE
    Unsigned _federateIsWaitingForCommitId;
#endif
    // Tells us if the federate is locked by next message request
#ifndef EXPERIMENTAL_NO_LOCKED_BY_NEXT_MESSAGE
    bool _federateIsLocked;
#endif
  };
  // there's exactly one commit per federate
  typedef std::map<FederateHandle, Commit> FederateHandleCommitMap;
  FederateHandleCommitMap _federateHandleCommitMap;
};

} // namespace OpenRTI

#endif
