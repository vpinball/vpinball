#pragma once

#include "typedefs3D.h"
class RenderDevice;

enum SamplerFilter
{
   SF_NONE, // No mipmapping
   SF_POINT, // Point sampled (aka nearest mipmap) texture filtering.
   SF_BILINEAR, // Bilinar texture filtering.
   SF_TRILINEAR, // Trilinar texture filtering.
   SF_ANISOTROPIC // Anisotropic texture filtering.
};

enum SamplerAddressMode
{
#ifdef ENABLE_SDL
   SA_WRAP = GL_REPEAT,
   SA_CLAMP = GL_CLAMP_TO_EDGE,
   SA_MIRROR = GL_MIRRORED_REPEAT
#else
   SA_WRAP,
   SA_CLAMP,
   SA_MIRROR
#endif
};

class Sampler
{
public:
   Sampler(RenderDevice* rd, BaseTexture* const surf, const bool force_linear_rgb);
#ifdef ENABLE_SDL
   Sampler(RenderDevice* rd, GLuint glTexture, bool ownTexture, bool isMSAA, bool force_linear_rgb);
   GLuint GetCoreTexture() const { return m_texture; }
#else
   Sampler(RenderDevice* rd, IDirect3DTexture9* dx9Texture, bool ownTexture, bool force_linear_rgb);
   IDirect3DTexture9* GetCoreTexture() { return m_texture;  }
#endif
   ~Sampler();

   void UpdateTexture(BaseTexture* const surf, const bool force_linear_rgb);
   bool IsLinear() const { return m_isLinear; }
   bool IsMSAA() const { return m_isMSAA; }
   int GetWidth() const { return m_width; }
   int GetHeight() const { return m_height; }

public:
   bool m_dirty;

private:
   bool m_ownTexture;
   bool m_isLinear;
   bool m_isMSAA;
   RenderDevice* m_rd;
   int m_width;
   int m_height;
#ifdef ENABLE_SDL
   GLuint m_texture = 0;
   GLuint CreateTexture(UINT Width, UINT Height, UINT Levels, colorFormat Format, void* data, int stereo);
#else
   IDirect3DTexture9* m_texture;
   IDirect3DTexture9* CreateSystemTexture(BaseTexture* const surf, const bool force_linear_rgb, colorFormat& texformat);
#endif
};
