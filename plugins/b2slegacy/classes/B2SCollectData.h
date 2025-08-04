#pragma once

#include <map>
#include <mutex>


namespace B2SLegacy {

class CollectData;

class B2SCollectData final : public std::map<int, CollectData*>
{
public:
   B2SCollectData(int skipFrames);
   ~B2SCollectData();

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

}
