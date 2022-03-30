
#pragma once

struct AbstractNotificationHandle
{
  AbstractNotificationHandle() noexcept = default;
  virtual ~AbstractNotificationHandle() noexcept = default;
  virtual void Signal() = 0;
};
