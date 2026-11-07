// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "renderer/Window.h"

#include "doctest.h"

TEST_CASE("VR output window")
{
   // The playfield window used in VR is a headless output (no SDL window): it must always report
   // being focused and visible so that the player does not pause when the desktop display loses
   // focus (e.g. another person using the computer while playing in VR)
   VPX::Window wnd(1920, 1080);
   CHECK(wnd.GetCore() == nullptr);
   CHECK(wnd.IsFocused());
   CHECK(wnd.IsVisible());
   CHECK(!wnd.IsMinimized());
}
