#pragma once


#include <map>

#include "common.h"


namespace B2SLegacy {

class B2SPictureBox;

struct CaseInsensitiveComparatorPictureBox {
   bool operator() (const string& a, const string& b) const noexcept {
      return std::ranges::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(),
         [](char c1, char c2) { return cLower(c1) < cLower(c2); });
   }
};

class PictureBoxCollection final : public std::map<string, B2SPictureBox*, CaseInsensitiveComparatorPictureBox>
{
public:
   void Add(B2SPictureBox* pPicbox, eDualMode dualMode = eDualMode_Both);
};

}
