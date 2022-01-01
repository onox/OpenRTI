/* -*-c++-*- OpenRTI - Copyright (C) 2004-2022 Mathias Froehlich
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

#include "ExpatXMLReader.h"

#include <istream>
#include <cstring>
#include <expat.h>
#include "Exception.h"

#if XML_MAJOR_VERSION < 2
# define XML_StopParser(a, b) do {} while (0)
#endif

namespace OpenRTI {
namespace XML {

class ExpatXMLAttributes : public Attributes {
public:
  ExpatXMLAttributes(const char**atts);
  virtual int getIndex(const char* qName) const;
  virtual int getLength(void) const;
  virtual const char* getLocalName(int index) const;
  virtual Type getType(int) const;
  virtual Type getType(const char*) const;
  virtual const char* getValue(int index) const;
  virtual const char* getValue(const char* qName) const;
private:
  int mLength;
  const char** mAtts;
};

struct UserData {
  XML_Parser parser;
  ExpatXMLReader* reader;
  std::string exceptionString;
};

static inline ContentHandler*
userDataToContentHandler(void* userData)
{
  ExpatXMLReader* reader = reinterpret_cast<UserData*>(userData)->reader;
  if (!reader)
    return 0;
  ContentHandler* contentHandler = reader->getContentHandler();
  return contentHandler;
}

static inline XML_Parser
userDataToParser(void* userData)
{
  return reinterpret_cast<UserData*>(userData)->parser;
}

static inline void
userDataSetException(void* userData, const std::string& exceptionString)
{
  reinterpret_cast<UserData*>(userData)->exceptionString = exceptionString;
}

ExpatXMLAttributes::ExpatXMLAttributes(const char ** atts) :
  mLength(0), mAtts(atts)
{
  for (unsigned i = 0; mAtts[i] != 0; i += 2)
    ++mLength;
}

int
ExpatXMLAttributes::getIndex(const char* qName) const
{
  for (unsigned i = 0; mAtts[2*i] != 0; ++i) {
    if (strcmp(mAtts[2*i], qName) == 0)
      return i;
  }
  return -1;
}

int
ExpatXMLAttributes::getLength(void) const
{
  return mLength;
}

const char*
ExpatXMLAttributes::getLocalName(int index) const
{
  return (index < mLength) ? mAtts[index*2] : 0;
}

Type
ExpatXMLAttributes::getType(int) const
{
  return CDATA; /*FIXME*/
}

Type
ExpatXMLAttributes::getType(const char*) const
{
  return CDATA; /*FIXME*/
}

const char*
ExpatXMLAttributes::getValue(int index) const
{
  return (0 <= index && index < mLength) ? mAtts[index*2+1] : 0;
}

const char*
ExpatXMLAttributes::getValue(const char* qName) const
{
  for (unsigned i = 0; mAtts[i] != 0; i += 2) {
    if (strcmp(mAtts[i], qName) == 0)
      return mAtts[i+1];
  }
  return 0;
}

static void
ExpatStartElement(void* userData, const char* name, const char** atts)
{
  try {
    ContentHandler* contentHandler = userDataToContentHandler(userData);
    ExpatXMLAttributes eAtts(atts);
    if (contentHandler)
      contentHandler->startElement("", name, name, &eAtts);
  } catch (Exception& e) {
    userDataSetException(userData, e.getReason());
    XML_StopParser(userDataToParser(userData), XML_FALSE);
  } catch (...) {
    XML_StopParser(userDataToParser(userData), XML_FALSE);
  }
}

static void
ExpatEndElement(void* userData, const char* name)
{
  try {
    ContentHandler* contentHandler = userDataToContentHandler(userData);
    if (contentHandler)
      contentHandler->endElement("", name, name);
  } catch (Exception& e) {
    userDataSetException(userData, e.getReason());
    XML_StopParser(userDataToParser(userData), XML_FALSE);
  } catch (...) {
    XML_StopParser(userDataToParser(userData), XML_FALSE);
  }
}

