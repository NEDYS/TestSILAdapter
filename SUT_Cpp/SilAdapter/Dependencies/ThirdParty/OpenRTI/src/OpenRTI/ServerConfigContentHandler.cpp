/* -*-c++-*- OpenRTI - Copyright (C) 2009-2012 Mathias Froehlich
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
#include "ServerConfigContentHandler.h"

#include <iosfwd>
#include <sstream>
#include <cstring>
#include <vector>

#include "Attributes.h"
#include "ContentHandler.h"
#include "ExpatXMLReader.h"
#include "StringUtils.h"
#include "dprintf.h"

namespace OpenRTI {

static bool enableFlagToBool(const char* enableValue)
{
  if (!enableValue)
  {
    return false;
  }
  std::string enable = trim(enableValue);
  if (!enable.empty())
  {
    if (enable[0] == '1' || enable[0] == 't' || enable[0] == 'T')
    {
      return true;
    }
    if (enable[0] == '0' || enable[0] == 'f' || enable[0] == 'F')
    {
      return false;
    }
  }
  throw RTIinternalError("Invalid enable flag!");
}

ServerConfigContentHandler::ServerConfigContentHandler() noexcept :
  _enableZLibCompression(true),
  _enableNetworkStatistics(false)
{
}

ServerConfigContentHandler::~ServerConfigContentHandler() noexcept
{
}

void
ServerConfigContentHandler::startDocument()
{
  OpenRTIAssert(_modeStack.empty());
}

void
ServerConfigContentHandler::endDocument()
{
  OpenRTIAssert(_modeStack.empty());
}

void
ServerConfigContentHandler::parseLogCategory(const std::string& s)
{
  std::vector<std::string> categories = split(s, ", |");
  if (!categories.empty())
  {
    _LogCategory = 0;
    for (std::string category : categories)
    {
      static const struct
      {
        const char* name;
        LogStream::Category bit;
      } categoryToBit[] =
      {
#define CATEGORY(name) { #name, LogStream::name }
        CATEGORY(Assert),
        CATEGORY(Network),
        CATEGORY(MessageCoding),
        CATEGORY(FederateAmbassador),
        CATEGORY(ServerConnect),
        CATEGORY(ServerMessage),
        CATEGORY(ServerFederation),
        CATEGORY(ServerFederate),
        CATEGORY(ServerSyncronization),
        CATEGORY(ServerTime),
        CATEGORY(ServerObjectInstance),
        CATEGORY(All)
#undef CATEGORY
      };
      for (unsigned j = 0; j < sizeof(categoryToBit) / sizeof(categoryToBit[0]); ++j)
      {
        if (category == categoryToBit[j].name)
        {
          _LogCategory |= categoryToBit[j].bit;
          //printf(" + %s 0x%08x\n", categoryToBit[j].name, categoryToBit[j].bit);
        }
      }
    }
    if (_LogCategory == 0) {
      _LogCategory = static_cast<LogStream::Category>(strtol(s.c_str(), nullptr, 16));
    }
  }
}

void
ServerConfigContentHandler::startElement(const char* /*uri*/, const char* name,
  const char* /*qName*/, const XML::Attributes* atts)
{
  if (strcmp(name, "OpenRTIServerConfig") == 0)
  {
    if (!_modeStack.empty())
    {
      throw RTIinternalError("OpenRTIServerConfig is not top level tag!");
    }
    _modeStack.push_back(OpenRTIServerConfigMode);
    std::string version = trim(atts->getValue("version"));
    if (version != "1")
    {
      throw RTIinternalError("Unknown OpenRTIServerConfig version!");
    }
  }
  else if (strcmp(name, "parentServer") == 0)
  {
    if (getCurrentMode() != OpenRTIServerConfigMode)
    {
      throw RTIinternalError("parentServer tag not inside of OpenRTIServerConfig tag!");
    }
    _modeStack.push_back(ParentServerMode);
    _parentServerUrl = trim(atts->getValue("url"));
  }
  else if (strcmp(name, "enableZLibCompression") == 0)
  {
    if (getCurrentMode() != OpenRTIServerConfigMode)
    {
      throw RTIinternalError("enableZLibCompression tag not inside of OpenRTIServerConfig or listen tag!");
    }
    _modeStack.push_back(EnableZLibCompressionMode);
    bool enable = enableFlagToBool(atts->getValue("enable"));
    _enableZLibCompression = enable;
  }
  else if (strcmp(name, "listen") == 0)
  {
    if (getCurrentMode() != OpenRTIServerConfigMode)
    {
      throw RTIinternalError("listen tag not inside of OpenRTIServerConfig!");
    }
    _modeStack.push_back(ListenMode);
    _listenConfig.push_back(ListenConfig());
    _listenConfig.back()._url = trim(atts->getValue("url"));
  }
  else if (strcmp(name, "QueueLimit") == 0)
  {
    if (getCurrentMode() != OpenRTIServerConfigMode)
    {
      throw RTIinternalError("QueueLimit tag not inside of OpenRTIServerConfig!");
    }
    _modeStack.push_back(QueueLimitMode);
    _queueLimit = static_cast<size_t>(std::stoull(trim(atts->getValue("value"))) * 1024);
  }
  else if (strcmp(name, "LogPriority") == 0)
  {
    if (getCurrentMode() != OpenRTIServerConfigMode)
    {
      throw RTIinternalError("LogPriority tag not inside of OpenRTIServerConfig or listen tag!");
    }
    _modeStack.push_back(LogPriorityMode);
    std::string value = trim(atts->getValue("value"));
    _LogPriority = static_cast<LogStream::Priority>(strtol(value.c_str(), nullptr, 16));
    _LogPrioritySet = true;
    //printf("_LogPriority=0x%08x\n", _LogPriority);
  }
  else if (strcmp(name, "LogCategory") == 0)
  {
    if (getCurrentMode() != OpenRTIServerConfigMode)
    {
      throw RTIinternalError("LogPriority tag not inside of OpenRTIServerConfig or listen tag!");
    }
    _modeStack.push_back(LogCategoryMode);
    std::string value = trim(atts->getValue("value"));
    parseLogCategory(value);
    _LogCategorySet = true;
  }
  else if (strcmp(name, "LogFile") == 0)
  {
    if (getCurrentMode() != OpenRTIServerConfigMode)
    {
      throw RTIinternalError("LogPriority tag not inside of OpenRTIServerConfig or listen tag!");
    }
    _modeStack.push_back(LogFileMode);
    std::string value = trim(atts->getValue("value"));
    _LogFile = value;
    _LogFileSet = true;
  }
  else if (strcmp(name, "LogToConsole") == 0)
  {
    if (getCurrentMode() != OpenRTIServerConfigMode)
    {
      throw RTIinternalError("LogToConsole tag not inside of OpenRTIServerConfig or listen tag!");
    }
    _modeStack.push_back(LogToConsoleMode);
    _LogToConsole = enableFlagToBool(atts->getValue("enable"));
    _LogToConsoleSet = true;
  }
  else if (strcmp(name, "LogToDebug") == 0)
  {
    if (getCurrentMode() != OpenRTIServerConfigMode)
    {
      throw RTIinternalError("LogToDebug tag not inside of OpenRTIServerConfig or listen tag!");
    }
    _modeStack.push_back(LogToDebugMode);
    sEnableDebugPrintf = enableFlagToBool(atts->getValue("enable"));
  }
  else if (strcmp(name, "NetworkStatistics") == 0)
  {
    if (getCurrentMode() != OpenRTIServerConfigMode && getCurrentMode() != ListenMode)
    {
      throw RTIinternalError("NetworkStatistics tag not inside of OpenRTIServerConfig or listen tag!");
    }
    _modeStack.push_back(NetworkStatisticsMode);
    bool enable = enableFlagToBool(atts->getValue("enable"));
    _enableNetworkStatistics = enable;
  }
  else
  {
    _modeStack.push_back(UnknownMode);
    throw RTIinternalError("Unknown tag!");
  }
}

void
ServerConfigContentHandler::endElement(const char* /*uri*/, const char* /*name*/, const char* /*qName*/)
{
  _modeStack.pop_back();
}

}
