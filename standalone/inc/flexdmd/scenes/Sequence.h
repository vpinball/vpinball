#pragma once

#include "../actors/Group.h"
#include "Scene.h"

class Sequence : public Group
{
public:
   Sequence(FlexDMD* pFlexDMD, const string& name);
   ~Sequence();

   void Enqueue(Scene* scene);
   void RemoveAllScenes();
   void RemoveScene(string name);
   bool IsFinished();
   Scene* GetActiveScene() { return m_pActiveScene; };
   void Update(float delta) override;
   void Draw(VP::SurfaceGraphics* pGraphics) override;

private:
   vector<Scene*> m_pendingScenes;
   bool m_finished;
   Scene* m_pActiveScene;
};