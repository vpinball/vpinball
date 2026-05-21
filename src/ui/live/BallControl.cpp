// license:GPLv3+

#include "core/stdafx.h"
#include "BallControl.h"

#include "parts/ball.h"
#include "renderer/Renderer.h"
#include "math/ModelViewProj.h"
#include "ui/live/LiveUI.h"

void BallControl::LoadSettings(const Settings& settings)
{
   const bool throwBalls = settings.GetEditor_ThrowBallsAlwaysOn();
   const bool ballControl = settings.GetEditor_BallControlAlwaysOn();
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

void BallControl::SetMode(Mode mode)
{
   if (m_mode == mode)
      return;
   EndBallDrag();
   m_mode = mode;
}

bool BallControl::IsSelectedBallDraggable() const
{
   // A ball is only a valid drag target while it is in active play: visible and not held in a kicker
   // (trough/saucer/lock). Checked at use time rather than registration because a ball's state changes
   // (e.g. trough balls are briefly unlocked while being shuffled along the trough). Matches the physics
   // engine, which skips locked balls in HitBall::UpdateVelocities.
   return m_draggedBall != nullptr && m_draggedBall->m_d.m_visible && !m_draggedBall->m_hitBall.m_d.m_lockedInKicker;
}

void BallControl::Update(const int width, const int height)
{
   using enum Mode;
   const Player *const player = g_pplayer;
   const InputManager::ActionState &inputState = player->m_pininput.GetActionState();
   const bool leftFlipperPressed = inputState.IsKeyPressed(player->m_pininput.GetLeftFlipperActionId(), m_prevActionState);
   m_prevActionState = inputState;

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
      if (IsSelectedBallDraggable() && ImGui::IsMouseDown(ImGuiMouseButton_Left) && !leftFlipperPressed)
         HandleDragBall(width, height);
      break;
      
   case Disabled:
      break;
   }
   
   if (leftFlipperPressed)
      EndBallDrag();

   // Transient visual indicator (only while the FPS/stats overlay is shown): a ring on the current drag
   // target so it is clear which ball is grabbed (yellow = grabbable, red = locked in a kicker/trough and
   // cannot be dragged, green = being dragged). Shown at full alpha when a ball is selected or while it is
   // dragged, then fading out over a few seconds.
   constexpr float ringFadeSeconds = 3.f;
   constexpr int ringPeakAlpha = 200;
   if (m_draggedBall != nullptr && m_draggedBall != m_ringHighlightBall) // new selection
      m_ringHighlight = ringFadeSeconds;
   m_ringHighlightBall = m_draggedBall;
   if (m_dragging) // keep fully visible while dragging
      m_ringHighlight = ringFadeSeconds;

   if (m_liveUI.IsShowingFPSDetails() && (m_mode == DragBall || m_mode == ThrowDraggedBall)
      && m_draggedBall != nullptr && m_ringHighlight > 0.f)
   {
      const Matrix3D mvp = player->m_renderer->GetMVP().GetModelViewProj(0);
      auto project = [&mvp, width, height](const Vertex3Ds& v, ImVec2& out)
      {
         const float xp = mvp._11 * v.x + mvp._21 * v.y + mvp._31 * v.z + mvp._41;
         const float yp = mvp._12 * v.x + mvp._22 * v.y + mvp._32 * v.z + mvp._42;
         const float wp = mvp._14 * v.x + mvp._24 * v.y + mvp._34 * v.z + mvp._44;
         if (wp <= 1e-10f)
            return false;
         out = ImVec2((wp + xp) * (static_cast<float>(width) * 0.5f) / wp, (wp - yp) * (static_cast<float>(height) * 0.5f) / wp);
         return true;
      };
      ImVec2 center, edge;
      if (project(m_draggedBall->GetPosition(), center)
         && project(m_draggedBall->GetPosition() + Vertex3Ds(m_draggedBall->GetRadius(), 0.f, 0.f), edge))
      {
         const float dx = edge.x - center.x;
         const float dy = edge.y - center.y;
         float radius = sqrtf(dx * dx + dy * dy) * 1.5f;
         if (radius < 10.f)
            radius = 10.f;
         const int alpha = static_cast<int>(ringPeakAlpha * clamp(m_ringHighlight / ringFadeSeconds, 0.f, 1.f));
         const bool locked = m_draggedBall->m_hitBall.m_d.m_lockedInKicker;
         const ImU32 col = m_dragging ? IM_COL32(80, 255, 80, alpha)
            : (locked ? IM_COL32(255, 80, 80, alpha) : IM_COL32(255, 220, 60, alpha));
         ImGui::GetForegroundDrawList()->AddCircle(center, radius, col, 24, 3.f);
      }
   }
   m_ringHighlight -= ImGui::GetIO().DeltaTime;
   if (m_ringHighlight < 0.f)
      m_ringHighlight = 0.f;
}

