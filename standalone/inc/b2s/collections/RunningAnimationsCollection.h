#pragma once

#include <map>

class RunningAnimationsCollection : public vector<string>
{
public:
   static RunningAnimationsCollection* GetInstance();

   void Add(const string& item);
   bool Remove(const string& item);
   bool Contains(const string& item);

private:
   static RunningAnimationsCollection* m_pInstance;
};