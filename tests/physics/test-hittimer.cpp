// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "physics/hittimer.h"
#include "physics/physconst.h"

#include "doctest.h"

namespace
{

class TestFireEvents final : public IFireEvents
{
public:
   void FireGroupEvent(const int dispid) override
   {
      ++m_fireCount;
      m_lastDispId = dispid;
   }
   IDispatch* GetIDispatch() override { return nullptr; }
   const IDispatch* GetIDispatch() const override { return nullptr; }

   int m_fireCount = 0;
   int m_lastDispId = 0;
};

} // namespace

TEST_CASE("Hit timer")
{
   SUBCASE("interval is clamped to the valid range")
   {
      TestFireEvents handler;
      HitTimer zeroInterval("timer0", 0, &handler);
      CHECK(zeroInterval.GetInterval() == 1); // clamped to the minimum interval

      HitTimer tooShort("timer-5", -5, &handler);
      CHECK(tooShort.GetInterval() == -2);

      HitTimer positive("timer100", 100, &handler);
      CHECK(positive.GetInterval() == 100);

      positive.SetInterval(7);
      CHECK(positive.GetInterval() == 7);
   }

   SUBCASE("timer does not fire before its interval")
   {
      TestFireEvents handler;
      HitTimer timer("timer", 100, &handler); // first fire is scheduled at time 100

      timer.Update(99);
      CHECK(handler.m_fireCount == 0);
   }

   SUBCASE("negative interval timers never fire from Update")
   {
      TestFireEvents handler;
      HitTimer timer("timer", -1, &handler);

      timer.Update(1000);
      timer.Update(2000);
      CHECK(handler.m_fireCount == 0);
   }

   SUBCASE("deferred timer does not fire")
   {
      TestFireEvents handler;
      HitTimer timer("timer", 100, &handler);

      timer.Defer();
      timer.Update(100000);
      CHECK(handler.m_fireCount == 0);
   }
}
