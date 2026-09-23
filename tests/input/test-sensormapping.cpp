// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "input/SensorMapping.h"

#include "doctest.h"

namespace
{

class TestAxisEventManager final : public SensorMapping::AxisInputEventManager
{
public:
   void Register(SensorMapping* mapping) override { m_mappings.push_back(mapping); }
   void Unregister(SensorMapping* mapping) override { std::erase(m_mappings, mapping); }
   vector<SensorMapping*> m_mappings;
};

class TestSensorMappingHandler final : public SensorMapping::SensorMappingHandler
{
public:
   void OnInputChanged(SensorMapping* mapping) override
   {
      ++m_callCount;
      m_lastMapping = mapping;
   }
   int m_callCount = 0;
   SensorMapping* m_lastMapping = nullptr;
};

} // namespace

TEST_CASE("Sensor mapping")
{
   SUBCASE("raw value is remapped through dead zone, limit and scale")
   {
      SensorMapping mapping(nullptr, nullptr, 1, 2, SensorMapping::Type::Position, 0.1f, 2.f, 1.f);

      mapping.SetAxisPosition(1000000ull, 0.05f); // inside the dead zone
      CHECK(mapping.GetValue() == doctest::Approx(0.f));
      CHECK(mapping.GetRawValue() == doctest::Approx(0.05f));
      CHECK(mapping.GetRawValueTimestampNs() == 1000000ull);

      mapping.SetAxisPosition(2000000ull, 0.55f); // (0.55 - 0.1) / (1 - 0.1) = 0.5 -> * 2 = 1.0
      CHECK(mapping.GetValue() == doctest::Approx(1.f));

      mapping.SetAxisPosition(3000000ull, -0.55f);
      CHECK(mapping.GetValue() == doctest::Approx(-1.f));
   }

   SUBCASE("limit clamps before scaling")
   {
      SensorMapping mapping(nullptr, nullptr, 1, 2, SensorMapping::Type::Position, 0.f, 2.f, 0.5f);
      mapping.SetAxisPosition(0ull, 1.f);
      CHECK(mapping.GetValue() == doctest::Approx(1.f)); // clamped to 0.5 then scaled by 2
   }

   SUBCASE("handler is notified on every input")
   {
      TestSensorMappingHandler handler;
      SensorMapping mapping(nullptr, &handler, 1, 2, SensorMapping::Type::Position);
      mapping.SetAxisPosition(0ull, 0.f);
      mapping.SetAxisPosition(1000000ull, 0.f); // unchanged value: still notified
      CHECK(handler.m_callCount == 2);
      CHECK(handler.m_lastMapping == &mapping);
   }

   SUBCASE("shortest update interval is tracked")
   {
      SensorMapping mapping(nullptr, nullptr, 1, 2, SensorMapping::Type::Position);
      mapping.SetAxisPosition(0ull, 0.f);
      mapping.SetAxisPosition(5000000ull, 0.1f); // 5ms update
      mapping.SetAxisPosition(15000000ull, 0.2f); // 10ms update
      CHECK(mapping.GetShortestUpdateMs() == 5);
      mapping.ResetShortestUpdateMs();
      CHECK(mapping.GetShortestUpdateMs() == 0);
   }

   SUBCASE("dead zone, scale and limit can be changed at runtime")
   {
      SensorMapping mapping(nullptr, nullptr, 1, 2, SensorMapping::Type::Position);
      mapping.SetAxisPosition(0ull, 0.5f);
      CHECK(mapping.GetValue() == doctest::Approx(0.5f));
      mapping.SetDeadZone(0.6f);
      CHECK(mapping.GetValue() == doctest::Approx(0.f));
      mapping.SetScale(2.f);
      mapping.SetLimit(0.25f);
      mapping.SetAxisPosition(1000000ull, 1.f); // (1 - 0.6) / 0.4 = 1 -> clamped to 0.25 -> * 2 = 0.5
      CHECK(mapping.GetValue() == doctest::Approx(0.5f));
   }

   SUBCASE("mappings register and unregister from the event manager")
   {
      TestAxisEventManager manager;
      {
         SensorMapping mapping(&manager, nullptr, 1, 2, SensorMapping::Type::Velocity);
         CHECK(manager.m_mappings.size() == 1);
         CHECK(manager.m_mappings[0] == &mapping);

         SensorMapping copy = mapping.WithScale(3.f); // copy constructor registers the new instance
         CHECK(manager.m_mappings.size() == 2);
         CHECK(copy.GetScale() == doctest::Approx(3.f));
         CHECK(!copy.IsSame(mapping)); // IsSame compares the scale factor
         CHECK(!(copy == mapping));

         SensorMapping same = mapping.WithScale(mapping.GetScale());
         CHECK(same.IsSame(mapping));
      }
      CHECK(manager.m_mappings.empty());
   }

   SUBCASE("IsSame ignores event manager and handler")
   {
      TestAxisEventManager manager;
      TestSensorMappingHandler handler;
      SensorMapping a(nullptr, nullptr, 1, 2, SensorMapping::Type::Position);
      SensorMapping b(&manager, &handler, 1, 2, SensorMapping::Type::Position);
      CHECK(a.IsSame(b));
      CHECK(!(a == b));
   }

   SUBCASE("main frequency is computed asynchronously")
   {
      SensorMapping mapping(nullptr, nullptr, 1, 0, SensorMapping::Type::Position);
      CHECK(mapping.GetMainFrequency() == doctest::Approx(0.f)); // first call starts the capture

      // Feed a 16 Hz sine wave sampled at 1kHz (1ms steps) until the capture buffer is full
      const double freq = 16.0;
      for (int i = 0; i < 1100; ++i)
         mapping.SetAxisPosition(static_cast<uint64_t>(i) * 1000000ull, static_cast<float>(sin(2.0 * M_PI * freq * i / 1000.0)));

      float mainFreq = 0.f;
      for (int i = 0; i < 500 && mainFreq == 0.f; ++i)
      {
         std::this_thread::sleep_for(std::chrono::milliseconds(10));
         mainFreq = mapping.GetMainFrequency();
      }
      CHECK(mainFreq == doctest::Approx(freq).epsilon(0.2));
   }
}
