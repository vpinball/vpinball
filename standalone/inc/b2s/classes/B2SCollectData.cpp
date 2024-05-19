#include "core/stdafx.h"

#include "B2SCollectData.h"
#include "CollectData.h"

B2SCollectData::B2SCollectData(int skipFrames)
{
   m_skipFrames = skipFrames;
}

bool B2SCollectData::Add(int key, CollectData* pCollectData)
{
   bool ret = false;

   m_mutex.lock();

   if (contains(key)) {
      if (pCollectData->IsEarlyOffMode() && (*this)[key]->GetState() == 0 && pCollectData->GetState() == 0)
         (*this)[key]->SetState(2);
      else
         (*this)[key]->SetState(pCollectData->GetState());
      (*this)[key]->SetTypes((*this)[key]->GetTypes() | pCollectData->GetTypes());
      ret = true;
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