#pragma once

#include <map>


namespace B2SLegacy {

class B2SPictureBox;

class ZOrderCollection final : public std::map<int, vector<B2SPictureBox*>>
{
public:
   void Add(B2SPictureBox* pPicbox);
};

}
