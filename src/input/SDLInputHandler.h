#pragma once

#include <filesystem>

#include "pininput.h"

class SDLInputHandler final : public InputHandler
{
public:
   explicit SDLInputHandler(PinInput& pininput)
      : m_pininput(pininput)
   {
      PLOGI << "SDL input handler registered";
      if (!SDL_InitSubSystem(SDL_INIT_GAMEPAD | SDL_INIT_HAPTIC | SDL_INIT_JOYSTICK))
      {
         PLOGE << "SDL_InitSubSystem(SDL_INIT_GAMEPAD | SDL_INIT_HAPTIC | SDL_INIT_JOYSTICK) failed: " << SDL_GetError();
         exit(1);
      }
      const string path = g_pvp->m_szMyPrefPath + "gamecontrollerdb.txt";
      if (!std::filesystem::exists(path))
         std::filesystem::copy(g_pvp->m_szMyPath + "assets" + PATH_SEPARATOR_CHAR + "Default_gamecontrollerdb.txt", path);
      const int count = SDL_AddGamepadMappingsFromFile(path.c_str());
      if (count > 0)
      {
         PLOGI.printf("Game controller mappings added: count=%d, path=%s", count, path.c_str());
      }
      else
      {
         PLOGI.printf("No game controller mappings added: path=%s", path.c_str());
      }
   }

   ~SDLInputHandler() override
   {
      ReleaseSDLDevice();
      SDL_QuitSubSystem(SDL_INIT_GAMEPAD | SDL_INIT_HAPTIC | SDL_INIT_JOYSTICK);
   }

   void PlayRumble(const float lowFrequencySpeed, const float highFrequencySpeed, const int ms_duration) override
   {
      for (auto& gamepad : m_gamePads)
      {
         const SDL_PropertiesID props = SDL_GetGamepadProperties(gamepad);
         if (gamepad && SDL_GetBooleanProperty(props, SDL_PROP_GAMEPAD_CAP_RUMBLE_BOOLEAN, false))
            SDL_RumbleGamepad(gamepad, (Uint16)(saturate(lowFrequencySpeed) * 65535.f), (Uint16)(saturate(highFrequencySpeed) * 65535.f), ms_duration);
      }
      for (auto& haptic : m_haptics)
         SDL_PlayHapticRumble(haptic, saturate(max(lowFrequencySpeed, highFrequencySpeed)), ms_duration); //!! meh
   }

   void Update() override
   {
      // When SDL Video is used, SDL events are processed during the main application message loop, so we do not do it again here
      #if defined(ENABLE_SDL_INPUT) && !defined(ENABLE_SDL_VIDEO)
         SDL_Event e;
         while (SDL_PollEvent(&e) != 0)
            HandleSDLEvent(e);
      #endif
   }

   static constexpr uint64_t GetJoyId(const SDL_JoystickID& sdlId) {return static_cast<uint64_t>(0x200000000) | static_cast<uint64_t>(sdlId); }

