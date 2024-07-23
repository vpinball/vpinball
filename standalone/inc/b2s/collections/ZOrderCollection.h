#pragma once

#include <map>

class B2SPictureBox;

class ZOrderCollection : public std::map<int, vector<B2SPictureBox*>>
{
public:
   void Add(B2SPictureBox* pPicbox);
};
