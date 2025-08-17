// license:GPLv3+

#pragma once

class EscSplashModal final
{
public:
   explicit EscSplashModal(LiveUI& liveUI)
      : m_liveUI(liveUI)
   {
   }
   ~EscSplashModal() = default;

   void Open();
   bool IsOpened() const { return ImGui::IsPopupOpen(ID_MODAL_SPLASH); }
   void Update();

private:
   static inline constexpr const char* ID_MODAL_SPLASH = "EscModalSplash";

   LiveUI& m_liveUI;
   ImVec2 m_initialDragPos;
   bool m_disable_esc = false;
   PinInput::InputState m_prevInputState { 0 };
   uint32_t m_openTimestamp = 0; // Used to delay keyboard shortcut
};
