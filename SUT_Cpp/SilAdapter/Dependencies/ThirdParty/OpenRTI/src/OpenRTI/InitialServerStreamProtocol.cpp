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
#include "InitialServerStreamProtocol.h"

#include "LogStream.h"
#include "AbstractServer.h"
#include "MessageEncodingRegistry.h"
#include "ServerOptions.h"
#include "ZLibProtocolLayer.h"

namespace OpenRTI {

InitialServerStreamProtocol::InitialServerStreamProtocol(AbstractServer& abstractServer) :
  _abstractServer(abstractServer)
{
  // Add space for the initial header
  addScratchReadBuffer(12);
}

InitialServerStreamProtocol::~InitialServerStreamProtocol() noexcept
{
}

void
InitialServerStreamProtocol::readOptionMap(const StringStringListMap& clientOptionMap)
{
#if defined OPENRTI_ENCODING_DEVELOPMENT_WARNING
  Log(MessageCoding, Warning) << OPENRTI_ENCODING_DEVELOPMENT_WARNING << std::endl;
#endif

  // Given the clients values in value map, choose something sensible.
  // Here we might want to have something configurable to prefer something over the other??
  StringStringListMap::const_iterator i;
  i = clientOptionMap.find("version");
  if (i == clientOptionMap.end()) {
    errorResponse("No version field in the connect header given.");
    return;
  }

  //DebugPrintf("%s: version=%s\n", __FUNCTION__, join(i->second, ",").c_str());
  std::string protocolVersion = std::to_string(OPENRTI_ENCODING_VERSION);
  if (!getCompatibleVersion(protocolVersion, i->second)) {
    errorResponse("Client does not support version " OPENRTI_ENCODING_VERSION_STRING " of the protocol.");
    return;
  }
  //DebugPrintf("%s: compatible version=%s\n", __FUNCTION__, protocolVersion.c_str());

  // Check the encodings
  i = clientOptionMap.find("encoding");
  if (i == clientOptionMap.end() || i->second.empty()) {
    errorResponse("Client advertises no encoding!");
    return;
  }
  // collect some possible encodings
  StringList encodingList = MessageEncodingRegistry::instance().getCommonEncodings(i->second);
  if (encodingList.empty()) {
    errorResponse("Client and server have no common encoding!");
    return;
  }
  // FIXME Here, we could negotiate with a server configuration which one to take...

  // Preload this with the server nodes configuration
  StringStringListMap responseValueMap;
  responseValueMap = _abstractServer.getServerNode().getServerOptions()._optionMap;

  // Since we already asked for the common encodings, this must be successful now.
  SharedPtr<AbstractMessageEncoding> messageProtocol;
  messageProtocol = MessageEncodingRegistry::instance().getEncoding(encodingList.front());

  // Get a new client connect from the server implementation.
  StringStringListMap resultingClientOptionMap;
  for (auto option : clientOptionMap)
  {
    if (option.first == "version")
    {
      StringList compatibleVersionList{protocolVersion};
      resultingClientOptionMap.insert(std::make_pair(option.first, compatibleVersionList));
    }
    else
    {
      resultingClientOptionMap.insert(std::make_pair(option.first, option.second));
    }
  }
  SharedPtr<AbstractConnect> connect;
  connect = _abstractServer.sendConnect(resultingClientOptionMap, false /*parent*/);
  if (!connect.valid()) {
    errorResponse("Could not get an internal connect structure from the server!");
    return;
  }
  messageProtocol->setConnect(connect);

  // This is the part of the protocol stack that replaces this initial stuff.
  SharedPtr<AbstractProtocolLayer> protocolStack = messageProtocol;

  // Survived, respond with a valid response packet
  responseValueMap["version"].clear();
  responseValueMap["version"].push_back(protocolVersion);
  responseValueMap["encoding"].clear();
  responseValueMap["encoding"].push_back(encodingList.front());
  responseValueMap["compression"].clear();

  if (_abstractServer.getServerNode().getServerOptions()._preferCompression) {
    i = clientOptionMap.find("compression");
    if (i != clientOptionMap.end()) {
      for (StringList::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
// #if defined(OPENRTI_HAVE_XZ)
//           if (*j == "lzma") {
//             SharedPtr<LZMACompressionProtocolLayer> layer = new LZMACompressionProtocolLayer;
//             layer->setProtocolLayer(protocolStack);
//             protocolStack = layer;
//             responseValueMap["compression"].push_back("lzma");
//             break;
//           }
// #endif
#if defined(OPENRTI_HAVE_ZLIB)
        if (*j == "zlib") {
          SharedPtr<ZLibProtocolLayer> layer = new ZLibProtocolLayer;
          layer->setProtocolLayer(protocolStack);
          protocolStack = layer;
          responseValueMap["compression"].push_back("zlib");
          break;
        }
#endif
      }
    }
  }
  if (responseValueMap["compression"].empty())
    responseValueMap["compression"].push_back("no");

  writeOptionMap(responseValueMap);
  setFollowupProtocol(protocolStack);
}

void
InitialServerStreamProtocol::errorResponse(const std::string& errorMessage)
{
  StringStringListMap responseValueMap;
  responseValueMap["error"].push_back(errorMessage);
  writeOptionMap(responseValueMap);
}

} // namespace OpenRTI
