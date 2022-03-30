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

#ifndef OpenRTI_InitialClientStreamProtocol_h
#define OpenRTI_InitialClientStreamProtocol_h

#include "InitialStreamProtocol.h"

namespace OpenRTI {

class AbstractServer;

class OPENRTI_API InitialClientStreamProtocol final : public InitialStreamProtocol {
public:
  InitialClientStreamProtocol(AbstractServer& abstractServer);
  InitialClientStreamProtocol(AbstractServer& abstractServer, const StringStringListMap& connectOptions);
  virtual ~InitialClientStreamProtocol() noexcept;

  void setConnectOptions(StringStringListMap connectOptions);
  void readOptionMap(const StringStringListMap& optionMap) override;

  void error(const Exception& e) override;

  bool getSuccessfulConnect() const
  { return _successfulConnect; }
  const std::string& getErrorMessage() const
  { return _errorMessage; }

private:
  AbstractServer& _abstractServer;
  std::string _errorMessage;
  bool _successfulConnect;
};

} // namespace OpenRTI

#endif
