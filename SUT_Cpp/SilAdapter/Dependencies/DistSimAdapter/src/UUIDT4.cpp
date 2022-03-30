/*-----------------------------------------------------------------------------
Copyright (c) Vector Informatik GmbH. All rights reserved.
-----------------------------------------------------------------------------*/

#include "UUIDT4.h"

namespace NDistSimIB
{
  namespace UUIDT4 {

    // Type 4 UUID Generator
    UUID generate() {
      thread_local static std::mt19937 gen(std::random_device{}()); // Seed with random_device
      thread_local static std::uniform_int_distribution<uint64_t> dist(0, (uint64_t)(~0)); // Invert 0 with ~ to get max of uint64_t

      UUID res;
      res.ab = dist(gen);
      res.cd = dist(gen);

      // First entry of third block is UUID Version (Fixed to type 4)
      res.ab = (res.ab & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL;
      // First entry of fourth block is UUID Variant (8,9,A,B)
      res.cd = (res.cd & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;

      return res;
    }

    // UUID put together from two uint64_t
    UUID rebuild(uint64_t ab, uint64_t cd) {
      UUID u;
      u.ab = ab; u.cd = cd;
      return u;
    }

    // String conversion
    std::string toString(const UUID& u) {
      std::stringstream ss;
      ss << std::hex << std::nouppercase << std::setfill('0');

      uint32_t a = (u.ab >> 32);
      uint32_t b = (u.ab & 0xFFFFFFFF);
      uint32_t c = (u.cd >> 32);
      uint32_t d = (u.cd & 0xFFFFFFFF);

      ss << std::setw(8) << (a) << '-';
      ss << std::setw(4) << (b >> 16) << '-';
      ss << std::setw(4) << (b & 0xFFFF) << '-';
      ss << std::setw(4) << (c >> 16) << '-';
      ss << std::setw(4) << (c & 0xFFFF);
      ss << std::setw(8) << d;

      return ss.str();
    }

  } // namespace UUIDT4

} // namespace NDistSimIB