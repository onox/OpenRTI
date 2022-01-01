/* -*-c++-*- OpenRTI - Copyright (C) 2009-2022 Mathias Froehlich
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

#ifndef OpenRTI_AbstractMessageEncoding_h
#define OpenRTI_AbstractMessageEncoding_h

#include "AbstractConnect.h"
#include "StreamBufferProtocol.h"

namespace OpenRTI {

class OPENRTI_API AbstractMessageEncoding : public StreamBufferProtocol {
public:
  AbstractMessageEncoding();
  virtual ~AbstractMessageEncoding();

  void setConnect(const SharedPtr<AbstractConnect>& connect)
  { _connect = connect; }
  const SharedPtr<AbstractConnect>& getConnect() const
  { return _connect; }

  /// Should return the name of the encoding
  virtual const char* getName() const = 0;

  /// Still to be implemented in the actual encodings
  virtual void readPacket(const Buffer& buffer) = 0;
  virtual void writeMessage(const AbstractMessage& message) = 0;

  /// Already implemented here
  virtual bool getEnableRead() const;
  virtual void writePacket();
  virtual bool getMoreToSend() const;
  virtual void error(const Exception& e);

protected:
  SharedPtr<AbstractConnect> _connect;
};

} // namespace OpenRTI

#endif
