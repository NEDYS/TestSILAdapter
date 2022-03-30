/* -*-c++-*- OpenRTI - Copyright (C) 2004-2015 Mathias Froehlich
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

#ifndef OpenRTI_Clock_h
#define OpenRTI_Clock_h

#include <iomanip>
#include <limits>
#include <ostream>
#include <sstream>
#include "OpenRTIConfig.h"
#include "Export.h"
#include "Types.h"


#ifndef __CPlusPlusStd
#error "must include OpenRTIConfig.h!"
#endif

#include <chrono>
#include <thread>

namespace OpenRTI {

// use something sophisticated for the posix clock stuff.
// win32 timeouts are just relative milliseconds which is easy to handle with any clock as long as it is consistent
class OPENRTI_API Clock {
public:
  using Rep = uint64_t;

  Clock() noexcept : _nsec(0) { }
  Clock(const Clock&) noexcept = default;
  Clock(Clock&&) noexcept = default;

  explicit Clock(const std::chrono::seconds& s) noexcept
  {
    _nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(s).count();
  }
  explicit Clock(const std::chrono::milliseconds& ms) noexcept
  {
    _nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(ms).count();
  }
  explicit Clock(const std::chrono::nanoseconds& us) noexcept
  {
    _nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(us).count();
  }
  explicit Clock(const std::chrono::steady_clock::time_point& tp) noexcept
  {
    _nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(tp.time_since_epoch()).count();
  }

  ~Clock() noexcept = default;
  Clock& operator=(const Clock&) = default;
  Clock& operator=(Clock&&) = default;

  static Clock now() noexcept
  {
    return Clock(std::chrono::steady_clock::now());
  }

  operator std::chrono::steady_clock::time_point() const noexcept { return std::chrono::steady_clock::time_point(std::chrono::nanoseconds(_nsec)); }
  static void sleep_for(const Clock& reltime)
  {
    std::this_thread::sleep_for(std::chrono::nanoseconds(reltime.getNanoSeconds()));
  }

  static Clock zero() noexcept { return Clock(0); }
  static Clock max() noexcept { return Clock(std::numeric_limits<uint64_t>::max()); }

  /// Conversion from seconds, note that these conversions saturate
  static Clock fromSeconds(int seconds) noexcept
  {
    if (seconds <= 0)
      return zero();
    // Note that the unsigned cast has a guarantee to be the same
    // width than the int in the input argument. Those get upcasted
    // to the bigger unsigned then.
    if (std::numeric_limits<uint64_t>::max()/1000000000ULL <= static_cast<uint64_t>(seconds))
      return max();
    return Clock(seconds * 1000000000ULL);
  }
  static Clock fromSeconds(const double& seconds) noexcept
  {
    if (seconds <= 0.0)
      return zero();
    else {
      double nsec = 1e9*seconds;
      if (double(std::numeric_limits<uint64_t>::max()) <= nsec)
        return max();
      return Clock(uint64_t(nsec));
    }
  }
  static Clock fromMilliSeconds(uint32_t milliSeconds) noexcept
  {
    if (milliSeconds <= 0)
      return zero();
    // Note that the unsigned cast has a guarantee to be the same
    // width than the int in the input argument. Those get upcasted
    // to the bigger unsigned then.
    if (std::numeric_limits<uint64_t>::max()/1000000ULL <= uint64_t(milliSeconds))
      return max();
    return Clock(milliSeconds*1000000ULL);
  }
  uint64_t getMilliSeconds() const noexcept
  {
    return _nsec / 1000000ULL;
  }
  uint64_t getMicroSeconds() const noexcept
  {
    return _nsec / 1000ULL;
  }
  /// Conversion from nanoseconds
  static Clock fromNanoSeconds(uint64_t nsec) noexcept { return Clock(nsec); }

  uint64_t getNanoSeconds() const noexcept { return _nsec; }
  void setNanoSeconds(uint64_t nsec) noexcept { _nsec = nsec; }

  // Arithmetic, note that these two do not wrap.
  Clock& operator+=(const Clock& clock) noexcept
  {
    if (~_nsec <= clock._nsec)
      _nsec = std::numeric_limits<uint64_t>::max();
    else
      _nsec += clock._nsec;
    return *this;
  }
  Clock& operator-=(const Clock& clock) noexcept
  {
    if (_nsec <= clock._nsec)
      _nsec = 0;
    else
      _nsec -= clock._nsec;
    return *this;
  }

  // Compares
  bool operator==(const Clock& clock) const noexcept
  { return _nsec == clock._nsec; }
  bool operator!=(const Clock& clock) const noexcept
  { return _nsec != clock._nsec; }
  bool operator<(const Clock& clock) const noexcept
  { return _nsec < clock._nsec; }
  bool operator<=(const Clock& clock) const noexcept
  { return _nsec <= clock._nsec; }
  bool operator>(const Clock& clock) const noexcept
  { return _nsec > clock._nsec; }
  bool operator>=(const Clock& clock) const noexcept
  { return _nsec >= clock._nsec; }

private:
  explicit Clock(Rep nsecs) noexcept
    : _nsec(nsecs)
  { }

  // In the worst case this is nanoseconds since epoch.
  // In the usual case this is the simulation time in nanoseconds for the logical time
  // and the monotonic posix clock for timeouts.
  Rep _nsec;
};

inline
Clock operator+(const Clock& clock1, const Clock& clock2) noexcept
{ return Clock(clock1) += clock2; }
inline
Clock operator-(const Clock& clock1, const Clock& clock2) noexcept
{ return Clock(clock1) -= clock2; }
// inline
// Clock operator*(const Clock& clock1, const Clock& clock2)
// { return Clock(clock1) *= clock2; }
// inline
// Clock operator/(const Clock& clock1, const Clock& clock2)
// { return Clock(clock1) /= clock2; }

inline
std::ostream&
operator<<(std::ostream& os, const Clock& c)
{
  os << "Clock(" << c.getNanoSeconds() * 1e-9;
  os << ", 0x" << std::hex << std::setw(16) << std::setfill('0') << c.getNanoSeconds();
  return os << ")";
}

} // namespace OpenRTI

#endif
