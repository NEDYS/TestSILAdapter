/*-----------------------------------------------------------------------------
Copyright (c) Vector Informatik GmbH. All rights reserved.
-----------------------------------------------------------------------------*/

#include "Utils.h"

namespace NDistSimIB
{
// Thread counter for DebugPrints
unsigned long long thread_id() {
    thread_local unsigned long long tid = ++thread_counter;
    return tid;
}


} //namespace NDistSimIB