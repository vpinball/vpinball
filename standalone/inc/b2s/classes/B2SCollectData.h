#pragma once

#include <map>

class CollectData;

class B2SCollectData : public std::map<int, CollectData*>
{
public:
   B2SCollectData(int skipFrames);

   bool Add(int key, CollectData* pCollectData);
   void DataAdded();
   bool ShowData();
   void ClearData(int skipFrames);

private:
   int m_skipFrames;
};