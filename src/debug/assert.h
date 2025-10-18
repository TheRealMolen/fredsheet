#pragma once

#include "helper/cpp.h"

#include "fredplatform.h"

#ifdef _WIN32
#include <debugapi.h>
#endif
#include <signal.h>

//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------

#ifdef NDEBUG

#define _ASSERT(expr)   do{}while(0)
#define assert(expr)    _ASSERT(expr)

#else // !NDEBUG

#define ASSERTS_ENABLED


#define _ASSERT(expr)               \
    do {                            \
        if (!(expr)) [[unlikely]]   \
            HandleAssertFiring(__FILE__, __LINE__,  #expr);   \
    } while (0)

#define assert(expr)    _ASSERT(expr)



#include <cstdio>
#include <format>


//-------------------------------------------------------------------------------------------------------------

#ifndef _WIN32
inline void __debugbreak()
{
    raise(SIGTRAP);
}
inline void OutputDebugString(const char* str)
{
    puts(str);
}
#define OutputDebugStringA OutputDebugString
#endif

//-------------------------------------------------------------------------------------------------------------

enum class AssertMode
{
    BreakIfDebuggerAttached,    // ... otherwise throw an AssertException
    BreakAlways,
    ThrowAlways,
};

extern AssertMode g_assertMode;


class ScopedAssertMode
{
    NON_COPYABLE(ScopedAssertMode);
    ScopedAssertMode() = delete;
public:
    explicit ScopedAssertMode(AssertMode newMode) : m_oldMode(g_assertMode)
    {
        g_assertMode = newMode;
    }
    ~ScopedAssertMode()
    {
        g_assertMode = m_oldMode;
    }

private:
    AssertMode m_oldMode;
};


//-------------------------------------------------------------------------------------------------------------

struct AssertException : std::exception
{
    std::string msg;

    AssertException(const char* file, int line, const char* expr)
        : msg(std::format("{}({}): ASSERT: {}", file, line, expr))
    {
        OutputDebugStringA(msg.c_str());
        OutputDebugStringA("\n");
    }

    virtual char const* what() const _NOEXCEPT override
    {
        return msg.c_str();
    }
};


//-------------------------------------------------------------------------------------------------------------

inline void HandleAssertFiring(const char* file, int line, const char* expr, const char* message=nullptr)
{
    (void)message;

    using enum AssertMode;
    switch (g_assertMode)
    {
    case BreakIfDebuggerAttached:
#ifdef _WIN32
        if (IsDebuggerPresent())
            __debugbreak();
        else
#endif
            throw AssertException(file, line, expr);
        break;

    case BreakAlways:
        __debugbreak();
        break;

    case ThrowAlways:
        throw AssertException(file, line, expr);
    }
}


#endif // !NDEBUG

//-------------------------------------------------------------------------------------------------------------

#include <stdexcept>
#include <type_traits>


inline constexpr void constexpr_assert(bool condition, const char* msg)
{
    if (std::is_constant_evaluated())
    {
        if (!condition)
            throw std::logic_error(msg);
    }
    else
    {
        _ASSERT(condition && msg);
    }
}

#define CONSTEXPR_ASSERT(expr)      constexpr_assert(bool((expr)), #expr)

//-----------------------------------------------------------------------------------------------------------
