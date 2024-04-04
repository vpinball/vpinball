#pragma once

#include <map>

class B2SReelBox;

struct CaseInsensitiveComparatorReelBoxCollection {
    bool operator() (const string& a, const string& b) const noexcept {
        return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(), 
            [](unsigned char c1, unsigned char c2) { return std::tolower(c1) < std::tolower(c2); });
    }
};

class ReelBoxCollection : public std::map<string, B2SReelBox*, CaseInsensitiveComparatorReelBoxCollection>
{
public:
   void Add(B2SReelBox* pReelbox);
};