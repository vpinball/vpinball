#include "Sequence.h"

#include <algorithm>

Sequence::Sequence(FlexDMD* pFlexDMD, const string& name)
   : Group(pFlexDMD, name)
   , m_finished(false)
   , m_pActiveScene(nullptr)
{
}

Sequence::~Sequence()
{
}

void Sequence::Enqueue(Scene* scene)
{
   scene->AddRef();
   m_pendingScenes.push_back(scene);
   m_finished = false;
}

void Sequence::RemoveAllScenes()
{
   if (m_pActiveScene)
      m_pActiveScene->Remove();
   for (auto scene : m_pendingScenes)
      scene->Release();
   m_pActiveScene = nullptr;
   m_pendingScenes.clear();
   m_finished = true;
}

void Sequence::RemoveScene(const string& name)
{
   if (m_pActiveScene->GetName() == name) {
      m_pActiveScene->Remove();
      m_pActiveScene = nullptr;
   }
   auto it = std::remove_if(m_pendingScenes.begin(), m_pendingScenes.end(), [name](Group* p) { return p->GetName() == name; });
   std::for_each(it, m_pendingScenes.end(), [](Scene* scene) { scene->Release(); });
   m_pendingScenes.erase(it, m_pendingScenes.end());
   m_finished = !m_pActiveScene && m_pendingScenes.empty();
}

bool Sequence::IsFinished() const
{
   return m_finished;
}

void Sequence::Update(float delta)
{
   Group::Update(delta);
   if (m_pActiveScene && m_pActiveScene->IsFinished()) {
      m_pActiveScene->Remove();
      m_pActiveScene = nullptr;
   }
   if (!m_pActiveScene && !m_pendingScenes.empty()) {
      m_pActiveScene = m_pendingScenes[0];
      m_pendingScenes.erase(m_pendingScenes.begin());
      AddActor(m_pActiveScene);
      m_pActiveScene->Update(0);
   }
   m_finished = !m_pActiveScene && m_pendingScenes.empty();
}

void Sequence::Draw(VP::SurfaceGraphics* pGraphics)
{
   if (GetVisible() && m_pActiveScene) {
      pGraphics->SetColor(RGB(0, 0, 0));
      pGraphics->Clear();
      Group::Draw(pGraphics);
   }
}
