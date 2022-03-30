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

#pragma once

#include "VariableLengthData.h"
#include <map>
#include <set>

#ifndef __CPlusPlusStd
#error "must include OpenRTIConfig.h!"
#endif

namespace OpenRTI {

class VariableLengthDataTuple
{
  private:
    typedef std::vector<VariableLengthData> ImplType;
  public:
    typedef ImplType::size_type size_type;

    VariableLengthDataTuple() noexcept : mImpl() {}
    VariableLengthDataTuple(size_t size)  : mImpl(size) {}
    VariableLengthDataTuple(const VariableLengthDataTuple& other) : mImpl(other.mImpl) {}
    VariableLengthDataTuple(VariableLengthDataTuple&& other) : mImpl(std::move(other.mImpl)) {}
    ~VariableLengthDataTuple() noexcept = default;
    VariableLengthDataTuple& operator=(const VariableLengthDataTuple& other)
    {
      if (this != std::addressof(other))
      {
        mImpl = other.mImpl;
      }
      return *this;
    }
    VariableLengthDataTuple& operator=(VariableLengthDataTuple&& other) noexcept
    {
      if (this != std::addressof(other))
      {
        mImpl = std::move(other.mImpl);
      }
      return *this;
    }
    ImplType::iterator begin() noexcept { return mImpl.begin(); }
    ImplType::iterator end() noexcept { return mImpl.end(); }
    ImplType::const_iterator begin() const noexcept { return mImpl.begin(); }
    ImplType::const_iterator end() const noexcept { return mImpl.end(); }
    bool empty() const noexcept { return mImpl.empty(); }
    ImplType::size_type size() const noexcept { return mImpl.size(); }
    void resize(ImplType::size_type newSize) { mImpl.resize(newSize); }
    VariableLengthData& operator[](size_t index) { return mImpl[index]; }
    const VariableLengthData& operator[](size_t index) const { return mImpl[index]; }
    bool operator==(const VariableLengthDataTuple& other) const { return mImpl == other.mImpl; }
    bool operator!=(const VariableLengthDataTuple& other) const { return mImpl != other.mImpl; }
    bool operator<(const VariableLengthDataTuple& other) const { return mImpl < other.mImpl; }
    bool operator>(const VariableLengthDataTuple& other) const { return mImpl > other.mImpl; }
    bool operator<=(const VariableLengthDataTuple& other) const { return mImpl <= other.mImpl; }
    bool operator>=(const VariableLengthDataTuple& other) const { return mImpl >= other.mImpl; }

    friend std::ostream& operator<<(std::ostream& out, const VariableLengthDataTuple& vldTuple);
  private:
    std::vector<VariableLengthData> mImpl;
};

class VariableLengthDataTupleSet
{
  private:
    typedef std::vector<VariableLengthDataTuple> ImplType;
    typedef ImplType::size_type IndexType;
    // set of row indices
    typedef std::set<IndexType> IndexSet;
    // mapping of a VLD value, specific to a column, to row indices in which the value occurs
    typedef std::map<VariableLengthData, IndexSet> ValueToIndexSet;
    // type of the whole container: the vector index is the respective column index
    typedef std::vector<ValueToIndexSet> ValueToIndexSets;
    // for a given column (= index into the vector), the IndexSet (= rows) containing wildcards
    typedef std::vector<IndexSet> WildcardIndexSets;

  public:
    VariableLengthDataTupleSet() = default;
    VariableLengthDataTupleSet(const VariableLengthDataTupleSet& other) = default;
    VariableLengthDataTupleSet(VariableLengthDataTupleSet&& other) = default;
    VariableLengthDataTupleSet& operator=(const VariableLengthDataTupleSet& other) = default;
    VariableLengthDataTupleSet& operator=(VariableLengthDataTupleSet&& other) = default;

    ImplType::iterator begin() noexcept { return mImpl.begin(); }
    ImplType::iterator end() noexcept { return mImpl.end(); }
    ImplType::const_iterator begin() const noexcept { return mImpl.begin(); }
    ImplType::const_iterator end() const noexcept { return mImpl.end(); }
    bool empty() const noexcept { return mImpl.empty(); }
    ImplType::size_type size() const noexcept { return mImpl.size(); }

    const VariableLengthDataTuple& operator[](size_t index) const { return mImpl[index]; }

    // Insert a value tuple. A match for existing tuples is not done here!
    void Insert(const VariableLengthDataTuple& newTuple);
    void SetTupleSize(size_t newTupleSize);
    // Returns true if newTuple matches any existing tuple, including wildcard match
    bool FindNormalized(const VariableLengthDataTuple& newTuple) const;
    // Returns true if the tuple set already contains a tuple equal to newTuple. Just a linear search for the tuple will be done.
    bool Contains(const VariableLengthDataTuple& newTuple) const;
    bool Erase(const VariableLengthDataTuple& ref);
    friend std::ostream& operator<<(std::ostream& out, const VariableLengthDataTupleSet& tupleSet);
    friend std::ostream& operator<<(std::ostream& out, const VariableLengthDataTupleSet::IndexSet& indexSet);
  private:
    // updates the index set relating to the given column ( = columnIndex) with the new index (row) of the given value
    void AddColumnValueToIndexSets(const VariableLengthData& value, VariableLengthDataTupleSet::ImplType::size_type rowIndex, VariableLengthDataTuple::size_type columnIndex);
    void RemoveRowFromIndexSets(VariableLengthDataTupleSet::ImplType::size_type rowIndex, VariableLengthDataTuple::size_type columnIndex);
    bool FindMatchingTuples(const VariableLengthDataTuple& newTuple, VariableLengthDataTupleSet::IndexSet &matchingRows) const;
    IndexType mTupleSize = 0;
    ImplType mImpl;
    ValueToIndexSets mIndexSets;
    WildcardIndexSets mWildcardIndexSets;

};

} // namespace OpenRTI
