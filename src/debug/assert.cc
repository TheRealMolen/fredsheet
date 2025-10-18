#include "assert.h"

#include "test_framework.h"

#ifdef _WIN32
#include <crtdbg.h>
#endif

#ifndef NDEBUG

AssertMode g_assertMode = AssertMode::BreakIfDebuggerAttached;

REGISTER_EXCEPTION_TRANSLATOR(AssertException& e) {
    return doctest::String(e.msg.c_str());
}

#endif
