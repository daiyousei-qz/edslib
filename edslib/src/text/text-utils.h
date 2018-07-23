#pragma once
#include <cassert>
#include <algorithm>

namespace eds::text
{
    using zstring = const char*;

    inline char Consume(zstring& s)
    {
        assert(*s);

        return *(s++);
    }

    inline bool ConsumeIf(zstring& s, char ch)
    {
        assert(ch != '\0');

        if (*s == ch)
        {
            ++s;
            return true;
        }

        return false;
    }

    // range is inclusive
    inline bool ConsumeIfRange(zstring& s, int min, int max)
    {
        assert(min <= max && min > 0);

        if (*s >= min && *s <= max)
        {
            ++s;
            return true;
        }

        return false;
    }

    inline bool ConsumeIfSeq(zstring& s, zstring pred)
    {
        assert(*pred);

        auto sp = s;
        auto pp = pred;
        while (*pp)
        {
            if (*pp != *sp)
                return false;

            ++sp;
            ++pp;
        }

        s = sp;
        return true;
    }

    inline bool ConsumeIfAny(zstring& s, zstring pred)
    {
        assert(*pred);

        for (auto p = pred; *p; ++p)
        {
            if (*s == *p)
            {
                ++s;
                return true;
            }
        }

        return false;
    }
}