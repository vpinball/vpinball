// license:GPLv3+

#pragma once

#include "InputManager.h"

#include <vector>
#include <string>
#include <iostream>
#include <algorithm>

#include <openxr/openxr.h>


class XRInputHandler final : public InputManager::InputHandler
{
private:
   struct ActionTracker
   {
      std::string path;
      XrAction action;
      XrActionType type;
      bool isActive = false;
      uint16_t pinInputId;
   };

   template <size_t N> void CopyToXrBuffer(char (&dest)[N], const std::string_view& src)
   {
      size_t count = std::min(src.size(), N - 1);
      std::copy(src.begin(), src.begin() + count, dest);
      dest[count] = '\0';
   }

public:
   XRInputHandler(InputManager& pininput, XrInstance instance, XrSession session)
      : m_pininput(pininput)
      , m_instance(instance)
      , m_session(session)
   {
      XrActionSetCreateInfo actionSetInfo { XR_TYPE_ACTION_SET_CREATE_INFO };
      CopyToXrBuffer(actionSetInfo.actionSetName, "vpx_generic_input"s);
      CopyToXrBuffer(actionSetInfo.localizedActionSetName, "VPX Generic Input"s);
      xrCreateActionSet(m_instance, &actionSetInfo, &m_actionSet);

      m_joyId = m_pininput.RegisterDevice("OpenXR", InputManager::DeviceType::Joystick, "Controller");

      // There is no way to enumerate and listen for change in OpenXR, so we do it the other way: we define all known input path and listen for change on all of them
      std::vector<std::pair<std::string, XrActionType>> allPaths = { // Common Buttons (Boolean)
         { "/user/hand/left/input/select/click", XR_ACTION_TYPE_BOOLEAN_INPUT }, { "/user/hand/right/input/select/click", XR_ACTION_TYPE_BOOLEAN_INPUT },
         { "/user/hand/left/input/menu/click", XR_ACTION_TYPE_BOOLEAN_INPUT }, { "/user/hand/right/input/menu/click", XR_ACTION_TYPE_BOOLEAN_INPUT },
         { "/user/hand/left/input/squeeze/click", XR_ACTION_TYPE_BOOLEAN_INPUT }, { "/user/hand/right/input/squeeze/click", XR_ACTION_TYPE_BOOLEAN_INPUT },
         { "/user/hand/left/input/trigger/click", XR_ACTION_TYPE_BOOLEAN_INPUT }, { "/user/hand/right/input/trigger/click", XR_ACTION_TYPE_BOOLEAN_INPUT },

         // Controller Specific Buttons
         { "/user/hand/left/input/x/click", XR_ACTION_TYPE_BOOLEAN_INPUT }, { "/user/hand/left/input/y/click", XR_ACTION_TYPE_BOOLEAN_INPUT },
         { "/user/hand/right/input/a/click", XR_ACTION_TYPE_BOOLEAN_INPUT }, { "/user/hand/right/input/b/click", XR_ACTION_TYPE_BOOLEAN_INPUT },
         { "/user/hand/left/input/thumbstick/click", XR_ACTION_TYPE_BOOLEAN_INPUT }, { "/user/hand/right/input/thumbstick/click", XR_ACTION_TYPE_BOOLEAN_INPUT },
         { "/user/hand/left/input/trackpad/click", XR_ACTION_TYPE_BOOLEAN_INPUT }, { "/user/hand/right/input/trackpad/click", XR_ACTION_TYPE_BOOLEAN_INPUT },

         // Analog Inputs (Float)
         { "/user/hand/left/input/trigger/value", XR_ACTION_TYPE_FLOAT_INPUT }, { "/user/hand/right/input/trigger/value", XR_ACTION_TYPE_FLOAT_INPUT },
         { "/user/hand/left/input/squeeze/value", XR_ACTION_TYPE_FLOAT_INPUT }, { "/user/hand/right/input/squeeze/value", XR_ACTION_TYPE_FLOAT_INPUT },
         { "/user/hand/left/input/thumbstick/x", XR_ACTION_TYPE_FLOAT_INPUT }, { "/user/hand/left/input/thumbstick/y", XR_ACTION_TYPE_FLOAT_INPUT },
         { "/user/hand/right/input/thumbstick/x", XR_ACTION_TYPE_FLOAT_INPUT }, { "/user/hand/right/input/thumbstick/y", XR_ACTION_TYPE_FLOAT_INPUT },
         { "/user/hand/left/input/trackpad/x", XR_ACTION_TYPE_FLOAT_INPUT }, { "/user/hand/left/input/trackpad/y", XR_ACTION_TYPE_FLOAT_INPUT },
         { "/user/hand/right/input/trackpad/x", XR_ACTION_TYPE_FLOAT_INPUT }, { "/user/hand/right/input/trackpad/y", XR_ACTION_TYPE_FLOAT_INPUT },

         // Poses
         { "/user/hand/left/input/grip/pose", XR_ACTION_TYPE_POSE_INPUT }, { "/user/hand/right/input/grip/pose", XR_ACTION_TYPE_POSE_INPUT }
      };

      // Bind them
      uint16_t actionId = 0;
      std::vector<XrActionSuggestedBinding> bindings;
      for (auto& [path, type] : allPaths)
      {
         ActionTracker tracker;
         tracker.path = path;
         tracker.type = type;
         tracker.pinInputId = actionId;
         actionId++;

         XrActionCreateInfo actionInfo { XR_TYPE_ACTION_CREATE_INFO };
         actionInfo.actionType = type;

         // Sanitize name for OpenXR (no slashes)
         std::string name = path;
         std::replace(name.begin(), name.end(), '/', '_');
         name.erase(0, 1); // remove leading underscore

         CopyToXrBuffer(actionInfo.actionName, name);
         CopyToXrBuffer(actionInfo.localizedActionName, path);

         xrCreateAction(m_actionSet, &actionInfo, &tracker.action);

         XrPath xrPath;
         xrStringToPath(m_instance, path.c_str(), &xrPath);
         bindings.push_back({ tracker.action, xrPath });
         m_trackers.push_back(tracker);

         if (type != XR_ACTION_TYPE_POSE_INPUT)
            m_pininput.RegisterElementName(m_joyId, type == XR_ACTION_TYPE_FLOAT_INPUT, tracker.pinInputId, name);
      }

      // Register these bindings for major profiles
      std::array<const char*, 6> profiles { //
         "/interaction_profiles/khr/simple_controller", //
         "/interaction_profiles/oculus/touch_controller", //
         "/interaction_profiles/valve/index_controller", //
         "/interaction_profiles/htc/vive_controller", //
         "/interaction_profiles/microsoft/motion_controller", //
         "/interaction_profiles/google/daydream_controller" };
      for (const char* p : profiles)
      {
         SuggestBindings(p, bindings);
      }

      XrSessionActionSetsAttachInfo attachInfo { XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO };
      attachInfo.countActionSets = 1;
      attachInfo.actionSets = &m_actionSet;
      xrAttachSessionActionSets(m_session, &attachInfo);

      auto defaultMapping = [this]( //
               const std::function<bool(const vector<ButtonMapping>&, unsigned int)>& mapButton, //
               const std::function<bool(const SensorMapping&, SensorMapping::Type type, bool isLinear)>& mapPlunger, //
               const std::function<bool(const SensorMapping&, const SensorMapping&)>& mapNudge)
      {
         bool success = true;
         success &= mapButton(ButtonMapping::Create(m_joyId, 16, 0.3f), m_pininput.GetLeftFlipperActionId()); // Trigger
         success &= mapButton(ButtonMapping::Create(m_joyId, 17, 0.3f), m_pininput.GetRightFlipperActionId());
         success &= mapButton(ButtonMapping::Create(m_joyId, 16, 0.6f), m_pininput.GetStagedLeftFlipperActionId());
         success &= mapButton(ButtonMapping::Create(m_joyId, 17, 0.6f), m_pininput.GetStagedRightFlipperActionId());
         success &= mapButton(ButtonMapping::Create(m_joyId, 12), m_pininput.GetVRControllerViewCenteringActionId()); // Left Thumbstick click
         success &= mapButton(ButtonMapping::Create(m_joyId, 13), m_pininput.GetLaunchBallActionId()); // Right Thumbstick click
         success &= mapButton(ButtonMapping::Create(m_joyId, 4), m_pininput.GetLeftMagnaActionId()); // Squeeze
         success &= mapButton(ButtonMapping::Create(m_joyId, 5), m_pininput.GetRightMagnaActionId());
         success &= mapButton(ButtonMapping::Create(m_joyId, 11), m_pininput.GetAddCreditActionId(0)); // Right buttons
         success &= mapButton(ButtonMapping::Create(m_joyId, 10), m_pininput.GetStartActionId());
         success &= mapButton(ButtonMapping::Create(m_joyId, 8), m_pininput.GetOpenInGameUIActionId()); // Left buttons
         success &= mapButton(ButtonMapping::Create(m_joyId, 9), m_pininput.GetExitGameActionId());
         auto plungerMapping = SensorMapping::Create(m_joyId, 23, SensorMapping::Type::Position); // Right stick
         plungerMapping.SetScale(-1.f);
         success &= mapPlunger(plungerMapping, SensorMapping::Type::Position, true);
         success &= mapNudge(SensorMapping::Create(m_joyId, 20, SensorMapping::Type::Position), SensorMapping::Create(m_joyId, 21, SensorMapping::Type::Position)); // Left stick
         return success;
      };
      m_pininput.RegisterDefaultMapping(m_joyId, defaultMapping);
   }

