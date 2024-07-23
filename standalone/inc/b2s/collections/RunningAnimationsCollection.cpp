#include "stdafx.h"

#include "RunningAnimationsCollection.h"

RunningAnimationsCollection* RunningAnimationsCollection::m_pInstance = NULL;

RunningAnimationsCollection* RunningAnimationsCollection::GetInstance()
{
   if (!m_pInstance)
      m_pInstance = new RunningAnimationsCollection();

   return m_pInstance;
}

void RunningAnimationsCollection::Add(const string& item)
{
   auto it = std::find(begin(), end(), item);
   if (it == end())
      push_back(item);
}

bool RunningAnimationsCollection::Remove(const string& item)
{
   auto it = std::find(begin(), end(), item);
   if (it != end())
      erase(it);

   return true;
}

bool RunningAnimationsCollection::Contains(const string& item)
{
   auto it = std::find(begin(), end(), item);
   return it != end();
}