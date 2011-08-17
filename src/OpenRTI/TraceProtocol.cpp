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

#include "TraceProtocol.h"

#include <fstream>
#include <iostream>

#include "AbstractMessage.h"
#include "Clock.h"
#include "Message.h"
#include "ProtocolRegistry.h"
#include "StringUtils.h"

namespace OpenRTI {

class OPENRTI_LOCAL Stream : public Referenced {
public:
  Stream(const std::string& file) : _stream(utf8ToLocale(file).c_str()) {}
  virtual ~Stream() {}
  std::ofstream _stream;
};

class OPENRTI_LOCAL TraceProtocol::MessageSender : public AbstractMessageSender {
public:
  MessageSender(const SharedPtr<AbstractConnect>& connect, const SharedPtr<Stream>& stream) :
    _connect(connect),
    _stream(stream)
  {
  }
  virtual ~MessageSender()
  {
    close();
  }
  virtual void send(const SharedPtr<AbstractMessage>& message)
  {
    if (!_connect.valid())
        throw RTIinternalError("Trying to send message to a closed MessageSender");
    _stream->_stream << Clock::now() << ": out: " << *message << std::endl;
    _connect->send(message);
  }
  virtual void close()
  {
    if (!_connect.valid())
      return;
    _connect->getMessageSender()->close();
    _connect = 0;
  }
private:
  SharedPtr<AbstractConnect> _connect;
  SharedPtr<Stream> _stream;
};

class OPENRTI_LOCAL TraceProtocol::MessageReceiver : public AbstractMessageReceiver {
public:
  MessageReceiver(const SharedPtr<AbstractConnect>& connect, const SharedPtr<Stream>& stream) :
    _connect(connect),
    _stream(stream)
  {
  }
  virtual SharedPtr<AbstractMessage> receive(const Clock& timeout)
  {
    SharedPtr<AbstractMessage> message = _connect->receive(timeout);
    _stream->_stream << Clock::now() << ": in:  " << *message << std::endl;
    return message;
  }
  virtual bool isOpen() const
  {
    return _connect->getMessageReceiver()->isOpen();
  }
  virtual bool empty() const
  {
    return _connect->getMessageReceiver()->empty();
  }

private:
  SharedPtr<AbstractConnect> _connect;
  SharedPtr<Stream> _stream;
};

class OPENRTI_LOCAL TraceProtocol::Connect : public AbstractConnect {
public:
  Connect(const SharedPtr<AbstractConnect>& connect, const SharedPtr<Stream>& stream) :
    _messageSender(new MessageSender(connect, stream)),
    _messageReceiver(new MessageReceiver(connect, stream))
  {
  }
  virtual AbstractMessageSender* getMessageSender()
  { return _messageSender.get(); }
  virtual AbstractMessageReceiver* getMessageReceiver()
  { return _messageReceiver.get(); }

private:
  SharedPtr<AbstractMessageSender> _messageSender;
  SharedPtr<AbstractMessageReceiver> _messageReceiver;
  SharedPtr<Stream> _stream;
};

SharedPtr<AbstractConnect>
TraceProtocol::connect(const StringStringListMap& parameterMap, const Clock& abstime) const
{
  StringStringListMap::const_iterator i = parameterMap.find("traceProtocol");
  if (i == parameterMap.end())
    return 0;
  if (i->second.size() != 1)
    return 0;

  SharedPtr<const AbstractProtocol> protocol = ProtocolRegistry::instance()->getProtocol(i->second.front());
  if (!protocol.valid())
    return 0;

  SharedPtr<AbstractConnect> connect = protocol->connect(parameterMap, abstime);
  if (!connect.valid())
    return 0;

  std::string traceFile = "trace.txt";
  StringStringListMap::const_iterator j = parameterMap.find("traceFile");
  if (j != parameterMap.end() && !j->second.empty())
    traceFile = j->second.front();

  SharedPtr<Stream> stream = new Stream(traceFile);
  return new Connect(connect, stream);
}

} // namespace OpenRTI
