// license:GPLv3+

#pragma once

#include "InGameUIPage.h"

namespace VPX::InGameUI
{

class CabinetRender final
{
public:
   CabinetRender() = default;

   void Render(const ImVec4& rect, PinTable* table, const vec3& playerPos);
};

}
