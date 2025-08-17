// license:GPLv3+

#include "core/stdafx.h"

#include "imgui/imgui_internal.h" // Needed for FindRenderedTextEnd (should be adapted when this function will refactored in ImGui)

#include "NotificationOverlay.h"

unsigned int NotificationOverlay::PushNotification(const string &message, const int lengthMs, const unsigned int reuseId)
{
   auto notif = std::ranges::find_if(m_notifications, [reuseId](const Notification &n) { return n.id == reuseId; });
   if (notif != m_notifications.end())
   {
      notif->message = message;
      notif->disappearTick = msec() + lengthMs;
      return reuseId;
   }
   else
   {
      m_notifications.emplace_back(m_nextNotificationIs, message, msec() + lengthMs);
      m_nextNotificationIs++;
      return m_nextNotificationIs - 1;
   }
}

void NotificationOverlay::Update(bool showNotification, ImFont * font)
{
   const uint32_t tick = msec();
   const auto& io = ImGui::GetIO();
   float notifY = io.DisplaySize.y * 0.25f;
   ImGui::PushFont(font, font->LegacySize);
   for (int i = static_cast<int>(m_notifications.size()) - 1; i >= 0; i--)
   {
      if (tick > m_notifications[i].disappearTick)
         m_notifications.erase(m_notifications.begin() + i);
      else if (showNotification)
         notifY += 10.f + RenderNotification(i, notifY);
   }
   ImGui::PopFont();
}

float NotificationOverlay::RenderNotification(int index, float posY) const {
   ImFont *const font = ImGui::GetFont();
   const ImFontBaked *const fontBaked = ImGui::GetFontBaked();

   constexpr float padding = 50.f;
   const float maxWidth = ImGui::GetIO().DisplaySize.x - padding;

   vector<string> lines;
   ImVec2 text_size(0, 0);

   string line;
   std::istringstream iss(m_notifications[index].message);
   while (std::getline(iss, line))
   {
      if (line.empty())
      {
         lines.push_back(line);
         continue;
      }
      const char *textEnd = line.c_str();
      while (*textEnd)
      {
         const char *nextLineTextEnd = ImGui::FindRenderedTextEnd(textEnd, nullptr);
         ImVec2 lineSize = font->CalcTextSizeA(fontBaked->Size, FLT_MAX, 0.0f, textEnd, nextLineTextEnd);
         if (lineSize.x > maxWidth)
         {
            const char *wrapPoint = font->CalcWordWrapPositionA(font->Scale, textEnd, nextLineTextEnd, maxWidth);
            if (wrapPoint == textEnd)
               wrapPoint++;
            nextLineTextEnd = wrapPoint;
            lineSize = font->CalcTextSizeA(fontBaked->Size, FLT_MAX, 0.0f, textEnd, wrapPoint);
         }

         string newLine(textEnd, nextLineTextEnd);
         lines.push_back(newLine);

         if (lineSize.x > text_size.x)
            text_size.x = lineSize.x;

         textEnd = nextLineTextEnd;

         while (*textEnd == ' ')
            textEnd++;
      }
   }
   text_size.x += (padding / 2.f);
   text_size.y = ((float)lines.size() * ImGui::GetTextLineHeightWithSpacing()) + (padding / 2.f);

   constexpr ImGuiWindowFlags window_flags
      = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
   ImGui::SetNextWindowBgAlpha(0.666f);
   ImGui::SetNextWindowPos(ImVec2((ImGui::GetIO().DisplaySize.x - text_size.x) / 2, posY));
   ImGui::SetNextWindowSize(text_size);
   ImGui::Begin(std::format("Notification{}", index).c_str(), nullptr, window_flags);
   for (const string &lline : lines)
   {
      ImVec2 lineSize = font->CalcTextSizeA(fontBaked->Size, FLT_MAX, 0.0f, lline.c_str());
      ImGui::SetCursorPosX((text_size.x - lineSize.x) / 2);
      ImGui::TextUnformatted(lline.c_str());
   }
   ImGui::End();
   return text_size.y;
}
