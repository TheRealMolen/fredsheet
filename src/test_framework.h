#pragma once

#include "debug/assert.h"

#include "fredplatform.h"

#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include "doctest.h"


#ifdef ASSERTS_ENABLED

#define CHECK_ASSERTS(expr)     do{ ScopedAssertMode _(AssertMode::ThrowAlways); CHECK_THROWS_AS((expr), AssertException); }while(0)

#else

// run the code in release, to ensure it doesn't crash
#define CHECK_ASSERTS(expr)     expr

#endif
