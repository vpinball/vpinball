#pragma once

#include <map>

class ControlCollection;

class B2SPlayer : public std::map<int, ControlCollection*>
{
public:
   void Add(int playerno);
};