static void
ExpatCharacterData(void* userData, const char* data, int length)
{
  try {
    ContentHandler* contentHandler = userDataToContentHandler(userData);
    if (contentHandler)
      contentHandler->characters(data, length);
  } catch (Exception& e) {
    userDataSetException(userData, e.getReason());
    XML_StopParser(userDataToParser(userData), XML_FALSE);
  } catch (...) {
    XML_StopParser(userDataToParser(userData), XML_FALSE);
  }
}

static void
ExpatComment(void* userData, const char* data)
{
  try {
    ContentHandler* contentHandler = userDataToContentHandler(userData);
    if (contentHandler)
      contentHandler->comment(data, unsigned(strlen(data)));
  } catch (Exception& e) {
    userDataSetException(userData, e.getReason());
    XML_StopParser(userDataToParser(userData), XML_FALSE);
  } catch (...) {
    XML_StopParser(userDataToParser(userData), XML_FALSE);
  }
}

static void
ExpatProcessingInstructions(void* userData, const char* target, const char* data)
{
  try {
    ContentHandler* contentHandler = userDataToContentHandler(userData);
    if (contentHandler)
      contentHandler->processingInstruction(target, data);
  } catch (Exception& e) {
    userDataSetException(userData, e.getReason());
    XML_StopParser(userDataToParser(userData), XML_FALSE);
  } catch (...) {
    XML_StopParser(userDataToParser(userData), XML_FALSE);
  }
}

ExpatXMLReader::ExpatXMLReader(void)
{
}

ExpatXMLReader::~ExpatXMLReader(void)
{
}

void
ExpatXMLReader::parse(std::istream& stream, const std::string& encoding)
{
  UserData userData;
  userData.reader = this;
//   XML_Parser parser = XML_ParserCreateNS(0, ':');
  if (encoding.empty())
    userData.parser = XML_ParserCreate(0);
  else
    userData.parser = XML_ParserCreate(encoding.c_str());
  XML_SetUserData(userData.parser, &userData);
  XML_SetElementHandler(userData.parser, ExpatStartElement, ExpatEndElement);
  XML_SetCharacterDataHandler(userData.parser, ExpatCharacterData);
  XML_SetProcessingInstructionHandler(userData.parser, ExpatProcessingInstructions);
  XML_SetCommentHandler(userData.parser, ExpatComment);

  if (mContentHandler.valid())
    mContentHandler->startDocument();

  unsigned bufSize = 32*1024;
  char* buf = new char[bufSize];
  while (!stream.eof()) {
    if (!stream.good()) {
      if (mErrorHandler.valid())
        mErrorHandler->fatalError("ExpatXMLReader: "
                                  "Can not read from input stream",
                                  XML_GetCurrentLineNumber(userData.parser),
                                  XML_GetCurrentColumnNumber(userData.parser));
      XML_ParserFree(userData.parser);
      delete [] buf;
      return;
    }

    stream.read(buf, bufSize);
    if (!XML_Parse(userData.parser, buf, int(stream.gcount()), 0)) {
      if (mErrorHandler.valid()) {
        std::string errorString("XML error: ");
        const char* e = XML_ErrorString(XML_GetErrorCode(userData.parser));
        if (e)
          errorString.append(std::string(e) + ": ");
        errorString.append(userData.exceptionString);
        mErrorHandler->fatalError(errorString.c_str(),
                                  XML_GetCurrentLineNumber(userData.parser),
                                  XML_GetCurrentColumnNumber(userData.parser));
      }
      XML_ParserFree(userData.parser);
      delete [] buf;
      return;
    }
  }

  if (!XML_Parse(userData.parser, buf, 0, 1)) {
    if (mErrorHandler.valid()) {
      std::string errorString("XML error: ");
      const char* e = XML_ErrorString(XML_GetErrorCode(userData.parser));
      if (e)
        errorString.append(std::string(e) + ": ");
      errorString.append(userData.exceptionString);
      mErrorHandler->fatalError(errorString.c_str(),
                                XML_GetCurrentLineNumber(userData.parser),
                                XML_GetCurrentColumnNumber(userData.parser));
    }
    XML_ParserFree(userData.parser);
    delete [] buf;
    return;
  }

  XML_ParserFree(userData.parser);
  delete [] buf;

  if (mContentHandler.valid())
    mContentHandler->endDocument();
}

} // namespace XML
} // namespace OpenRTI
