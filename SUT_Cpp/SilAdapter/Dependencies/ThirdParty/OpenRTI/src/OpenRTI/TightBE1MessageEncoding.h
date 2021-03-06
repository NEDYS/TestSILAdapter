/* -*-c++-*- OpenRTI - Copyright (C) 2009-2013 Mathias Froehlich
 *
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
 * This file is autogenerated by messagegen.py. Do not edit!
 *
 */

#ifndef OpenRTI_TightBE1MessageEncoding_h
#define OpenRTI_TightBE1MessageEncoding_h

#include "DebugNew.h"
#include "AbstractMessageEncoding.h"
#include "Export.h"

namespace OpenRTI {

class OPENRTI_LOCAL TightBE1MessageEncoding : public AbstractMessageEncoding {
public:
  TightBE1MessageEncoding() noexcept;
  virtual ~TightBE1MessageEncoding() noexcept;

  const char* getName() const override;

  void readPacket(const Buffer& buffer) override;
  void decodeBody(const VariableLengthData& variableLengthData);
  void decodePayload(const Buffer::const_iterator& i);
  void writeMessage(const AbstractMessage& message) override;

private:
  class DecodeStream;
  class DispatchFunctor;
  class PayloadDecoder;
  class EncodeStream;

  SharedPtr<AbstractMessage> _message;
};

} // namespace OpenRTI

#endif
