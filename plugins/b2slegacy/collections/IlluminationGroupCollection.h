#pragma once

#include <map>

#include "common.h"


namespace B2SLegacy {

class B2SPictureBox;

struct CaseInsensitiveComparatorIlluminationGroupCollection {
   bool operator() (const string& a, const string& b) const noexcept {
      return std::ranges::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(),
         [](char c1, char c2) { return cLower(c1) < cLower(c2); });
   }
};

class IlluminationGroupCollection final : public std::map<string, vector<B2SPictureBox*>, CaseInsensitiveComparatorIlluminationGroupCollection>
{
public:
   void Add(B2SPictureBox* pPictureBox);
};

}
