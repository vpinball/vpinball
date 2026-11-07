// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "input/InputAction.h"

#include "doctest.h"

namespace
{

// Stands in for InputManager: registers the button mappings of all actions and answers the
// device/settings queries InputAction needs to build bindings from mapping strings.
class FakeActionEventManager final : public InputAction::EventManager
{
public:
   void Register(ButtonMapping* mapping) override { m_mappings.push_back(mapping); }
   void Unregister(ButtonMapping* mapping) override { std::erase(m_mappings, mapping); }

   uint16_t GetDeviceId(const string& settingsId) override
   {
      if (const auto it = m_deviceIds.find(settingsId); it != m_deviceIds.end())
         return it->second;
      const uint16_t id = static_cast<uint16_t>(m_deviceIds.size() + 1);
      m_deviceIds[settingsId] = id;
      m_deviceNames[id] = settingsId;
      return id;
   }
   const string& GetDeviceSettingId(uint16_t deviceId) const override { return m_deviceNames.at(deviceId); }
   string GetMappingLabel(const vector<ButtonMapping>&) const override { return ""s; }
   bool OnInputActionStateChanged(InputAction*) override { return true; }
   void RegisterOnUpdate(InputAction*) override { }
   void UnregisterOnUpdate(InputAction*) override { }
   bool IsUINavigationActionId(unsigned int) const override { return false; }

   vector<ButtonMapping*> m_mappings;
   ankerl::unordered_dense::map<string, uint16_t> m_deviceIds;
   ankerl::unordered_dense::map<uint16_t, string> m_deviceNames;
};

// Mirrors InputManager::PushButtonEvent: longest completed chord wins, then the state is applied.
void PushButtonEvent(FakeActionEventManager& manager, const vector<std::unique_ptr<InputAction>>& actions, uint16_t deviceId, uint16_t buttonId, bool isPressed)
{
   const uint32_t id = deviceId << 16 | buttonId;
   if (isPressed)
      InputAction::ResolveChordPriority(actions, id, [](const ButtonMapping&) { return true; });
   for (ButtonMapping* mapping : manager.m_mappings)
      if ((mapping->GetDeviceId() << 16 | mapping->GetAxisOrButtonId()) == id)
         mapping->SetPressed(isPressed);
}

// Mirrors the button-mapping branch of InputManager::PushAxisEvent.
void PushAxisEvent(FakeActionEventManager& manager, const vector<std::unique_ptr<InputAction>>& actions, uint16_t deviceId, uint16_t axisId, float position)
{
   const uint32_t id = deviceId << 16 | axisId;
   InputAction::ResolveChordPriority(actions, id, [position](const ButtonMapping& m) { return m.ComputeAxisPressed(position); });
   for (ButtonMapping* mapping : manager.m_mappings)
      if ((mapping->GetDeviceId() << 16 | mapping->GetAxisOrButtonId()) == id)
         mapping->SetAxisPosition(position);
}

} // namespace

