#include "../common.h"

#include "Form.h"
#include "../utils/DMDOverlay.h"
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
   if (m_dmdTex && m_vpxApi)
      m_vpxApi->DeleteTexture(m_dmdTex);
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
      int& boundW = m_dmdDisplayTarget == 1 ? doc->scoreViewW : doc->backglassW;
      int& boundH = m_dmdDisplayTarget == 1 ? doc->scoreViewH : doc->backglassH;
      if (boundW == 0 && docImage != nullptr)
         if (const VPXTextureInfo* const texInfo = m_vpxApi->GetTextureInfo(docImage))
         {
            boundW = texInfo->width;
            boundH = texInfo->height;
         }
      const int docW = boundW > 0 ? boundW : static_cast<int>(ctx->srcWidth);
      const int docH = boundH > 0 ? boundH : static_cast<int>(ctx->srcHeight);

      size_t n = 0;
      for (B2SDMDDisplay& display : doc->displays)
      {
         if (display.target != m_dmdDisplayTarget)
            continue;
         if (n >= m_dmdOverlays.size())
            m_dmdOverlays.emplace_back(std::make_unique<DMDOverlay::DMDOverlay>(m_vpxApi, *m_pResURIResolver, m_dmdTex));
         m_dmdOverlays[n]->Render(ctx, &display, docImage, docW, docH, docCut.y, SDL_RectEmpty(&docCut) ? 0 : docCut.h);
         n++;
      }
   }
}

}
