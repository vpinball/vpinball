// license:GPLv3+

#include "core/stdafx.h"

#include "CabinetRender.h"

namespace VPX::InGameUI
{

void CabinetRender::Render(const ImVec4& rect, PinTable* table, const vec3& playerPos)
{
   float posx = rect.x;
   float posy = rect.y;
   float width = rect.z;
   float height = rect.w;

   const ImU32 cabColor = IM_COL32(255, 0, 255, 255);
   const ImU32 glassColor = IM_COL32(0, 128, 255, 255);
   const ImU32 pfColor = IM_COL32(0, 0, 255, 255);
   const ImU32 screenColor = IM_COL32(255, 255, 0, 255);

   const float scale = height / 200.f;

   const float realToVirtualScale = table->GetViewSetup().GetRealToVirtualScale(table);

   const float windowLength = table->m_settings.GetPlayer_ScreenWidth() * realToVirtualScale / table->GetViewSetup().mSceneScaleY;
   const float windowAngle = ANGTORAD(table->m_settings.GetPlayer_ScreenInclination());

   const float playfieldLength = VPUTOCM(table->m_bottom - table->m_top);

   const float pfToGlassAngle = atan2f(VPUTOCM(table->m_glassTopHeight - table->m_glassBottomHeight), playfieldLength);
   const float glassLength = playfieldLength / cosf(pfToGlassAngle);

   const float cabX = posx + 0.5f * width - 0.5f * playfieldLength * scale;

   ImVec2 screenOrig(cabX, posy - 75.f * scale);

   ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
   draw_list->AddRectFilled(ImVec2(posx, posy), ImVec2(posx + width, posy - height), IM_COL32(0,0,0,64));
   
   // Ground
   draw_list->AddLine(ImVec2(posx, posy), ImVec2(posx + width, posy), IM_COL32(0, 0, 0, 255), 2);

   // Cabinet
   if ((table->GetViewMode() == ViewSetupID::BG_FULLSCREEN) && (table->GetViewSetup().mMode == VLM_WINDOW))
   {
      ImVec2 lockbar = ImVec2(cabX, posy - table->m_settings.GetPlayer_LockbarHeight() * scale);
      ImVec2 cabFrontBottom = lockbar + ImVec2(0.f, 39.f * scale);
      ImVec2 legFrontBottom(cabFrontBottom.x - 3.5f * scale, posy);

      // Playfield & Glass
      const float windowToPfAngle = atan2f(VPUTOCM(table->GetViewSetup().mWindowTopZOfs - table->GetViewSetup().mWindowBottomZOfs), playfieldLength);
      const float playfieldAngle = windowAngle - windowToPfAngle;
      const float glassAngle = playfieldAngle + pfToGlassAngle;
      ImVec2 glassBottom = lockbar + ImVec2(5.f * scale, 0.f);
      ImVec2 glassTop = glassBottom;
      glassTop.x += glassLength * cosf(glassAngle) * scale;
      glassTop.y -= glassLength * sinf(glassAngle) * scale;
      ImVec2 pfBottom = glassBottom;
      pfBottom.x += VPUTOCM(table->m_glassBottomHeight) * cosf(playfieldAngle - (float)(M_PI * 0.5)) * scale;
      pfBottom.y -= VPUTOCM(table->m_glassBottomHeight) * sinf(playfieldAngle - (float)(M_PI * 0.5)) * scale;
      ImVec2 pfTop = glassTop;
      pfTop.x += VPUTOCM(table->m_glassTopHeight) * cosf(playfieldAngle - (float)(M_PI * 0.5)) * scale;
      pfTop.y -= VPUTOCM(table->m_glassTopHeight) * sinf(playfieldAngle - (float)(M_PI * 0.5)) * scale;

      ImVec2 backglassFrontBottom = glassTop + ImVec2(-5.f * scale, 0.f);
      ImVec2 backglassBackBottom = backglassFrontBottom + ImVec2(15.f * scale, 0.f);
      ImVec2 backglassBackTop = ImVec2(backglassBackBottom.x, glassTop.y - 70.f * scale);
      ImVec2 backglassFrontTop = backglassBackTop + ImVec2(-25.f * scale, 0.f);

      ImVec2 cabBackBottom = ImVec2(backglassBackBottom.x + 2.5f * scale, cabFrontBottom.y);
      cabBackBottom.y -= (cabBackBottom.x - cabFrontBottom.x) * tanf(ANGTORAD(3.5f));
      ImVec2 legBackBottom(cabBackBottom.x + 5.f * scale, posy);
      ImVec2 legThickness(3.f * scale, 0.f);

      draw_list->AddLine(legFrontBottom, cabFrontBottom, cabColor, 2);
      draw_list->AddLine(legFrontBottom + legThickness, cabFrontBottom + legThickness, cabColor, 2);
      draw_list->AddLine(cabFrontBottom, lockbar, cabColor, 2);
      draw_list->AddLine(lockbar, glassBottom, cabColor, 2);
      draw_list->AddLine(glassTop, backglassFrontBottom, cabColor, 2);
      draw_list->AddLine(backglassFrontBottom, backglassFrontTop, cabColor, 2);
      draw_list->AddLine(backglassFrontTop, backglassBackTop, cabColor, 2);
      draw_list->AddLine(backglassBackTop, backglassBackBottom, cabColor, 2);
      draw_list->AddLine(backglassBackBottom, cabBackBottom, cabColor, 2);
      draw_list->AddLine(cabBackBottom, legBackBottom, cabColor, 2);
      draw_list->AddLine(cabBackBottom - legThickness, legBackBottom - legThickness, cabColor, 2);
      draw_list->AddLine(cabFrontBottom + legThickness, cabBackBottom - legThickness, cabColor, 2);
      //draw_list->AddLine(backglassFrontBottom, backglassBackBottom, cabColor, 2);

      draw_list->AddLine(glassBottom, glassTop, glassColor, 2); // Glass
      //draw_list->AddLine(glassBottom, pfBottom, pfColor, 2);
      //draw_list->AddLine(glassTop, pfTop, pfColor, 2);
      draw_list->AddLine(pfBottom, pfTop, pfColor, 2); // Playfield

      screenOrig = pfBottom;
      screenOrig.x += VPUTOCM(table->GetViewSetup().mWindowBottomZOfs) * cosf(playfieldAngle + (float)(M_PI * 0.5)) * scale;
      screenOrig.y -= VPUTOCM(table->GetViewSetup().mWindowBottomZOfs) * sinf(playfieldAngle + (float)(M_PI * 0.5)) * scale;
   }

   // Screen
   {
      const float delta = glassLength - windowLength;
      const float vOffset = delta * 0.5f * scale + table->GetViewSetup().mViewVOfs * scale;
      screenOrig.x += vOffset * cosf(windowAngle);
      screenOrig.y -= vOffset * sinf(windowAngle);
      ImVec2 screenEnd = screenOrig;
      screenEnd.x += windowLength * cosf(windowAngle) * scale;
      screenEnd.y -= windowLength * sinf(windowAngle) * scale;
      draw_list->AddLine(screenOrig, screenEnd, screenColor, 2);
   }

   // Player
   {
      // FIXME is it from screen origin or from glass (or from playfield) ?
      ImVec2 playerEye = screenOrig;
      playerEye.x += playerPos.y * scale;
      playerEye.y -= playerPos.z * scale;
      draw_list->AddCircleFilled(playerEye, 5.f, IM_COL32(255, 0, 0, 255));
   }
}

}
