/*-------------------------------------------------------------------------------
Copyright (c) Vector Informatik GmbH. All rights reserved.
-----------------------------------------------------------------------------*/
#pragma once

struct AbstractService
{
  virtual void OnJoined() = 0;
  virtual void Reset() = 0;
  // Returns true if any buffer has been processed. This skips the WaitForEvent and calls evokeCallback in polling thread.
  virtual bool ProcessBuffers() = 0;
};