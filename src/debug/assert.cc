#include "assert.h"

#include "test_framework.h"
#include <crtdbg.h>

#ifndef NDEBUG

AssertMode g_assertMode = AssertMode::BreakIfDebuggerAttached;

REGISTER_EXCEPTION_TRANSLATOR(AssertException& e) {
    return doctest::String(e.msg.c_str());
}

#endif
