#pragma once

#include <cstring>
#include <streambuf>


//-------------------------------------------------------------------------------------------------------------

class cstr_streambuf : public std::basic_streambuf<char>
{
public:
    explicit cstr_streambuf(const char* str)
        : m_buf(str)
        , m_next(str)
        , m_len(strlen(str))
    { /**/ }


    // return the next char or eof()
    virtual int_type underflow() override
    {
        if (m_next == (m_buf + m_len))
            return traits_type::eof();

        return traits_type::to_int_type(*m_next);
    }
    // return the next char or eof(), and increment next ptr
    virtual int_type uflow() override
    {
        if (m_next == (m_buf + m_len))
            return traits_type::eof();

        auto c = traits_type::to_int_type(*m_next);
        ++m_next;
        return c;
    }
    // handle base class assuming that a putback failed because we gave it no buffer
    virtual int_type pbackfail(int_type c) override
    {
        _ASSERT(m_next > m_buf);
        _ASSERT((c == traits_type::eof()) || (traits_type::to_int_type(*(m_next-1)) == c));

        if (m_next == m_buf)
            return traits_type::eof();
        if (c != traits_type::eof() && traits_type::to_int_type(*(m_next - 1)) != c)
            return traits_type::eof();

        --m_next;
        return traits_type::to_int_type(*m_next);
    }
    // report the number of chars left to read
    virtual std::streamsize showmanyc() override
    {
        const char* end = m_buf + m_len;
        return (end - m_next);
    }

private:
    const char* m_buf = nullptr;
    const char* m_next = nullptr;
    size_t      m_len = 0;
};

//-------------------------------------------------------------------------------------------------------------

