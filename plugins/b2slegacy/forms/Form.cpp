#include "../common.h"

#include "Form.h"
#include "../DMDOverlay/DMDOverlay.h"
#include "../Server.h"

namespace B2SLegacy {

Form::Form(VPXPluginAPI* vpxApi, const MsgPluginAPI* msgApi, uint32_t endpointId, B2SData* pB2SData, const string& overlayType) 
   : Control(vpxApi), 
     m_msgApi(msgApi), 
     m_endpointId(endpointId),
     m_pB2SData(pB2SData)
{
   if (!overlayType.empty()) {
      m_pResURIResolver = new PinballPlugin::ResURIResolver(*msgApi, m_endpointId, true, false, false);
      m_dmdDisplayTarget = overlayType == "ScoreView" ? 1 : 0;
   }
}

Form::~Form()
{
   delete m_pResURIResolver;
}

void Form::Show()
{
}

void Form::Hide()
{
}

void Form::OnPaint(VPXRenderContext2D* const ctx)
{
   Control::OnPaint(ctx);
   // Renders the <DMD> displays of the document targeted at this view, growing the overlay
   // pool on demand (a display presence in the document means enabled; HideDMD overrides all)
   if (m_pResURIResolver != nullptr)
   {
      B2SOverlayDoc* const doc = &m_pB2SData->GetServer()->GetOverlayDoc();
      const VPXTexture docImage = GetOverlayDocImage();
      const SDL_Rect docCut = GetOverlayDocCut();

      // Lazily record the artwork bounds of this view, used to limit the overlay rect editing
      // ranges. The document space is the native artwork, not the grill-cut/resized background
      int& boundW = doc->viewW[m_dmdDisplayTarget];
      int& boundH = doc->viewH[m_dmdDisplayTarget];
      if (boundW == 0 && docImage != nullptr)
         if (const VPXTextureInfo* const texInfo = m_vpxApi->GetTextureInfo(docImage))
         {
            boundW = texInfo->width;
            boundH = texInfo->height;
         }
      const int docW = boundW > 0 ? boundW : static_cast<int>(ctx->srcWidth);
      const int docH = boundH > 0 ? boundH : static_cast<int>(ctx->srcHeight);

      // Map the display bounds from the B2S document space (native artwork, y downward) to the
      // render space (view size, y upward). The rendered artwork is the document image
      // anisotropically scaled, minus a cut band: doc rows [docCutY, docCutY + docCutH) are not
      // displayed, doc rows below them shift up (grill clipped away when hidden, small grill
      // band removed when shown)
      const int docVisH = std::max(1, docH - docCut.h);
      const float scaleX = ctx->srcWidth / static_cast<float>(docW);
      const float scaleY = ctx->srcHeight / static_cast<float>(docVisH);
      const int docCutY = docCut.y;
      const int docCutH = SDL_RectEmpty(&docCut) ? 0 : docCut.h;
      doc->mapQuad = [docCutY, docCutH, scaleX, scaleY](VPXRenderContext2D* ctx, const B2SDMDDisplay& display) -> DMDOverlay::vec4<int>
      {
         const auto mapY = [docCutY, docCutH](int y) { return y <= docCutY ? y : std::max(y - docCutH, docCutY); };
         const int visY0 = mapY(display.y);
         const int visY1 = mapY(display.y + display.h);
         return { static_cast<int>(display.x * scaleX), static_cast<int>(ctx->srcHeight - visY1 * scaleY), static_cast<int>(display.w * scaleX), static_cast<int>((visY1 - visY0) * scaleY) };
      };

      size_t n = 0;
      for (B2SDMDDisplay& display : doc->displays)
      {
         if (display.target != m_dmdDisplayTarget)
            continue;
         if (n >= m_dmdOverlays.size())
            m_dmdOverlays.emplace_back(std::make_unique<DMDOverlay::Renderer>(m_vpxApi, *m_pResURIResolver));
         m_dmdOverlays[n]->Render(ctx, display, docImage);
         n++;
      }
   }
}

}
