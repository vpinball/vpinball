// license:GPLv3+

#pragma once

#include <mutex>
#include <string>
using std::string;
#include "robin_hood.h"
#include "typedefs3D.h"

class RenderDevice;
class BaseTexture;

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

enum SurfaceType
{
   RT_DEFAULT, // Default single layer surface
   RT_STEREO, // Texture array with 2 layers
   RT_CUBEMAP // Cubemap texture (6 layers)
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
   Sampler(RenderDevice* rd, const BaseTexture* const surf, const bool force_linear_rgb, const SamplerAddressMode clampu = SA_UNDEFINED, const SamplerAddressMode clampv = SA_UNDEFINED, const SamplerFilter filter = SF_UNDEFINED);
#if defined(ENABLE_BGFX)
   Sampler(RenderDevice* rd, SurfaceType type, bgfx::TextureHandle bgfxTexture, unsigned int width, unsigned int height, bool ownTexture, bool linear_rgb, const SamplerAddressMode clampu = SA_UNDEFINED, const SamplerAddressMode clampv = SA_UNDEFINED, const SamplerFilter filter = SF_UNDEFINED);
   bgfx::TextureHandle GetCoreTexture();
   bool IsMipMapGenerated() const { return (m_textureUpdate == nullptr) && !bgfx::isValid(m_nomipsTexture); }
   uintptr_t GetNativeTexture();
#elif defined(ENABLE_OPENGL)
   Sampler(RenderDevice* rd, SurfaceType type, GLuint glTexture, bool ownTexture, bool force_linear_rgb, const SamplerAddressMode clampu = SA_UNDEFINED, const SamplerAddressMode clampv = SA_UNDEFINED, const SamplerFilter filter = SF_UNDEFINED);
   GLuint GetCoreTexture() const { return m_texture; }
   GLenum GetCoreTarget() const { return m_texTarget; }
#elif defined(ENABLE_DX9)
   Sampler(RenderDevice* rd, IDirect3DTexture9* dx9Texture, bool ownTexture, bool force_linear_rgb, const SamplerAddressMode clampu = SA_UNDEFINED, const SamplerAddressMode clampv = SA_UNDEFINED, const SamplerFilter filter = SF_UNDEFINED);
   IDirect3DTexture9* GetCoreTexture() { return m_texture; }
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
   const SurfaceType m_type;

private:
   bool m_ownTexture;
   bool m_isLinear;
   RenderDevice* const m_rd;
   unsigned int m_width;
   unsigned int m_height;
   SamplerAddressMode m_clampu;
   SamplerAddressMode m_clampv;
   SamplerFilter m_filter;

#if defined(ENABLE_BGFX)
   string m_name;
   bgfx::TextureFormat::Enum m_bgfx_format = bgfx::TextureFormat::Enum::Count;
   bgfx::TextureHandle m_nomipsTexture = BGFX_INVALID_HANDLE; // The texture without any mipmaps
   bgfx::TextureHandle m_mipsTexture = BGFX_INVALID_HANDLE;
   std::mutex m_textureUpdateMutex;
   const bgfx::Memory* m_textureUpdate = nullptr;
   uintptr_t m_texture_override = 0;
#elif defined(ENABLE_OPENGL)
   GLenum m_texTarget = 0;
   GLuint m_texture = 0;
   GLuint CreateTexture(const BaseTexture* const surf, unsigned int Levels, colorFormat Format, int stereo);
#elif defined(ENABLE_DX9)
   IDirect3DTexture9* m_texture = nullptr;
   IDirect3DTexture9* CreateSystemTexture(const BaseTexture* const surf, const bool force_linear_rgb, colorFormat& texformat);
#endif
};
