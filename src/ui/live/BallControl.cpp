// license:GPLv3+

#include "core/stdafx.h"

#include "BallControl.h"


void BallControl::LoadSettings(const Settings& settings)
{
   bool throwBalls = settings.LoadValueWithDefault(Settings::Editor, "ThrowBallsAlwaysOn"s, false);
   bool ballControl = settings.LoadValueWithDefault(Settings::Editor, "BallControlAlwaysOn"s, false);
   SetMode(ballControl, throwBalls);
}

void BallControl::SetMode(bool ballControl, bool throwBalls)
{
   using enum Mode;
   if (throwBalls && !ballControl)
      SetMode(ThrowNewBall);
   else if (throwBalls && ballControl)
      SetMode(ThrowDraggedBall);
   else if (ballControl)
      SetMode(DragBall);
   else
      SetMode(Disabled);
}

void BallControl::SetMode(Mode mode) {
   if (m_mode == mode)
      return;
   EndBallDrag();
   m_mode = mode;
}

void BallControl::Update(const int width, const int height)
{
   using enum Mode;
   const Player *const m_player = g_pplayer;
   const PinInput::InputState &inputState = m_player->m_pininput.GetInputState();
   const bool leftFlipperPressed = inputState.IsKeyPressed(eLeftFlipperKey, m_prevInputState);
   m_prevInputState = inputState;

   switch (m_mode)
   {
   case ThrowNewBall:
   case ThrowDraggedBall:
      if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
         HandleDestroyBall(width, height); 
      else if (ImGui::GetMouseDragDelta().x != 0.f || ImGui::GetMouseDragDelta().y != 0.f)
         HandleThrowBalls(width, height);
      break;

   case DragBall:
      if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && !leftFlipperPressed)
         HandleDragBall(width, height);
      break;
      
   case Disabled:
      break;
   }
   
   if (leftFlipperPressed)
      EndBallDrag();
}

void BallControl::HandleDragBall(const int width, const int height)
{
   Player *m_player = g_pplayer;
   Renderer *m_renderer = m_player->m_renderer;
   const PinTable *const m_live_table = m_player->m_ptable;

   // Note that ball control release is handled by pininput
   m_dragging = true;
   const ImVec2 mousePos = ImGui::GetMousePos();
   m_dragTarget = m_renderer->Get3DPointFrom2D(width, height, Vertex2D(mousePos.x, mousePos.y), m_draggedBall ? m_draggedBall->m_d.m_pos.z : 25.f);
   m_dragTarget.x = clamp(m_dragTarget.x, 0.f, m_live_table->m_right);
   m_dragTarget.y = clamp(m_dragTarget.y, 0.f, m_live_table->m_bottom);
   
   // Double click.  Move the ball directly to the target if possible.
   // Drop it from the glass height, so it will appear over any object (or on a raised playfield)
   if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && m_draggedBall && !m_draggedBall->m_d.m_lockedInKicker)
   {
      m_draggedBall->m_d.m_pos.x = m_dragTarget.x;
      m_draggedBall->m_d.m_pos.y = m_dragTarget.y;
      m_draggedBall->m_d.m_pos.z = m_live_table->m_glassTopHeight;
      m_draggedBall->m_d.m_vel.x = 0.0f;
      m_draggedBall->m_d.m_vel.y = 0.0f;
      m_draggedBall->m_d.m_vel.z = -1000.0f;
   }
}

void BallControl::HandleDestroyBall(const int width, const int height) const
{
   Player *m_player = g_pplayer;
   Renderer *m_renderer = m_player->m_renderer;

   const ImVec2 mousePos = ImGui::GetMousePos();
   const Vertex3Ds vertex = m_renderer->Get3DPointFrom2D(width, height, Vertex2D(mousePos.x, mousePos.y), 25.f);
   for (size_t i = 0; i < m_player->m_vball.size(); i++)
   {
      HitBall *const pBall = m_player->m_vball[i];
      const float dx = fabsf(vertex.x - pBall->m_d.m_pos.x);
      const float dy = fabsf(vertex.y - pBall->m_d.m_pos.y);
      if (dx < pBall->m_d.m_radius * 2.f && dy < pBall->m_d.m_radius * 2.f)
      {
         m_player->DestroyBall(pBall);
         break;
      }
   }
}

