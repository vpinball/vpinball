#pragma once

#include <map>


namespace B2SLegacy {

class ControlCollection;

class B2SPlayer : public std::map<int, ControlCollection*>
{
public:
   void Add(int playerno);
};

}
