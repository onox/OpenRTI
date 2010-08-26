/* -*-c++-*- OpenRTI - Copyright (C) 2009-2010 Mathias Froehlich
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

#ifndef OpenRTI_NetworkBuffer_h
#define OpenRTI_NetworkBuffer_h

#include <list>
#include <vector>
#include "Types.h"
#include "VariableLengthData.h"

namespace OpenRTI {

/// A bunch of variable lenght data elements that compose a network packet.
/// The trick is that it pools all the data elements required for the whole
/// build - use - clear cycle with a minimum amount of reallocations.
class OPENRTI_API NetworkBuffer {
public:
  NetworkBuffer() :
    _dataVectorIndex(0),
    _dataVectorOffset(0)
  { }

  // The size of the buffer, this is *not* the total size in bytes
  size_t size() const
  { return _packetVector.size(); }
  bool empty() const
  { return _packetVector.empty(); }
  void clear()
  {
    _packetVector.clear();

    // FIXME: may be have some NetworkBufferState object???
    _dataVectorIndex = 0;
    _dataVectorOffset = 0;

    // Flush the active scrach buffer list
    _scratchBufferPool.splice(_scratchBufferPool.begin(), _scratchBufferList);
    // Dereference the active buffer list
    for (VariableLengthDataList::iterator i = _externalBufferList.begin(); i != _externalBufferList.end(); ++i) {
      VariableLengthData().swap(*i);
    }
    // Flush the active buffer list
    _listElementPool.splice(_listElementPool.begin(), _externalBufferList);
  }

  const VariableLengthData& operator[](size_t index) const
  { return *_packetVector[index]; }
  VariableLengthData& operator[](size_t index)
  { return *_packetVector[index]; }

  // Add a new scratch buffer element from the pool of scratch buffers.
  // Its initial size is zero.
  VariableLengthData& addScratchBuffer()
  {
    if (_scratchBufferPool.empty()) {
      _scratchBufferList.push_front(VariableLengthData());
    } else {
      _scratchBufferList.splice(_scratchBufferList.begin(), _scratchBufferPool, _scratchBufferPool.begin());
      // Note that this resize behaves like a vector resize: only the logical size is reset
      _scratchBufferList.front().resize(0);
    }
    _packetVector.push_back(_scratchBufferList.begin());
    return _scratchBufferList.front();
  }
  // Add a new buffer element, probably payload from the message
  const VariableLengthData& addBuffer(const VariableLengthData& variableLengthData)
  {
    if (_listElementPool.empty()) {
      _externalBufferList.push_front(variableLengthData);
    } else {
      _externalBufferList.splice(_externalBufferList.begin(), _listElementPool, _listElementPool.begin());
      _externalBufferList.front() = variableLengthData;
    }
    _packetVector.push_back(_externalBufferList.begin());
    return _externalBufferList.front();
  }

  bool complete() const
  {
    OpenRTIAssert(_dataVectorIndex <= _packetVector.size());
    return _dataVectorIndex == _packetVector.size();
  }

  size_t processed(size_t size)
  {
    while (_dataVectorIndex < _packetVector.size()) {
      size_t itemSize = _packetVector[_dataVectorIndex]->size();
      size_t remaining = itemSize - _dataVectorOffset;
      if (size < remaining) {
        _dataVectorOffset += size;
        return 0;
      } else {
        size -= remaining;
        _dataVectorOffset = 0;
        ++_dataVectorIndex;
      }
    }

    OpenRTIAssert(!size);
    return size;
  }

  size_t getNumPendingBuffers() const
  {
    OpenRTIAssert(_dataVectorIndex <= _packetVector.size());
    return _packetVector.size() - _dataVectorIndex;
  }
  char* getPendingBuffer(size_t index)
  {
    if (index == 0)
      return _packetVector[_dataVectorIndex]->charData(_dataVectorOffset);
    else if (index + _dataVectorIndex < _packetVector.size())
      return _packetVector[_dataVectorIndex + index]->charData();
    else
      return 0;
  }
  const char* getPendingBuffer(size_t index) const
  {
    if (index == 0)
      return _packetVector[_dataVectorIndex]->charData(_dataVectorOffset);
    else if (index + _dataVectorIndex < _packetVector.size())
      return _packetVector[_dataVectorIndex + index]->charData();
    else
      return 0;
  }
  size_t getPendingBufferSize(size_t index) const
  {
    if (index == 0)
      return _packetVector[_dataVectorIndex]->size() - _dataVectorOffset;
    else if (index + _dataVectorIndex < _packetVector.size())
      return _packetVector[_dataVectorIndex + index]->size();
    else
      return 0;
  }

private:
  typedef std::list<VariableLengthData> VariableLengthDataList;
  typedef std::vector<VariableLengthDataList::iterator> PacketVector;

  // This one describes a whole packet
  PacketVector _packetVector;

  // The lists of active buffers, the scratch buffers are held seperate than the external buffers
  VariableLengthDataList _scratchBufferList;
  VariableLengthDataList _externalBufferList;

  // Our internal pool of scratch buffers, all of them might contain some data.
  // Over time all of them contain enough data to hold the usual demanded packets
  VariableLengthDataList _scratchBufferPool;
  // Pool of empty list elements that are used for external buffers. These are dereferenced before
  // theay are put back into the pool.
  VariableLengthDataList _listElementPool;

  // The current read/write offset in the network buffer
  size_t _dataVectorIndex;
  size_t _dataVectorOffset;
};

} // namespace OpenRTI

#endif
