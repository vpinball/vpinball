#include "stdafx.h"

#include "Group.h"
#include "Label.h"
#include "Frame.h"
#include "Image.h"
#include "AnimatedActor.h"

Group::Group(FlexDMD* pFlexDMD, const string& name) : Actor(pFlexDMD, name)
{
   m_clip = false;
}

Group::~Group()
{
}

void Group::OnStageStateChanged()
{
   for (Actor* child : m_children)
      child->SetOnStage(GetOnStage());
}

void Group::Update(float delta)
{
   Actor::Update(delta);
   if (!GetOnStage()) return;
   size_t i = 0;
   while (i < m_children.size()) {
      Actor* child = m_children[i];
      child->Update(delta);
      if (i < m_children.size() && child == m_children[i])
         i++;
   }
}

void Group::Draw(VP::SurfaceGraphics* pGraphics)
{
   if (GetVisible()) {
      pGraphics->TranslateTransform(GetX(), GetY());
      if (m_clip) {
         pGraphics->SetClip({0, 0, (int)GetWidth(), (int)GetHeight()});

         Actor::Draw(pGraphics);

         for (Actor* child : m_children)
            child->Draw(pGraphics);

         pGraphics->ResetClip();
      }
      else {
         Actor::Draw(pGraphics);

         for (Actor* child : m_children)
            child->Draw(pGraphics);
      }
      pGraphics->TranslateTransform(-GetX(), -GetY());
   }
}

Group* Group::GetRoot()
{
   Group* root = this;
   while (root->GetParent() != NULL)
      root = root->GetParent();
   return root;
}

Actor* Group::Get(string name)
{
   if (GetName() == name)
      return this;

   if (GetFlexDMD()->GetRuntimeVersion() <= 1008) {
      for (Actor* child : m_children) {
         if (child->GetName() == name)
            return child;

         Group* group = dynamic_cast<Group*>(child);
         if (group != NULL) {
            Actor* found = group->Get(name);
            if (found)
               return found;
         }
      }
   }
   else {
      size_t pos = name.find('/');
      if (pos == string::npos) {
         // direct child node search 'xx'
         for (Actor* child : m_children) {
            if (child->GetName() == name)
               return child;
         }
      }
      else if (pos == 0) {
         // absolute path from root '/xx/yy/zz', note that stage node is named 'Stage'
         return GetRoot()->Get(name.substr(1));
      }
      else
      {
         // relative path from current group 'xx/yy/zz'
         string groupName = name.substr(0, pos);
         for (Actor* child : m_children) {
            Group* group = dynamic_cast<Group*>(child);
            if (group != NULL && group->GetName() == groupName)
               return group->Get(name.substr(pos + 1));
         }
      }
   }

   // PLOGW.printf("Actor %s not found in children of %s", name.c_str(), GetName().c_str());

   return NULL;
}

STDMETHODIMP Group::get_Clip(VARIANT_BOOL *pRetVal)
{
   *pRetVal = m_clip ? VARIANT_TRUE : VARIANT_FALSE;

   return S_OK;
}

STDMETHODIMP Group::put_Clip(VARIANT_BOOL pRetVal)
{
   m_clip = (pRetVal == VARIANT_TRUE);

   return S_OK;
}

STDMETHODIMP Group::get_ChildCount(LONG *pRetVal)
{
   *pRetVal = m_children.size();

   return S_OK;
}

STDMETHODIMP Group::HasChild(BSTR Name,VARIANT_BOOL *pRetVal)
{
   *pRetVal = Get(MakeString(Name)) ? VARIANT_TRUE : VARIANT_FALSE;

   return S_OK;
}

STDMETHODIMP Group::GetGroup(BSTR Name,IGroupActor **pRetVal)
{
   Actor* actor = Get(MakeString(Name));
   if (actor)
      return dynamic_cast<Group*>(actor)->QueryInterface(IID_IGroupActor, (void**)pRetVal);

   *pRetVal = NULL;

   return S_OK;
}

STDMETHODIMP Group::GetFrame(BSTR Name,IFrameActor **pRetVal)
{
   Actor* actor = Get(MakeString(Name));
   if (actor)
      return dynamic_cast<Frame*>(actor)->QueryInterface(IID_IFrameActor, (void**)pRetVal);

   *pRetVal = NULL;

   return S_OK;
}

STDMETHODIMP Group::GetLabel(BSTR Name,ILabelActor **pRetVal)
{
   Actor* actor = Get(MakeString(Name));
   if (actor)
      return dynamic_cast<Label*>(actor)->QueryInterface(IID_ILabelActor, (void**)pRetVal);

   *pRetVal = NULL;

   return S_OK;
}

STDMETHODIMP Group::GetVideo(BSTR Name,IVideoActor **pRetVal)
{
   Actor* actor = Get(MakeString(Name));
   if (actor)
      return dynamic_cast<AnimatedActor*>(actor)->QueryInterface(IID_IVideoActor, (void**)pRetVal);

   *pRetVal = NULL;

   return S_OK;
}

STDMETHODIMP Group::GetImage(BSTR Name,IImageActor **pRetVal)
{
   Actor* actor = Get(MakeString(Name));
   if (actor)
      return dynamic_cast<Image*>(actor)->QueryInterface(IID_IImageActor, (void**)pRetVal);

   *pRetVal = NULL;

   return S_OK;
}

STDMETHODIMP Group::RemoveAll()
{
   PLOGD.printf("Remove all children %s", GetName().c_str());

   for (Actor* child : m_children) {
      child->SetParent(NULL);
      child->SetOnStage(false);
   }
   m_children.clear();

   return S_OK;
}

STDMETHODIMP Group::AddActor(IUnknown *child)
{
   Actor* actor = dynamic_cast<Actor*>(child);

   if (!actor)
      return E_FAIL;

   actor->Remove();
   actor->SetParent(this);
   m_children.push_back(actor);
   actor->SetOnStage(GetOnStage());

   return S_OK;
}

STDMETHODIMP Group::RemoveActor(IUnknown *child)
{
   Actor* actor = dynamic_cast<Actor*>(child);

   if (!actor)
      return E_FAIL;

   actor->SetParent(NULL);

   m_children.erase(std::remove_if(m_children.begin(), m_children.end(), [actor](Actor* p) {
      return p == actor;
   }), m_children.end());

   actor->SetOnStage(false);

   return S_OK;
}

void Group::AddActor(Actor* actor)
{
   if (!actor)
      return;

   actor->Remove();
   actor->SetParent(this);
   m_children.push_back(actor);
   actor->SetOnStage(GetOnStage());
}

void Group::AddActorAt(Actor* actor, int index)
{
   if (!actor)
      return;

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

   m_children.erase(std::remove_if(m_children.begin(), m_children.end(), [actor](Actor* p) {
      return p == actor;
   }), m_children.end());

   actor->SetOnStage(false);
}