void BallControl::HandleThrowBalls(const int width, const int height)
{
   Player *m_player = g_pplayer;
   Renderer *m_renderer = m_player->m_renderer;
   const PinTable * const m_live_table = m_player->m_ptable;

   const ImVec2 mouseDrag = ImGui::GetMouseDragDelta();
   ImVec2 mousePos = ImGui::GetMousePos();
   ImVec2 mouseInitalPos = mousePos - mouseDrag;

   // Throw direction/speed overlay
   ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
   ImGui::SetNextWindowPos(ImVec2(0, 0));
   ImGui::PushStyleColor(ImGuiCol_WindowBg, 0);
   ImGui::PushStyleColor(ImGuiCol_Border, 0);
   ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
   ImGui::Begin("Ball throw overlay", nullptr,
      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoSavedSettings
         | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus);
   ImGui::GetWindowDrawList()->AddLine(mouseInitalPos, mousePos, IM_COL32(255, 128, 0, 255));
   ImGui::End();
   ImGui::PopStyleVar();
   ImGui::PopStyleColor(2);

   if (!ImGui::IsMouseReleased(ImGuiMouseButton_Left) && !ImGui::IsMouseReleased(ImGuiMouseButton_Middle))
      return;

   // Adjust mouse position based on UI orientation
   switch (m_liveUI.GetUIOrientation())
   {
   case 0:
      break;
   case 1:
      mousePos = ImVec2(static_cast<float>(width) - mousePos.y, mousePos.x);
      mouseInitalPos = ImVec2(static_cast<float>(width) - mouseInitalPos.y, mouseInitalPos.x);
      break;
   case 2:
      mousePos = ImVec2(mousePos.x, static_cast<float>(height) - mousePos.y);
      mouseInitalPos = ImVec2(mouseInitalPos.x, static_cast<float>(height) - mouseInitalPos.y);
      break;
   case 3:
      mousePos = ImVec2(mousePos.y, static_cast<float>(height) - mousePos.x);
      mouseInitalPos = ImVec2(mouseInitalPos.y, static_cast<float>(height) - mouseInitalPos.x);
      break;
   default:
      assert(false);
      return;
   }
   const Vertex3Ds throwCenter = m_renderer->Get3DPointFrom2D(width, height, Vertex2D(mouseInitalPos.x, mouseInitalPos.y), 25.f);
   const Vertex3Ds throwTarget = m_renderer->Get3DPointFrom2D(width, height, Vertex2D(mousePos.x, mousePos.y), 25.f);

   const float vx = (throwTarget.x - throwCenter.x) * 0.25f;
   const float vy = (throwTarget.y - throwCenter.y) * 0.25f;

   HitBall *grabbedBall = m_mode == Mode::ThrowDraggedBall ? m_draggedBall : nullptr;
   const bool isPlayfieldThrow = ImGui::IsMouseReleased(ImGuiMouseButton_Left);
   if (isPlayfieldThrow && grabbedBall == nullptr)
   {
      for (size_t i = 0; i < m_player->m_vball.size() && grabbedBall == nullptr; i++)
      {
         HitBall *const pBall = m_player->m_vball[i];
         const float dx = fabsf(throwCenter.x - pBall->m_d.m_pos.x);
         const float dy = fabsf(throwCenter.y - pBall->m_d.m_pos.y);
         if (dx < pBall->m_d.m_radius * 2.f && dy < pBall->m_d.m_radius * 2.f)
            grabbedBall = pBall;
      }
   }
   
   if (grabbedBall)
   {
      grabbedBall->m_d.m_pos.x = throwCenter.x;
      grabbedBall->m_d.m_pos.y = throwCenter.y;
      grabbedBall->m_d.m_vel.x = vx;
      grabbedBall->m_d.m_vel.y = vy;
   }
   else
   {
      const float z = isPlayfieldThrow ? 0.f : m_live_table->m_glassTopHeight;
      HitBall *const pball = m_player->CreateBall(throwCenter.x, throwCenter.y, z, vx, vy, 0, (float)m_player->m_debugBallSize * 0.5f, m_player->m_debugBallMass);
      pball->m_pBall->AddRef();
   }
}