void BallControl::HandleDragBall(const int width, const int height)
{
   Player * const player = g_pplayer;
   Renderer * const m_renderer = player->m_renderer;
   const PinTable *const live_table = player->m_ptable;

   // Note that ball control release is handled by pininput
   m_dragging = true;
   const ImVec2 mousePos = ImGui::GetMousePos();
   m_dragTarget = m_renderer->Get3DPointFrom2D(width, height, Vertex2D(mousePos.x, mousePos.y), m_draggedBall ? m_draggedBall->GetPosition().z : DEFAULT_BALL_SIZE);
   m_dragTarget.x = clamp(m_dragTarget.x, 0.f, live_table->m_right);
   m_dragTarget.y = clamp(m_dragTarget.y, 0.f, live_table->m_bottom);
   
   // Double click.  Move the ball directly to the target if possible.
   // Drop it from the glass height, so it will appear over any object (or on a raised playfield)
   if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && m_draggedBall && !m_draggedBall->m_hitBall.m_d.m_lockedInKicker)
   {
      m_draggedBall->SetPosition({ m_dragTarget.x, m_dragTarget.y, live_table->m_glassTopHeight });
      m_draggedBall->SetVelocity({ 0.f, 0.f, -1000.f });
   }
}

void BallControl::HandleDestroyBall(const int width, const int height) const
{
   Player * const player = g_pplayer;
   Renderer * const renderer = player->m_renderer;

   const ImVec2 mousePos = ImGui::GetMousePos();
   const Vertex3Ds vertex = renderer->Get3DPointFrom2D(width, height, Vertex2D(mousePos.x, mousePos.y), DEFAULT_BALL_SIZE);
   for (size_t i = 0; i < player->m_vball.size(); i++)
   {
      Ball *const pBall = player->m_vball[i];
      const float dx = fabsf(vertex.x - pBall->GetPosition().x);
      const float dy = fabsf(vertex.y - pBall->GetPosition().y);
      if (dx < pBall->GetRadius() * 2.f && dy < pBall->GetRadius() * 2.f)
      {
         player->DestroyBall(pBall);
         break;
      }
   }
}

void BallControl::HandleThrowBalls(const int width, const int height)
{
   Player * const player = g_pplayer;
   Renderer * const renderer = player->m_renderer;
   const PinTable * const live_table = player->m_ptable;

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
   const Vertex3Ds throwCenter = renderer->Get3DPointFrom2D(width, height, Vertex2D(mouseInitalPos.x, mouseInitalPos.y), DEFAULT_BALL_SIZE);
   const Vertex3Ds throwTarget = renderer->Get3DPointFrom2D(width, height, Vertex2D(mousePos.x, mousePos.y), DEFAULT_BALL_SIZE);

   const float vx = (throwTarget.x - throwCenter.x) * 0.25f;
   const float vy = (throwTarget.y - throwCenter.y) * 0.25f;

   Ball *grabbedBall = m_mode == Mode::ThrowDraggedBall ? m_draggedBall : nullptr;
   const bool isPlayfieldThrow = ImGui::IsMouseReleased(ImGuiMouseButton_Left);
   if (isPlayfieldThrow && grabbedBall == nullptr)
   {
      for (size_t i = 0; i < player->m_vball.size(); i++)
      {
         Ball *const pBall = player->m_vball[i];
         const float dx = fabsf(throwCenter.x - pBall->GetPosition().x);
         const float dy = fabsf(throwCenter.y - pBall->GetPosition().y);
         if (dx < pBall->GetRadius() * 2.f && dy < pBall->GetRadius() * 2.f)
         {
            grabbedBall = pBall;
            break;
         }
      }
   }
   
   if (grabbedBall)
   {
      grabbedBall->SetPosition({ throwCenter.x, throwCenter.y, grabbedBall->GetPosition().z });
      grabbedBall->SetVelocity({ vx, vy, grabbedBall->GetVelocity().z });
   }
   else
   {
      const float z = isPlayfieldThrow ? 0.f : live_table->m_glassTopHeight;
      player->CreateBall(throwCenter.x, throwCenter.y, z, vx, vy, 0,
         (float)live_table->m_settings.GetEditor_ThrowBallSize() * 0.5f, live_table->m_settings.GetEditor_ThrowBallMass());
   }
}
