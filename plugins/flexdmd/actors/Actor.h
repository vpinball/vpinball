#pragma once

#include "common.h"
#include "Layout.h"
#include "ScriptablePlugin.h"
#include "SurfaceGraphics.h"

#include <SDL3/SDL_surface.h>

class Group;
class FlexDMD;
class Action;
class ActionFactory;

class Actor
{
public:
   Actor(FlexDMD *pFlexDMD, const string& name);
   ~Actor();

   PSC_IMPLEMENT_REFCOUNT()

   enum ActorType
   {
      AT_Actor,
      AT_Group,
      AT_Frame,
      AT_Label,
      AT_Image,
      AT_AnimatedActor
   };
   virtual ActorType GetType() const { return AT_Actor; }

   // FIXME why can this be changed ?
   void SetFlexDMD(FlexDMD *pFlexDMD) { m_pFlexDMD = pFlexDMD; }
   FlexDMD *GetFlexDMD() const { return m_pFlexDMD; }
   
   void SetName(const string &szName) { m_name = szName; }
   const string &GetName() const { return m_name; }
   
   void SetBounds(float x, float y, float Width, float Height) { m_x = x; m_y = y; m_width = Width; m_height = Height; }
   void SetPosition(float x, float y) { m_x = x; m_y = y; }
   void SetAlignedPosition(float x, float y, Alignment alignment) { m_x = x; m_y = y; Layout::ApplyAlign(alignment, m_width, m_height, m_x, m_y); }

   void SetSize(float Width, float Height) { m_width = Width; m_height = Height; }
   void Pack() { m_width = GetPrefWidth(); m_height = GetPrefHeight(); }
   float GetX() const { return m_x; }
   void SetX(float x) { m_x = x; }
   float GetY() const { return m_y; }
   void SetY(float y) { m_y = y; }
   float GetWidth() const { return m_width; }
   void SetWidth(float width) { m_width = width; }
   float GetHeight() const { return m_height; }
   void SetHeight(float height) { m_height = height; }
   virtual float GetPrefWidth() const { return m_prefWidth; }
   void SetPrefWidth(float prefWidth) { m_prefWidth = prefWidth; }
   virtual float GetPrefHeight() const { return m_prefHeight; }
   void SetPrefHeight(float prefHeight) { m_prefHeight = prefHeight; }
   
   ActionFactory* GetActionFactory() const { return m_pActionFactory; };
   void AddAction(Action* action) { m_actions.push_back(action); }
   void ClearActions() { m_actions.clear(); }

   bool GetVisible() const { return m_visible; }
   virtual void SetVisible(bool visible) { m_visible = visible; }

   bool GetClearBackground() const { return m_clearBackground; };
   void SetClearBackground(bool v) { m_clearBackground = v; }

   bool GetOnStage() const { return m_onStage; }
   void SetOnStage(bool onStage) { m_onStage = onStage; OnStageStateChanged(); }

   bool GetFillParent() const { return m_fillParent; }
   void SetFillParent(bool fillParent) { m_fillParent = fillParent; }

   Group *GetParent() const { return m_parent; }
   void SetParent(Group *parent) { m_parent = parent; }
   void Remove();

   virtual void OnStageStateChanged() { }
   
   virtual void Update(float secondsElapsed);
   
   virtual void Draw(VP::SurfaceGraphics *pGraphics);
   
protected:
   ActionFactory *m_pActionFactory;

private:
   FlexDMD *m_pFlexDMD;
   string m_name;
   vector<Action*> m_actions;
   bool m_onStage = false;
   float m_x = 0.f;
   float m_y = 0.f;
   float m_width = 0.f;
   float m_height = 0.f;
   Group *m_parent = nullptr;
   bool m_fillParent = false;
   bool m_clearBackground = false;
   float m_prefWidth = 0.f;
   float m_prefHeight = 0.f;
   bool m_visible = true;
};
