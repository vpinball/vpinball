#pragma once

#include "ui/win/IWinUIPart.h"
#include "ui/win/PinTableWnd.h"

class Surface;

class SurfaceWinUIPart final : public IWinUIPart
{
public:
   explicit SurfaceWinUIPart(PinTableWnd* editor, Surface* surface);

   void UIRenderPass1(Sur* psur) override;
   void UIRenderPass2(Sur* psur) override;
   void RenderBlueprint(Sur* psur, bool solid) override;

private:
   PinTableWnd* const m_editor;
   Surface* const m_surface;
};
