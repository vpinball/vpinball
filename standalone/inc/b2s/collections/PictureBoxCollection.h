#pragma once

#include "../b2s_i.h"

#include <map>

class B2SPictureBox;

struct CaseInsensitiveComparatorPictureBox {
    bool operator() (const string& a, const string& b) const noexcept {
        return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(), 
            [](unsigned char c1, unsigned char c2) { return std::tolower(c1) < std::tolower(c2); });
    }
};

class PictureBoxCollection : public std::map<string, B2SPictureBox*, CaseInsensitiveComparatorPictureBox>
{
public:
   void Add(B2SPictureBox* pPicbox, eDualMode dualMode = eDualMode_Both);
};