#pragma once

#include "debug/assert.h"

// hide conflicts btw names in raylib and win32 api included by doctest
#if defined(_WIN32)           
#define NOGDI             // All GDI defines and routines
#define NOUSER            // All USER defines and routines
#endif

#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include "doctest.h"


#ifdef ASSERTS_ENABLED

#define CHECK_ASSERTS(expr)     do{ ScopedAssertMode _(AssertMode::ThrowAlways); CHECK_THROWS_AS((expr), AssertException); }while(0)

#else

// run the code in release, to ensure it doesn't crash
#define CHECK_ASSERTS(expr)     expr

#endif
