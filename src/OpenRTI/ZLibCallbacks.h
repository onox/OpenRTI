/* -*-c++-*- OpenRTI - Copyright (C) 2009-2011 Mathias Froehlich
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

#ifndef OpenRTI_ZLibCallbacks_h
#define OpenRTI_ZLibCallbacks_h

#include "LogStream.h"
#include "SocketStream.h"
#include "StreamSocketReadEvent.h"
#include "StreamSocketWriteEvent.h"

#ifdef OPENRTI_HAVE_ZLIB

/// FIXME!!! Crappy implementation up to now - it just works somehow
/// FIXME: Also provide lzma compression by #include <lzma.h> when available

#include "zlib.h"

namespace OpenRTI {

// FIXME clean up these classes

class OPENRTI_LOCAL ZLibReceiveCallback : public StreamSocketReadEvent::ReceiveCallback {
public:
  ZLibReceiveCallback()
  {
    std::memset(&_zstream, 0, sizeof(_zstream));
    inflateInit(&_zstream);
  }
  virtual ~ZLibReceiveCallback()
  {
    inflateEnd(&_zstream);
  }

  virtual ssize_t recv(SocketStream& socketStream, NetworkBuffer& networkBuffer)
  {
    // Should be the same than the above while condition
    OpenRTIAssert(networkBuffer.getNumPendingBuffers());

    if (_zstream.avail_in == 0) {
      // Nothing in the input buffer, read something from the socket
      _readBuffer.clear();
      _readBuffer.addScratchBuffer().resize(64*1024);
      ssize_t ret = socketStream.recv(_readBuffer);
      if (ret == -1) {
        _readBuffer[0].resize(0);
        return -1;
      }
      if (ret == 0) {
        _readBuffer[0].resize(0);
        return 0;
      }

      // Shorten to the correct size of the read buffer
      _readBuffer[0].resize(ret);
      _zstream.next_in = reinterpret_cast<Bytef*>(_readBuffer[0].data());
      _zstream.avail_in = _readBuffer[0].size();
    }

    // here we know that we really have something read
    _zstream.next_out = reinterpret_cast<Bytef*>(networkBuffer.getPendingBuffer(0));
    _zstream.avail_out = networkBuffer.getPendingBufferSize(0);
    size_t avail_out = _zstream.avail_out;
    int flush;
    if (networkBuffer.getNumPendingBuffers() <= 1)
      flush = Z_SYNC_FLUSH;
    else
      flush = Z_NO_FLUSH;
    int ret = inflate(&_zstream, flush);
    if (ret != Z_OK) {
      Log(MessageCoding, Warning) << "Compression error!" << std::endl;
      throw MessageError("Compression error!");
    }

    return avail_out - _zstream.avail_out;
  }

private:
  NetworkBuffer _readBuffer;
  z_stream _zstream;
};

class OPENRTI_LOCAL ZLibSendCallback : public StreamSocketWriteEvent::SendCallback {
public:
  ZLibSendCallback() :
    _uncompressedSize(0),
    _maximumWritteChunk(0)
  {
    std::memset(&_zstream, 0, sizeof(_zstream));
    deflateInit(&_zstream, Z_DEFAULT_COMPRESSION);
  }
  virtual ~ZLibSendCallback()
  {
    deflateEnd(&_zstream);
  }

  ssize_t send(SocketStream& socketStream, const NetworkBuffer& networkBuffer, bool moreToSend)
  {
    if (_compressBuffer.complete()) {
      for (size_t i = 0; i < networkBuffer.getNumPendingBuffers(); ++i) {
        _zstream.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(networkBuffer.getPendingBuffer(i)));
        _zstream.avail_in = networkBuffer.getPendingBufferSize(i);

        do {
          if (_zstream.avail_out == 0) {
            // Sets _zstream.next_out and _zstream.avail_out with some buffer space
            allocCompressBuffer();
          }
          // // Bulk << "deflate 1 " << _zstream.avail_in << " " << _zstream.avail_out << std::endl;
          size_t avail_in = _zstream.avail_in;
          int ret = deflate(&_zstream, Z_NO_FLUSH);
          // if (ret == Z_BUF_ERROR) {
          //     if (0 == _zstream.avail_out)
          //       continue;
          //     else
          //       break;
          //   }
          //   if (ret != Z_OK) {
          //     Log(MessageCoding, Warning) << "Compression error!" << std::endl;
          //     throw MessageError("Compression error!");
          //   }
          _uncompressedSize += avail_in - _zstream.avail_in;
        } while (0 == _zstream.avail_out);
      }

      if (!moreToSend) {
        do {
          if (_zstream.avail_out == 0) {
            allocCompressBuffer();
          }
          int ret = deflate(&_zstream, Z_SYNC_FLUSH);
        } while (_zstream.avail_out == 0);
      }
      //   flush = ;
      // else
      //   flush = Z_NO_FLUSH;
      // int flush;
      // if (!moreToSend && networkBuffer.getNumPendingBuffers() <= i + 1)
      //   flush = Z_SYNC_FLUSH;
      // else
      //   flush = ;



      {
        VariableLengthData& body = _compressBuffer[_compressBuffer.size() - 1];
        body.resize(_zstream.total_out);
      }
    }

    ssize_t ret = socketStream.send(_compressBuffer, moreToSend);
    if (ret == -1)
      return -1;
    // Store the hugest chunk that is written at once.
    // Implement someting smart with compression in these sized block at some time
    if (_maximumWritteChunk < ret)
      _maximumWritteChunk = ret;
    _compressBuffer.processed(ret);
    // If the pending chunk is not ready so far, do not report progress
    if (!_compressBuffer.complete())
      return 0;

    // ??? FIXME
    // _zstream.total_in;
    // _zstream.total_out;

    ret = _uncompressedSize;
    _uncompressedSize = 0;
    _compressBuffer.clear();
    _zstream.next_out = 0;
    _zstream.avail_out = 0;
    return ret;
  }

private:
  void allocCompressBuffer()
  {
    size_t size = _maximumWritteChunk + 1024;
    VariableLengthData& buffer = _compressBuffer.addScratchBuffer();
    buffer.resize(size);
    _zstream.next_out = static_cast<Bytef*>(buffer.data());
    _zstream.avail_out = size;
    _zstream.total_out = 0;
  }

  NetworkBuffer _compressBuffer;
  z_stream _zstream;
  size_t _uncompressedSize;
  size_t _maximumWritteChunk;
};

} // namespace OpenRTI

#endif // OPENRTI_HAVE_ZLIB

#endif
