#pragma once

unsigned long StringHash(const unsigned char *str);
inline unsigned long StringHash(const char *str)             { return StringHash((const unsigned char*)str); }

struct StringHashFunctor
{
    unsigned long operator() (const char* str) const
    {
        return StringHash(str);
    }
};

struct StringComparator
{
    bool operator()(const char* str1, const char* str2) const
    {
        return lstrcmp(str1, str2) == 0;
    }
};
