#include "../common.h"

#include "B2SCollectData.h"
#include "CollectData.h"

namespace B2SLegacy {

B2SCollectData::B2SCollectData(int skipFrames)
   : m_skipFrames(skipFrames)
{
}

B2SCollectData::~B2SCollectData()
{
   for (auto& it : *this)
      delete it.second;
}

bool B2SCollectData::Add(int key, CollectData* pCollectData)
{
   bool ret = false;

   m_mutex.lock();

   const auto& it = find(key);
   if (it != end()) {
      if (pCollectData->IsEarlyOffMode() && it->second->GetState() == 0 && pCollectData->GetState() == 0)
         it->second->SetState(2);
      else
         it->second->SetState(pCollectData->GetState());
      it->second->SetTypes(it->second->GetTypes() | pCollectData->GetTypes());
      ret = true;

      delete pCollectData;
   }
   else
      (*this)[key] = pCollectData;

   m_mutex.unlock();

   return ret;
}

void B2SCollectData::DataAdded()
{
   m_skipFrames--;
}

bool B2SCollectData::ShowData() const
{
   return (m_skipFrames < 0);
}


void B2SCollectData::ClearData(int skipFrames)
{
   m_mutex.lock();

   for (auto& it : *this)
      delete it.second;
   clear();

   m_mutex.unlock();

   if (m_skipFrames <= 0)
      m_skipFrames = skipFrames;
}

void B2SCollectData::Lock()
{
   m_mutex.lock();
}

void B2SCollectData::Unlock()
{
   m_mutex.unlock();
}

}
