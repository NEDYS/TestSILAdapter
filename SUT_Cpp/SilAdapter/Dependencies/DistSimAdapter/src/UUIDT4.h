/*-----------------------------------------------------------------------------
Copyright (c) Vector Informatik GmbH. All rights reserved.
-----------------------------------------------------------------------------*/

#pragma once

#include <string>
#include <iostream>
#include <iomanip>
#include <random>
#include <sstream>

namespace NDistSimIB
{
// =========================================
// Simple UUID implementation based on 2x64bit rnd numbers
namespace UUIDT4
{
  struct UUID
  {
    uint64_t ab;
    uint64_t cd;

    bool operator==(const UUID& other) {
      return ab == other.ab && cd == other.cd;
    }

    bool operator!=(const UUID& other) {
      return !operator==(other);
    }
  };

  UUID generate();
  UUID rebuild(uint64_t ab, uint64_t cd);
  std::string toString(const UUID& u);

} //namespace UUIDT4

} //namespace NDistSimIB