#include "stdafx.h"

#include "Sequence.h"

Sequence::Sequence(FlexDMD* pFlexDMD, const string& name) : Group(pFlexDMD, name)
{
   m_finished = false;
   m_pActiveScene = NULL;
}

Sequence::~Sequence()
{
}

void Sequence::Enqueue(Scene* scene)
{
   m_pendingScenes.push_back(scene);
   m_finished = false;
}

void Sequence::RemoveAllScenes()
{
   if (m_pActiveScene)
      m_pActiveScene->Remove();

   m_pActiveScene = NULL;
   m_pendingScenes.clear();
   m_finished = true;
}

void Sequence::RemoveScene(string name)
{
   if (m_pActiveScene->GetName() == name) {
      m_pActiveScene->Remove();
      m_pActiveScene = NULL;
   }
   m_pendingScenes.erase(std::remove_if(m_pendingScenes.begin(), m_pendingScenes.end(), [name](Group* p) {
      return p->GetName() == name;
   }), m_pendingScenes.end());
   m_finished = !m_pActiveScene && m_pendingScenes.size() == 0;
}

bool Sequence::IsFinished()
{
   return m_finished;
}

void Sequence::Update(float delta)
{
   Group::Update(delta);
   if (m_pActiveScene && m_pActiveScene->IsFinished()) {
      m_pActiveScene->Remove();
      m_pActiveScene = NULL;
   }
   if (!m_pActiveScene && m_pendingScenes.size() > 0) {
      m_pActiveScene = m_pendingScenes[0];
      m_pendingScenes.erase(m_pendingScenes.begin());
      AddActor((Actor*)m_pActiveScene);
      m_pActiveScene->Update(0);
   }
   m_finished = !m_pActiveScene && m_pendingScenes.size() == 0;
}

void Sequence::Draw(VP::SurfaceGraphics* pGraphics)
{
   if (GetVisible() && m_pActiveScene) {
      pGraphics->SetColor(RGB(0, 0, 0));
      pGraphics->Clear();
      Group::Draw(pGraphics);
   }
}