// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "input/ButtonMapping.h"

#include "doctest.h"

namespace
{

class TestButtonEventManager final : public ButtonMapping::ButtonInputEventManager
{
public:
   void Register(ButtonMapping* mapping) override { m_mappings.push_back(mapping); }
   void Unregister(ButtonMapping* mapping) override { std::erase(m_mappings, mapping); }
   vector<ButtonMapping*> m_mappings;
};

class TestButtonMappingHandler final : public ButtonMapping::ButtonMappingHandler
{
public:
   void OnInputChanged(ButtonMapping* mapping) override
   {
      ++m_callCount;
      m_lastMapping = mapping;
   }
   int m_callCount = 0;
   ButtonMapping* m_lastMapping = nullptr;
};

} // namespace

TEST_CASE("Button mapping")
{
   SUBCASE("pressed state changes trigger the handler once")
   {
      TestButtonMappingHandler handler;
      ButtonMapping mapping(nullptr, &handler, 1, 2);

      CHECK(!mapping.IsPressed());
      mapping.SetPressed(true);
      CHECK(mapping.IsPressed());
      CHECK(handler.m_callCount == 1);
      CHECK(handler.m_lastMapping == &mapping);

      mapping.SetPressed(true); // no change: no notification
      CHECK(handler.m_callCount == 1);

      mapping.SetPressed(false);
      CHECK(!mapping.IsPressed());
      CHECK(handler.m_callCount == 2);
   }

   SUBCASE("axis position applies the threshold")
   {
      TestButtonMappingHandler handler;
      ButtonMapping mapping(nullptr, &handler, 1, 2, 0.5f);

      mapping.SetAxisPosition(0.4f);
      CHECK(!mapping.IsPressed());
      mapping.SetAxisPosition(0.6f);
      CHECK(mapping.IsPressed());
      mapping.SetAxisPosition(0.5f);
      CHECK(mapping.IsPressed()); // threshold is inclusive
      mapping.SetAxisPosition(0.49f);
      CHECK(!mapping.IsPressed());
      CHECK(handler.m_callCount == 2);
   }

   SUBCASE("reversed axis presses below the threshold")
   {
      ButtonMapping mapping(nullptr, nullptr, 1, 2, 0.5f, true);

      mapping.SetAxisPosition(0.6f);
      CHECK(!mapping.IsPressed());
      mapping.SetAxisPosition(0.4f);
      CHECK(mapping.IsPressed());
   }

   SUBCASE("mappings register and unregister from the event manager")
   {
      TestButtonEventManager manager;
      {
         ButtonMapping mapping(&manager, nullptr, 1, 2);
         CHECK(manager.m_mappings.size() == 1);
         CHECK(manager.m_mappings[0] == &mapping);

         ButtonMapping moved(std::move(mapping));
         CHECK(manager.m_mappings.size() == 2); // moved-from mapping still registered until destruction
         CHECK(std::ranges::find(manager.m_mappings, &moved) != manager.m_mappings.end());
      }
      CHECK(manager.m_mappings.empty());
   }

   SUBCASE("move assignment transfers the mapping data")
   {
      TestButtonEventManager manager;
      ButtonMapping a(&manager, nullptr, 1, 2, 0.5f, true);
      ButtonMapping b(&manager, nullptr, 3, 4);
      b = std::move(a);
      CHECK(b.GetDeviceId() == 1);
      CHECK(b.GetAxisOrButtonId() == 2);
      CHECK(b.GetAxisThreshold() == doctest::Approx(0.5f));
      CHECK(b.IsAxisReversed());
   }

   SUBCASE("factory and equality")
   {
      const auto mappings = ButtonMapping::Create(3, 7, 0.25f, true);
      REQUIRE(mappings.size() == 1);
      CHECK(mappings[0].GetDeviceId() == 3);
      CHECK(mappings[0].GetAxisOrButtonId() == 7);
      CHECK(mappings[0].GetAxisThreshold() == doctest::Approx(0.25f));
      CHECK(mappings[0].IsAxisReversed());

      ButtonMapping a(nullptr, nullptr, 1, 2, 0.5f, false);
      ButtonMapping same(nullptr, nullptr, 1, 2, 0.5f, false);
      ButtonMapping different(nullptr, nullptr, 1, 3, 0.5f, false);
      CHECK(a == same);
      CHECK(!(a == different));
   }

   SUBCASE("With creates a bound copy")
   {
      TestButtonMappingHandler handler;
      auto sourceMappings = ButtonMapping::Create(1, 2);
      ButtonMapping bound = sourceMappings[0].With(nullptr, &handler);
      bound.SetPressed(true);
      CHECK(handler.m_callCount == 1);
      CHECK(handler.m_lastMapping == &bound);
   }
}