TEST_CASE("Input action chord priority")
{
   FakeActionEventManager manager;
   const uint16_t KEYBOARD = manager.GetDeviceId("Key"s); // resolved first, so always device 1
   vector<std::unique_ptr<InputAction>> actions;
   vector<string> events;
   auto addAction = [&manager, &actions, &events](const string& label, const string& mapping)
   {
      actions.push_back(std::make_unique<InputAction>(
         &manager, label, label, ""s, [&events, label](InputAction&, bool wasPressed, bool isPressed) { events.push_back(label + (isPressed ? ":down" : ":up")); }));
      actions.back()->SetMapping(mapping);
      return actions.back().get();
   };

   constexpr int A = 1;
   constexpr int B = 2;
   constexpr int C = 3;
   constexpr int D = 4;

   SUBCASE("a press completing a chord suppresses the shorter binding it also completes")
   {
      InputAction* chord = addAction("chord"s, "Key;1 & Key;2"s);
      InputAction* single = addAction("single"s, "Key;2"s);

      PushButtonEvent(manager, actions, KEYBOARD, A, true);
      CHECK(!chord->IsPressed());
      CHECK(!single->IsPressed());

      PushButtonEvent(manager, actions, KEYBOARD, B, true);
      CHECK(chord->IsPressed());
      CHECK(!single->IsPressed());
      CHECK(events == vector<string> { "chord:down"s });

      // The consumed press stays dead while its button is held, even once the chord breaks
      PushButtonEvent(manager, actions, KEYBOARD, A, false);
      CHECK(!chord->IsPressed());
      CHECK(!single->IsPressed());

      // Released then pressed again with A no longer held: the binding fires normally
      PushButtonEvent(manager, actions, KEYBOARD, B, false);
      PushButtonEvent(manager, actions, KEYBOARD, B, true);
      CHECK(!chord->IsPressed());
      CHECK(single->IsPressed());
      CHECK(events == vector<string> { "chord:down"s, "chord:up"s, "single:down"s });

      // And it competes (and loses) again on the next A+B gesture
      PushButtonEvent(manager, actions, KEYBOARD, B, false);
      PushButtonEvent(manager, actions, KEYBOARD, A, true);
      PushButtonEvent(manager, actions, KEYBOARD, B, true);
      CHECK(chord->IsPressed());
      CHECK(!single->IsPressed());
   }

   SUBCASE("no delay on the first button and no retraction of a running action")
   {
      InputAction* chord = addAction("chord"s, "Key;1 & Key;2"s);
      InputAction* single = addAction("single"s, "Key;2"s);

      PushButtonEvent(manager, actions, KEYBOARD, B, true);
      CHECK(!chord->IsPressed());
      CHECK(single->IsPressed());

      // Pressing the second button later fires the chord too, without cancelling the first action
      PushButtonEvent(manager, actions, KEYBOARD, A, true);
      CHECK(chord->IsPressed());
      CHECK(single->IsPressed());

      // The single action's fire ends when its own button is released, not when the chord breaks
      PushButtonEvent(manager, actions, KEYBOARD, A, false);
      CHECK(!chord->IsPressed());
      CHECK(single->IsPressed());
      PushButtonEvent(manager, actions, KEYBOARD, B, false);
      CHECK(!single->IsPressed());
   }

   SUBCASE("the longest completed binding wins at every press")
   {
      InputAction* pairAB = addAction("pairAB"s, "Key;1 & Key;2"s);
      InputAction* singleB = addAction("singleB"s, "Key;2"s);
      InputAction* trioABC = addAction("trioABC"s, "Key;1 & Key;2 & Key;3"s);

      PushButtonEvent(manager, actions, KEYBOARD, A, true);
      PushButtonEvent(manager, actions, KEYBOARD, B, true);
      CHECK(pairAB->IsPressed());
      CHECK(!singleB->IsPressed());

      PushButtonEvent(manager, actions, KEYBOARD, C, true);
      CHECK(trioABC->IsPressed());
      CHECK(pairAB->IsPressed()); // already firing actions are not retracted
      CHECK(!singleB->IsPressed());
   }

   SUBCASE("the longest binding wins even when the shorter one is not a subset")
   {
      InputAction* pairAB = addAction("pairAB"s, "Key;1 & Key;2"s);
      InputAction* trioBCD = addAction("trioBCD"s, "Key;2 & Key;3 & Key;4"s);

      PushButtonEvent(manager, actions, KEYBOARD, A, true);
      PushButtonEvent(manager, actions, KEYBOARD, C, true);
      PushButtonEvent(manager, actions, KEYBOARD, D, true);
      PushButtonEvent(manager, actions, KEYBOARD, B, true);
      CHECK(!pairAB->IsPressed());
      CHECK(trioBCD->IsPressed());
   }

   SUBCASE("chords of the same length completed together all fire")
   {
      InputAction* pairAB = addAction("pairAB"s, "Key;1 & Key;2"s);
      InputAction* pairAC = addAction("pairAC"s, "Key;1 & Key;3"s);

      PushButtonEvent(manager, actions, KEYBOARD, B, true);
      PushButtonEvent(manager, actions, KEYBOARD, C, true);
      PushButtonEvent(manager, actions, KEYBOARD, A, true);
      CHECK(pairAB->IsPressed());
      CHECK(pairAC->IsPressed());
   }

   SUBCASE("a chord already held does not veto later completions")
   {
      InputAction* pairAB = addAction("pairAB"s, "Key;1 & Key;2"s);
      InputAction* pairBC = addAction("pairBC"s, "Key;2 & Key;3"s);

      PushButtonEvent(manager, actions, KEYBOARD, A, true);
      PushButtonEvent(manager, actions, KEYBOARD, B, true);
      CHECK(pairAB->IsPressed());

      PushButtonEvent(manager, actions, KEYBOARD, C, true);
      CHECK(pairBC->IsPressed());
   }

   SUBCASE("chords mixing an axis-as-button member follow the same priority")
   {
      InputAction* chord = addAction("chord"s, "Joy;0;o;0.5 & Key;2"s);
      InputAction* single = addAction("single"s, "Key;2"s);
      const uint16_t joyId = manager.GetDeviceId("Joy"s);
      const uint16_t keyId = manager.GetDeviceId("Key"s);

      PushAxisEvent(manager, actions, joyId, 0, 0.6f); // past the 0.5 threshold
      PushButtonEvent(manager, actions, keyId, B, true);
      CHECK(chord->IsPressed());
      CHECK(!single->IsPressed());

      // Below the threshold the chord is not complete and the single button fires
      PushButtonEvent(manager, actions, keyId, B, false);
      PushAxisEvent(manager, actions, joyId, 0, 0.4f);
      PushButtonEvent(manager, actions, keyId, B, true);
      CHECK(!chord->IsPressed());
      CHECK(single->IsPressed());
   }

   SUBCASE("repeat press events do not re-trigger or clear the veto")
   {
      InputAction* chord = addAction("chord"s, "Key;1 & Key;2"s);
      InputAction* single = addAction("single"s, "Key;2"s);

      PushButtonEvent(manager, actions, KEYBOARD, A, true);
      PushButtonEvent(manager, actions, KEYBOARD, B, true);
      PushButtonEvent(manager, actions, KEYBOARD, B, true);
      PushButtonEvent(manager, actions, KEYBOARD, B, true);
      CHECK(chord->IsPressed());
      CHECK(!single->IsPressed());
      CHECK(events == vector<string> { "chord:down"s });
   }
}