   ~XRInputHandler() override
   {
      for (auto& t : m_trackers)
         xrDestroyAction(t.action);
      xrDestroyActionSet(m_actionSet);
   }

   void Update() override
   {
      XrActiveActionSet activeSet { m_actionSet, XR_NULL_PATH };
      XrActionsSyncInfo syncInfo { XR_TYPE_ACTIONS_SYNC_INFO };
      syncInfo.countActiveActionSets = 1;
      syncInfo.activeActionSets = &activeSet;
      xrSyncActions(m_session, &syncInfo);

      for (auto& t : m_trackers)
      {
         XrActionStateGetInfo getInfo { XR_TYPE_ACTION_STATE_GET_INFO, nullptr, t.action };

         if (t.type == XR_ACTION_TYPE_BOOLEAN_INPUT)
         {
            XrActionStateBoolean state { XR_TYPE_ACTION_STATE_BOOLEAN };
            xrGetActionStateBoolean(m_session, &getInfo, &state);

            if (state.isActive && state.changedSinceLastSync)
            {
               //PLOGI << "[BTN] " << t.path << " -> " << (state.currentState ? "ON" : "OFF");
               m_pininput.PushButtonEvent(m_joyId, t.pinInputId, usec() * 1000ULL, state.currentState);
            }
         }
         else if (t.type == XR_ACTION_TYPE_FLOAT_INPUT)
         {
            XrActionStateFloat state { XR_TYPE_ACTION_STATE_FLOAT };
            xrGetActionStateFloat(m_session, &getInfo, &state);

            if (state.isActive && state.changedSinceLastSync)
            {
               // -1 .. 1 for sticks / 0 (unpressed) .. 1 (pressed) for triggers
               //PLOGI << "[ANA] " << t.path << " -> " << state.currentState;
               m_pininput.PushAxisEvent(m_joyId, t.pinInputId, usec() * 1000ULL, state.currentState);
            }
         }
      }
   }

private:
   void SuggestBindings(const char* profile, std::vector<XrActionSuggestedBinding>& bindings)
   {
      XrPath profilePath;
      if (xrStringToPath(m_instance, profile, &profilePath) != XR_SUCCESS)
         return;

      vector<XrActionSuggestedBinding> acceptedBindings;
      for (XrActionSuggestedBinding& binding : bindings)
      {
         XrInteractionProfileSuggestedBinding suggested { XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };
         suggested.interactionProfile = profilePath;
         suggested.suggestedBindings = &binding;
         suggested.countSuggestedBindings = 1;
         if (xrSuggestInteractionProfileBindings(m_instance, &suggested) == XR_SUCCESS)
            acceptedBindings.push_back(binding);
      }

      XrInteractionProfileSuggestedBinding suggested { XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };
      suggested.interactionProfile = profilePath;
      suggested.suggestedBindings = acceptedBindings.data();
      suggested.countSuggestedBindings = (uint32_t)acceptedBindings.size();
      if (xrSuggestInteractionProfileBindings(m_instance, &suggested) != XR_SUCCESS)
      {
         PLOGE << "Failed to setup OpenXR controller bindings for profile " << profile;
      }
      else
      {
         PLOGI << "Successfully defined " << acceptedBindings.size() << " OpenXR controller bindings for profile " << profile;
      }
   }

public:
   XrAction GetAction(const std::string& path) const
   {
      for (const auto& t : m_trackers)
         if (t.path == path)
            return t.action;
      return XR_NULL_HANDLE;
   }

private:

   InputManager& m_pininput;
   XrInstance m_instance;
   XrSession m_session;
   XrActionSet m_actionSet;
   std::vector<ActionTracker> m_trackers;

   uint16_t m_joyId;
};
