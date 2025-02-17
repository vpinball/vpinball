// license:GPLv3+

#pragma once

#include "renderer/typedefs3D.h"

class RenderDevice;

class BackGlass
{
public:
   BackGlass(RenderDevice* const device, Texture * backgroundFallback);
   ~BackGlass();
   void Render();
   void GetDMDPos(float& DMDposx, float& DMDposy, float& DMDwidth, float& DMDheight);

private:
   RenderDevice* m_pd3dDevice;
   BaseTexture* m_loaded_image; 
   Texture* m_backgroundFallback;
   Sampler* m_backgroundTexture;
   int2 m_backglass_dmd;
#if defined(ENABLE_VR) || defined(ENABLE_XR)
   unsigned int m_backglass_dmd_width;
   unsigned int m_backglass_dmd_height;
#endif
   float m_dmd_height;
   float m_dmd_width;
   Vertex2D m_dmd;
   unsigned int m_backglass_grill_height;
   unsigned int m_backglass_width;
   unsigned int m_backglass_height;

   static constexpr float m_backglass_scale = 1.2f;
};
