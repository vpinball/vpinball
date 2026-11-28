// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "ui/live/frontend/FrontendMedia.h"

#include "doctest.h"

#include <fstream>
#include <thread>


TEST_CASE("FrontendMedia failed video lifecycle")
{
   const std::filesystem::path file = GetTestTmpDir() / "invalid.mp4"s;
   std::filesystem::create_directories(file.parent_path());
   {
      std::ofstream stream(file, std::ios::binary);
      stream << "not a video";
   }

   {
      VPX::Frontend::FrontendMedia media(nullptr);
      CHECK(media.Get(file) == nullptr);
      std::this_thread::sleep_for(std::chrono::milliseconds(250));
      CHECK(media.Get(file) == nullptr);
      media.BeginFrame();
      CHECK(media.Get(file) == nullptr);
   }
}