   void HandleSDLEvent(SDL_Event& e)
   {
      static constexpr int axisMultiplier[] = { 2, 2, 2, 2, 256, 256 }; // NOTE - this is a hard-coded assumption that JOYRANGE is -65536..+65536
      switch (e.type)
      {
      case SDL_EVENT_KEY_DOWN:
      case SDL_EVENT_KEY_UP:
         if (e.key.repeat == 0)
         {
            const unsigned int dik = get_dik_from_sdlk(e.key.key);
            if (dik != ~0u)
               m_pininput.PushKeyboardEvent(dik, e.type == SDL_EVENT_KEY_DOWN);
         }
         break;

      case SDL_EVENT_GAMEPAD_ADDED:
      case SDL_EVENT_GAMEPAD_REMOVED:
         RefreshSDLDevice();
         break;

      case SDL_EVENT_GAMEPAD_AXIS_MOTION:
         if (e.gaxis.axis < 6)
            m_pininput.PushJoystickAxisEvent(GetJoyId(e.gaxis.which), e.gaxis.axis + 1, e.gaxis.value * axisMultiplier[e.gaxis.axis]);
         break;

      case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
      case SDL_EVENT_GAMEPAD_BUTTON_UP:
         if (e.gbutton.button < 32)
            m_pininput.PushJoystickButtonEvent(GetJoyId(e.gaxis.which), e.gbutton.button, e.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN);
         break;

      case SDL_EVENT_JOYSTICK_ADDED:
      case SDL_EVENT_JOYSTICK_REMOVED:
         RefreshSDLDevice();
         break;

      case SDL_EVENT_JOYSTICK_AXIS_MOTION:
         if (e.jaxis.axis < 6)
            m_pininput.PushJoystickAxisEvent(GetJoyId(e.gaxis.which), e.gaxis.axis + 1, e.gaxis.value * axisMultiplier[e.gaxis.axis]);
         break;

      case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
      case SDL_EVENT_JOYSTICK_BUTTON_UP:
         if (e.jbutton.button < 32)
            m_pininput.PushJoystickButtonEvent(GetJoyId(e.gaxis.which), e.gbutton.button, e.type == SDL_EVENT_JOYSTICK_BUTTON_DOWN);
         break;

      #if (defined(__APPLE__) && (defined(TARGET_OS_IOS) && TARGET_OS_IOS)) || defined(__ANDROID__)
         case SDL_EVENT_FINGER_DOWN:
         case SDL_EVENT_FINGER_UP:
         {
            POINT point;
            point.x = (int)((float)g_pplayer->m_playfieldWnd->GetWidth() * e.tfinger.x);
            point.y = (int)((float)g_pplayer->m_playfieldWnd->GetHeight() * e.tfinger.y);
            for (unsigned int i = 0; i < MAX_TOUCHREGION; ++i)
            {
               if ((g_pplayer->m_touchregion_pressed[i] != (e.type == SDL_EVENT_FINGER_DOWN))
                  && Intersect(touchregion[i], g_pplayer->m_playfieldWnd->GetWidth(), g_pplayer->m_playfieldWnd->GetHeight(), point,
                     fmodf(g_pplayer->m_ptable->mViewSetups[g_pplayer->m_ptable->m_BG_current_set].mViewportRotation, 360.0f) != 0.f))
               {
                  g_pplayer->m_touchregion_pressed[i] = (e.type == SDL_EVENT_FINGER_DOWN);
                  m_pininput.PushActionEvent(touchActionMap[i], g_pplayer->m_touchregion_pressed[i]);
               }
            }
         }
         break;
      #endif
      }
   }

private:
   void RefreshSDLDevice()
   {
      // Close any previously opened devices
      ReleaseSDLDevice();

      // Get list of all connected devices
      int joystick_count = 0;
      SDL_JoystickID* const joystick_ids = SDL_GetJoysticks(&joystick_count);
      if (joystick_count > 0)
      {
         for (int idx = 0; idx < joystick_count; ++idx)
         {
            // Log the device we found
            PLOGI.printf("Input device found: id=%d name=%s", joystick_ids[idx], SDL_GetJoystickNameForID(joystick_ids[idx]));

            // Process each device as either a gamepad or joystick
            if (SDL_IsGamepad(joystick_ids[idx]))
            {
               // Skip Apple Remote on iOS/tvOS devices
               #if defined(__APPLE__) && ((defined(TARGET_OS_IOS) && TARGET_OS_IOS) || (defined(TARGET_OS_TV) && TARGET_OS_TV))
                  if (!lstrcmpi(SDL_GetGamepadNameForID(joystick_ids[idx]), "Remote"))
                     continue;
               #endif

               // Try to open as gamepad
               SDL_Gamepad* const gamePad = SDL_OpenGamepad(joystick_ids[idx]);
               if (gamePad)
               {
                  m_gamePads.push_back(gamePad);
                  m_pininput.SetupJoyMapping(GetJoyId(joystick_ids[idx]), PinInput::InputLayout::Gamepad);
                  const SDL_PropertiesID props = SDL_GetGamepadProperties(gamePad);
                  PLOGI.printf("Processing as Gamepad: %d axes, %d buttons, rumble=%s",
                     6, // Standard gamepad has 6 axes
                     15, // Standard gamepad has 15 buttons
                     SDL_GetBooleanProperty(props, SDL_PROP_GAMEPAD_CAP_RUMBLE_BOOLEAN, false) ? "true" : "false");
               }
            }
            else
            {
               // Try to open as standard joystick
               SDL_Joystick* joystick = SDL_OpenJoystick(joystick_ids[idx]);
               if (joystick)
               {
                  m_joysticks.push_back(joystick);
                  m_pininput.SetupJoyMapping(GetJoyId(joystick_ids[idx]), PinInput::InputLayout::Generic);
                  // Check if joystick supports force feedback
                  SDL_Haptic* haptic = nullptr;
                  if (SDL_IsJoystickHaptic(joystick))
                  {
                     haptic = SDL_OpenHapticFromJoystick(joystick);
                     if (haptic)
                     {
                        if (!SDL_InitHapticRumble(haptic))
                        {
                           ShowError(SDL_GetError());
                           SDL_CloseHaptic(haptic);
                        }
                        else
                        {
                           m_haptics.push_back(haptic);
                        }
                     }
                  }
                  PLOGI.printf("Processing as Joystick: %d axes, %d buttons, rumble=%s", SDL_GetNumJoystickAxes(joystick), SDL_GetNumJoystickButtons(joystick), haptic ? "true" : "false");
               }
            }
         }
      }
   }

   void ReleaseSDLDevice()
   {
      for (auto gamePad : m_gamePads)
      {
         m_pininput.UnmapJoy(GetJoyId(SDL_GetGamepadID(gamePad)));
         SDL_CloseGamepad(gamePad);
      }
      m_gamePads.clear();
      for (auto haptic : m_haptics)
         SDL_CloseHaptic(haptic);
      m_haptics.clear();
      for (auto joystick : m_joysticks)
      {
         m_pininput.UnmapJoy(GetJoyId(SDL_GetJoystickID(joystick)));
         SDL_CloseJoystick(joystick);
      }
      m_joysticks.clear();
      for (auto gamePad : m_gamePads)
         SDL_CloseGamepad(gamePad);
      m_gamePads.clear();
   }

private:
   PinInput& m_pininput;

   vector<SDL_Gamepad*> m_gamePads;
   vector<SDL_Joystick*> m_joysticks;
   vector<SDL_Haptic*> m_haptics;
};
