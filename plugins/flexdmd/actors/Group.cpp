#include "FlexDMD.h"
#include "Group.h"
#include "Label.h"
#include "Frame.h"
#include "AnimatedActor.h"
#include "Image.h"

#include <algorithm>

Group::~Group()
{
   while (!m_children.empty())
      m_children.back()->Remove();
}

void Group::OnStageStateChanged()
{
   for (Actor* child : m_children)
      child->SetOnStage(GetOnStage());
}

void Group::Update(float delta)
{
   Actor::Update(delta);
   if (!GetOnStage())
      return;
   size_t i = 0;
   const size_t c = m_children.size();
   while (i < c)
   {
      Actor* child = m_children[i];
      child->Update(delta);
      if (child == m_children[i])
         i++;
   }
}

void Group::Draw(VP::SurfaceGraphics* pGraphics)
{
   if (GetVisible())
   {
      pGraphics->TranslateTransform(static_cast<int>(GetX()), static_cast<int>(GetY()));
      if (m_clip)
      {
         pGraphics->SetClip({ 0, 0, static_cast<int>(GetWidth()), static_cast<int>(GetHeight()) });

         Actor::Draw(pGraphics);

         for (Actor* child : m_children)
            child->Draw(pGraphics);

         pGraphics->ResetClip();
      }
      else
      {
         Actor::Draw(pGraphics);

         for (Actor* child : m_children)
            child->Draw(pGraphics);
      }
      pGraphics->TranslateTransform(-static_cast<int>(GetX()), -static_cast<int>(GetY()));
   }
}

Group* Group::GetRoot()
{
   Group* root = this;
   while (root->GetParent() != nullptr)
      root = root->GetParent();
   return root;
}

Actor* Group::Get(const string& name)
{
   if (GetName() == name)
      return this;

   if (GetFlexDMD()->GetRuntimeVersion() <= 1008)
   {
      for (Actor* child : m_children)
      {
         if (child->GetName() == name)
            return child;
         if (child->GetType() == Actor::AT_Group)
         {
            Group* group = static_cast<Group*>(child);
            Actor* found = group->Get(name);
            if (found)
               return found;
         }
      }
   }
   else
   {
      size_t pos = name.find('/');
      if (pos == string::npos)
      {
         // direct child node search 'xx'
         for (Actor* child : m_children)
         {
            if (child->GetName() == name)
               return child;
         }
      }
      else if (pos == 0)
      {
         // absolute path from root '/xx/yy/zz', note that stage node is named 'Stage'
         return GetRoot()->Get(name.substr(1));
      }
      else
      {
         // relative path from current group 'xx/yy/zz'
         string groupName = name.substr(0, pos);
         for (Actor* child : m_children)
         {
            if ((child->GetType() == Actor::AT_Group) && (child->GetName() == groupName))
               return static_cast<Group*>(child)->Get(name.substr(pos + 1));
         }
      }
   }
   
   // PLOGW.printf("Actor %s not found in children of %s", name.c_str(), GetName().c_str());

   return NULL;
}

Group* Group::GetGroup(const string& Name)
{
   Actor* actor = Get(Name);
   if ((actor != nullptr) && (actor->GetType() == Actor::AT_Group))
      return dynamic_cast<Group*>(actor);
   return nullptr;
}

Frame* Group::GetFrame(const string& Name)
{
   Actor* actor = Get(Name);
   if ((actor != nullptr) && (actor->GetType() == Actor::AT_Frame))
      return dynamic_cast<Frame*>(actor);
   return nullptr;
}

Label* Group::GetLabel(const string& Name)
{
   Actor* actor = Get(Name);
   if ((actor != nullptr) && (actor->GetType() == Actor::AT_Label))
      return dynamic_cast<Label*>(actor);
   return nullptr;
}

AnimatedActor* Group::GetVideo(const string& Name)
{
   Actor* actor = Get(Name);
   if ((actor != nullptr) && (actor->GetType() == Actor::AT_AnimatedActor))
      return dynamic_cast<AnimatedActor*>(actor);
   return nullptr;
}

Image* Group::GetImage(const string& Name)
{
   Actor* actor = Get(Name);
   if ((actor != nullptr) && (actor->GetType() == Actor::AT_Image))
      return dynamic_cast<Image*>(actor);
   return nullptr;
}

void Group::RemoveAll()
{
   //PLOGD.printf("Remove all children %s", GetName().c_str());
   for (Actor* child : m_children)
   {
      child->SetParent(nullptr);
      child->SetOnStage(false);
      child->Release();
   }
   m_children.clear();
}

void Group::AddActor(Actor* actor)
{
   if (!actor)
      return;
   actor->AddRef();
   actor->Remove();
   actor->SetParent(this);
   m_children.push_back(actor);
   actor->SetOnStage(GetOnStage());
}

void Group::AddActorAt(Actor* actor, int index)
{
   if (!actor)
      return;
   actor->AddRef();
   actor->Remove();
   actor->SetParent(this);
   m_children.insert(m_children.begin() + index, actor);
   actor->SetOnStage(GetOnStage());
}

void Group::RemoveActor(Actor* actor)
{
   if (!actor)
      return;

   actor->SetParent(NULL);
   m_children.erase(std::remove_if(m_children.begin(), m_children.end(), [actor](Actor* p) { return p == actor; }), m_children.end());
   actor->SetOnStage(false);
   actor->Release();
}
