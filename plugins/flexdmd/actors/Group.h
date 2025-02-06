#pragma once

#include "Actor.h"
#include "VPXPlugin.h"

class FlexDMD;
class Image;
class Label;
class Frame;
class AnimatedActor;

class Group : public Actor
{
public:
   Group(FlexDMD *pFlexDMD, const string &name) : Actor(pFlexDMD, name) {}
   ~Group();

   ActorType GetType() const override { return AT_Group; }

   int GetChildCount() const { return static_cast<int>(m_children.size()); }
   bool HasChild(const string &name) { return Get(name) != nullptr; }
   Group *GetGroup(const string &Name);
   Frame *GetFrame(const string &Name);
   Label *GetLabel(const string &Name);
   AnimatedActor *GetVideo(const string &Name);
   Image *GetImage(const string &Name);

   bool GetClip() const { return m_clip; }
   void SetClip(bool clip) { m_clip = clip; }

   void AddActor(Actor *actor);
   void AddActorAt(Actor *actor, int index);
   void RemoveActor(Actor *actor);
   void RemoveAll();
   vector<Actor *> GetChildren() { return m_children; }

   void OnStageStateChanged() override;
   void Update(float delta) override;
   void Draw(VP::SurfaceGraphics* pGraphics) override;

private:
   Group *GetRoot();
   Actor *Get(const string &name);

   bool m_clip = false;
   vector<Actor *> m_children;
};
