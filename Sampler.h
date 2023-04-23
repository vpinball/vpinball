#pragma once

#include "inc/robin_hood.h"
#include "typedefs3D.h"

class RenderDevice;

enum SamplerFilter : unsigned int
{
   SF_NONE, // No filtering at all. DX: MIPFILTER = NONE; MAGFILTER = POINT; MINFILTER = POINT; / OpenGL Nearest/Nearest
   SF_POINT, // Point sampled (aka nearest mipmap) texture filtering.
   SF_BILINEAR, // Bilinar texture filtering (linear min/mag, no mipmapping). DX: MIPFILTER = NONE; MAGFILTER = LINEAR; MINFILTER = LINEAR;
   SF_TRILINEAR, // Trilinar texture filtering (linear min/mag, with mipmapping). DX: MIPFILTER = LINEAR; MAGFILTER = LINEAR; MINFILTER = LINEAR;
   SF_ANISOTROPIC, // Anisotropic texture filtering.
   SF_UNDEFINED, // Used for undefined default values
};

enum SamplerAddressMode : unsigned int
{
   SA_REPEAT,
   SA_CLAMP,
   SA_MIRROR,
   SA_UNDEFINED, // Used for undefined default values
};

class Sampler;
struct SamplerBinding
{
   int unit;
   int use_rank;
   Sampler* sampler;
   SamplerFilter filter;
   SamplerAddressMode clamp_u;
   SamplerAddressMode clamp_v;
};

class Sampler
{
public:
   Sampler(RenderDevice* rd, BaseTexture* const surf, const bool force_linear_rgb, const SamplerAddressMode clampu = SA_UNDEFINED, const SamplerAddressMode clampv = SA_UNDEFINED, const SamplerFilter filter = SF_UNDEFINED);
#ifdef ENABLE_SDL
   Sampler(RenderDevice* rd, GLuint glTexture, bool ownTexture, bool force_linear_rgb, const SamplerAddressMode clampu = SA_UNDEFINED, const SamplerAddressMode clampv = SA_UNDEFINED, const SamplerFilter filter = SF_UNDEFINED);
   GLuint GetCoreTexture() const { return m_texture; }
#else
   Sampler(RenderDevice* rd, IDirect3DTexture9* dx9Texture, bool ownTexture, bool force_linear_rgb, const SamplerAddressMode clampu = SA_UNDEFINED, const SamplerAddressMode clampv = SA_UNDEFINED, const SamplerFilter filter = SF_UNDEFINED);
   IDirect3DTexture9* GetCoreTexture() { return m_texture;  }
#endif
   ~Sampler();

   void Unbind();
   void UpdateTexture(BaseTexture* const surf, const bool force_linear_rgb);
   void SetClamp(const SamplerAddressMode clampu, const SamplerAddressMode clampv);
   void SetFilter(const SamplerFilter filter);
   void SetName(const string& name);

   bool IsLinear() const { return m_isLinear; }
   int GetWidth() const { return m_width; }
   int GetHeight() const { return m_height; }
   SamplerFilter GetFilter() const { return m_filter; }
   SamplerAddressMode GetClampU() const { return m_clampu; }
   SamplerAddressMode GetClampV() const { return m_clampv; }

   bool m_dirty;
   robin_hood::unordered_set<SamplerBinding*> m_bindings;

private:
   bool m_ownTexture;
   bool m_isLinear;
   RenderDevice* m_rd;
   int m_width;
   int m_height;
   SamplerAddressMode m_clampu;
   SamplerAddressMode m_clampv;
   SamplerFilter m_filter;

#ifdef ENABLE_SDL
   GLuint m_texture = 0;
   GLuint CreateTexture(unsigned int Width, unsigned int Height, unsigned int Levels, colorFormat Format, void* data, int stereo);
#else
   IDirect3DTexture9* m_texture;
   IDirect3DTexture9* CreateSystemTexture(BaseTexture* const surf, const bool force_linear_rgb, colorFormat& texformat);
#endif
};
