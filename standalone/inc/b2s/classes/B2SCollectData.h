#pragma once

#include <map>
#include <mutex>

class CollectData;

class B2SCollectData : public std::map<int, CollectData*>
{
public:
   B2SCollectData(int skipFrames);

   bool Add(int key, CollectData* pCollectData);
   void DataAdded();
   bool ShowData() const;
   void ClearData(int skipFrames);

   void Lock();
   void Unlock();

private:
   int m_skipFrames;
   std::mutex m_mutex;
};
