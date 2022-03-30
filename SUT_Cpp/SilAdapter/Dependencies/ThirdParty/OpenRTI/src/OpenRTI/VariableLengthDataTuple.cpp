/* -*-c++-*- OpenRTI - Copyright (C) 2009-2015 Mathias Froehlich
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

#include "DebugNew.h"
#include "OpenRTIConfig.h"
#include "Types.h"
#include "VariableLengthDataTuple.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <iterator>

namespace OpenRTI {

// update index sets of tuple column 'positionInTuple'. 'newIndex' is the new row index of the newly inserted tuple.
void VariableLengthDataTupleSet::AddColumnValueToIndexSets(const VariableLengthData& value, VariableLengthDataTupleSet::ImplType::size_type rowIndex, VariableLengthDataTuple::size_type columnIndex)
{
  //DebugPrintf("%s: row=%d col=%d\n", __FUNCTION__, rowIndex, columnIndex);
  auto& valueIndexSet = mIndexSets[columnIndex];
  // rows containing wildcards
  auto& wildcardIndexSet = mWildcardIndexSets[columnIndex];
  ValueToIndexSet::iterator indexSetIter = valueIndexSet.find(value);
  if (indexSetIter == valueIndexSet.end())
  {
    // new value for tuple column: insert new set
    IndexSet indexSet;
    indexSet.insert(rowIndex);
    // add rows containing wildcards to new index set - this is needed to catch the
    // wildcarded elements in the set intersection
    indexSet.insert(wildcardIndexSet.begin(), wildcardIndexSet.end());
    valueIndexSet.insert(ValueToIndexSet::value_type(value, indexSet));
    //std::cout << "insert new IndexSet: value=" << value << " indexSet=" << indexSet << std::endl;
  }
  else
  {
    // value already exists in tuple column: extend index set
    IndexSet& indexSet = indexSetIter->second;
    indexSet.insert(wildcardIndexSet.begin(), wildcardIndexSet.end());
    indexSet.insert(rowIndex);
  }
  // wildcard handling: add index of new entry to every existing value's index set
  if (value.size() == 0)
  {
    wildcardIndexSet.insert(rowIndex);
    for (auto& indexSetEntry : valueIndexSet)
    {
      IndexSet& indexSet = indexSetEntry.second;
      indexSet.insert(wildcardIndexSet.begin(), wildcardIndexSet.end());
    }
  }
}

void VariableLengthDataTupleSet::RemoveRowFromIndexSets(VariableLengthDataTupleSet::ImplType::size_type rowIndex, VariableLengthDataTuple::size_type columnIndex)
{
  //DebugPrintf("%s: row=%d col=%d\n", __FUNCTION__, rowIndex, columnIndex);
  // value => index set mapping
  ValueToIndexSet& valueIndexSet = mIndexSets[columnIndex];
  // rebuild the index set, removing the given row index and shifting all subsequent row indices one up (decrement by one)
  for (auto valueIndexSetIter = valueIndexSet.begin(); valueIndexSetIter != valueIndexSet.end(); )
  {
    auto currentEntryIter = valueIndexSetIter;
    valueIndexSetIter++;

    auto& valueIndexSetEntry = *currentEntryIter;
    IndexSet modifiedIndexSet;
    IndexSet& rows = valueIndexSetEntry.second;
    for (auto indexSetIter = rows.begin(); indexSetIter != rows.end(); indexSetIter++)
    {
      IndexType oldRowIndex = *indexSetIter;
      if (oldRowIndex < rowIndex)
      {
        modifiedIndexSet.insert(oldRowIndex);
      }
      else if (oldRowIndex > rowIndex)
      {
        modifiedIndexSet.insert(oldRowIndex-1);
      }
    }
    //std::cout << "row=" << rowIndex << " col=" << columnIndex << " old=" << valueIndexSetEntry.second << " new=" << modifiedIndexSet << std::endl;
    if (modifiedIndexSet.empty())
    {
      valueIndexSet.erase(currentEntryIter);
    }
    else
    {
      valueIndexSetEntry.second = modifiedIndexSet;
    }
  }
  // rows containing wildcards
  auto& wildcardIndexSet = mWildcardIndexSets[columnIndex];
  wildcardIndexSet.erase(rowIndex);
}

void VariableLengthDataTupleSet::Insert(const VariableLengthDataTuple& newTuple)
{
  //std::cout << "Insert " << newTuple << std::endl;
  SetTupleSize(newTuple.size());
  mImpl.push_back(newTuple);
  IndexType newRowIndex = mImpl.size() - 1;
  for (VariableLengthDataTuple::size_type columnIndex = 0; columnIndex < newTuple.size(); columnIndex++)
  {
    AddColumnValueToIndexSets(newTuple[columnIndex], newRowIndex, columnIndex);
  }
  //std::cout << *this;
  //std::cout << "Done Insert " << newTuple << std::endl;
}


void VariableLengthDataTupleSet::SetTupleSize(size_t newTupleSize)
{
  if (mTupleSize < newTupleSize)
  {
    IndexType oldTupleSize = mTupleSize;
    //std::cout << "resizing tuples from " << mTupleSize << " to " << newTuple.size() << std::endl;
    mTupleSize = newTupleSize;
    for (auto& tuple : mImpl)
    {
      tuple.resize(mTupleSize);
    }
    mIndexSets.resize(mTupleSize);
    mWildcardIndexSets.resize(mTupleSize);
    // insert all existing rows as wildcards for the newly added columns - they shall match anything future wise
    for (IndexType column = oldTupleSize; column < mTupleSize; column++)
    {
      for (IndexType row = 0; row < mImpl.size(); row++)
      {
        mWildcardIndexSets[column].insert(row);
      }
    }
  }
}

bool VariableLengthDataTupleSet::FindMatchingTuples(const VariableLengthDataTuple& normalizedValueTuple, VariableLengthDataTupleSet::IndexSet &matchingRows) const
{
  //std::cout << "FindNormalized " << valueTuple << std::endl;
  std::list<IndexSet> resultIndexSets;
  if (normalizedValueTuple.size() != mTupleSize) return false;

  // step 1: for each column, build sets of matched row indices, and include these index sets to a list.
  for (VariableLengthDataTuple::size_type column = 0; column < mTupleSize; column++)
  {
    const auto& value = normalizedValueTuple[column];
    if (value.empty()) // given column value is a wildcard: do not include column in list of index sets to be intersected later
      continue;
    const ValueToIndexSet& valueToIndexSet = mIndexSets[column];
    auto indexSetIter = valueToIndexSet.find(value);
    if (indexSetIter != valueToIndexSet.end())
    {
      // add found index set to list of sets to be intersected
      //std::cout << "column " << column << " value " << value << " => " << indexSetIter->second << std::endl;
      resultIndexSets.push_back(indexSetIter->second);
    }
    else
    {
      resultIndexSets.push_back(mWildcardIndexSets[column]);
    }
  }
  // step 2: build intersection of all index sets. The resulting set is the set of rows matching the value tuple.
  if (resultIndexSets.empty())
  {
    // trivial case: all value columns were wildcards ==> all rows match
    matchingRows.clear();
    for (IndexType i=0;i<mImpl.size();i++)
    {
      matchingRows.insert(i);
    }
    return true;
  }
  IndexSet intersectionSet = resultIndexSets.front();
  resultIndexSets.pop_front();
  for (auto& indexSet : resultIndexSets)
  {
    IndexSet intersectionResult;
    //std::cout << intersectionSet << " X " << indexSet << "=";
    set_intersection(intersectionSet.begin(), intersectionSet.end(), indexSet.begin(), indexSet.end(), std::inserter(intersectionResult, intersectionResult.begin()));
    //std::cout << intersectionResult << std::endl;
    intersectionSet = intersectionResult;
  }
  //std::cout << "intersection set=" << intersectionSet << std::endl;
  matchingRows = intersectionSet;
  return !intersectionSet.empty();
}

bool VariableLengthDataTupleSet::FindNormalized(const VariableLengthDataTuple& valueTuple) const
{
  //std::cout << "FindNormalized " << valueTuple << std::endl;
  if (valueTuple.size() != mTupleSize) return false;

  // step 1: for each column, build sets of matched row indices, and include these index sets to a list.
  std::list<IndexSet> resultIndexSets;
  for (VariableLengthDataTuple::size_type column = 0; column < mTupleSize; column++)
  {
    const auto& value = valueTuple[column];
    if (value.empty()) // given column value is a wildcard
      continue;
    const ValueToIndexSet& valueToIndexSet = mIndexSets[column];
    auto indexSetIter = valueToIndexSet.find(value);
    if (indexSetIter != valueToIndexSet.end())
    {
      // add found index set to list of sets to be intersected
      //std::cout << "column " << column << " value " << value << " => " << indexSetIter->second << std::endl;
      resultIndexSets.push_back(indexSetIter->second);
    }
    else
    {
      resultIndexSets.push_back(mWildcardIndexSets[column]);
    }
  }
  // step 2: build intersection of all index sets. The resulting set is the set of rows matching the value tuple.
  if (resultIndexSets.empty())
  {
    // trivial case: all value columns were wildcards
    return true;
  }
  IndexSet intersectionSet = resultIndexSets.front();
  resultIndexSets.pop_front();
  for (auto& indexSet : resultIndexSets)
  {
    IndexSet intersectionResult;
    //std::cout << intersectionSet << " X " << indexSet << "=";
    set_intersection(intersectionSet.begin(), intersectionSet.end(), indexSet.begin(), indexSet.end(), std::inserter(intersectionResult, intersectionResult.begin()));
    //std::cout << intersectionResult << std::endl;
    intersectionSet = intersectionResult;
  }
  //std::cout << "intersection set=" << intersectionSet << std::endl;
  return !intersectionSet.empty();
}

bool VariableLengthDataTupleSet::Contains(const VariableLengthDataTuple& newTuple) const
{
  for (auto& tuple : mImpl)
  {
    if (tuple == newTuple)
    {
      return true;
    }
  }
  return false;
}


bool VariableLengthDataTupleSet::Erase(const VariableLengthDataTuple& ref)
{
  IndexType rowIndex = 0;
  for (auto iter = mImpl.begin(); iter != mImpl.end(); rowIndex++, iter++)
  {
    if (*iter == ref)
    {
      mImpl.erase(iter);
      for (IndexType columnIndex = 0; columnIndex < ref.size(); columnIndex++)
      {
        RemoveRowFromIndexSets(rowIndex, columnIndex);
      }
      return true;
    }
  }
  return false;
}

std::ostream& operator<<(std::ostream& out, const VariableLengthDataTuple& vldTuple)
{
  out << "(";
  auto iter = vldTuple.begin();
  auto end = vldTuple.end();
  while (iter != end)
  {
    out << *iter;
    if (++iter != end) out << ", ";
  }
  out << ")";
  return out;
}


std::ostream& operator<<(std::ostream& out, const VariableLengthDataTupleSet::IndexSet& indexSet)
{
  out << "[";
  auto iter = indexSet.begin();
  auto end = indexSet.end();
  while (iter != end)
  {
    out << *iter;
    if (++iter != end) out << ", ";
  }
  out << "]";
  return out;
}

std::ostream& operator<<(std::ostream& out, const VariableLengthDataTupleSet& tupleSet)
{
  out << tupleSet.mImpl.size() << " tuples:" << std::endl;
  VariableLengthDataTupleSet::IndexType tupleIndex = 0;
  for (auto& tuple : tupleSet.mImpl)
  {
    out << tupleIndex++ << " : " << tuple << std::endl;
  }
  for (VariableLengthDataTuple::size_type positionInTuple = 0; positionInTuple < tupleSet.mIndexSets.size(); positionInTuple++)
  {
    out << "IndexSet " << positionInTuple << ": ";
    const VariableLengthDataTupleSet::ValueToIndexSet& valueToIndexSet = tupleSet.mIndexSets[positionInTuple];
    for (const auto& valueToIndexSetEntry : valueToIndexSet)
    {
      const auto& value = valueToIndexSetEntry.first;
      const auto& indexSet = valueToIndexSetEntry.second;
      out << "value " << value << " => indexSet " << indexSet << std::endl;
    }
    out << "WildcardIndexSet " << positionInTuple << ": ";
    const VariableLengthDataTupleSet::IndexSet& wildcardIndexSet = tupleSet.mWildcardIndexSets[positionInTuple];
    out << wildcardIndexSet << std::endl;
  }
  return out;
}


} // namespace OpenRTI
