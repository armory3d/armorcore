#pragma once

#include "semaphore.h"
#include "../V8/include/v8.h"

void startDebugger(v8::Isolate* isolate, int port);
bool tickDebugger();

extern bool messageLoopPaused;
