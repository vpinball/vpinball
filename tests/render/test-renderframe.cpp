// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "doctest.h"

#include "renderer/RenderFrame.h"
#include "renderer/RenderPass.h"
#include "renderer/RenderCommand.h"

TEST_CASE("RenderFrame deferred commands")
{
   SUBCASE("Discard drops pending begin of frame commands")
   {
      RenderFrame frame(nullptr);
      int calls = 0;
      frame.AddBeginOfFrameCmd([&calls]() { ++calls; });
      frame.Discard();
      CHECK(frame.Execute() == false);
      CHECK(calls == 0);
   }

   SUBCASE("Discard runs pending end of frame commands")
   {
      RenderFrame frame(nullptr);
      int calls = 0;
      frame.AddEndOfFrameCmd([&calls]() { ++calls; });
      frame.Discard();
      CHECK(calls == 1);
      CHECK(frame.Execute() == false);
      CHECK(calls == 1);
   }
}

TEST_CASE("RenderCommand recycling")
{
   SUBCASE("Clear resets the pass dependency")
   {
      RenderCommand cmd(nullptr);
      CHECK(cmd.m_dependency == nullptr);
      RenderPass pass("test"s, nullptr);
      cmd.m_dependency = &pass;
      cmd.Clear();
      CHECK(cmd.m_dependency == nullptr);
   }
}
