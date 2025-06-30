// license:GPLv3+

#pragma once

#include <mutex>
#include <string>
using std::string;
#include "unordered_dense.h"
#include "typedefs3D.h"

class RenderDevice;
class BaseTexture;

enum SamplerFilter : unsigned int
{
   SF_NONE, // No filtering at all. DX: MIPFILTER = NONE; MAGFILTER = POINT; MINFILTER = POINT; / OpenGL Nearest/Nearest
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

class Sampler final
{
public:
   Sampler(RenderDevice* rd, string name, std::shared_ptr<const BaseTexture> surf, const bool force_linear_rgb);
   ~Sampler();

#if defined(ENABLE_BGFX)
   Sampler(RenderDevice* rd, string name, SurfaceType type, bgfx::TextureHandle bgfxTexture, unsigned int width, unsigned int height, bool ownTexture);
   bgfx::TextureHandle GetCoreTexture(bool genMipmaps);
   bool IsMipMapGenerated() const { return (m_textureUpdate == nullptr) && !bgfx::isValid(m_nomipsTexture); }
   uintptr_t GetNativeTexture();

#elif defined(ENABLE_OPENGL)
   Sampler(RenderDevice* rd, string name, SurfaceType type, GLuint glTexture, bool ownTexture);
   GLuint GetCoreTexture() const { return m_texture; }
   GLenum GetCoreTarget() const { return m_texTarget; }

#elif defined(ENABLE_DX9)
   Sampler(RenderDevice* rd, string name, IDirect3DTexture9* dx9Texture, bool ownTexture);
   IDirect3DTexture9* GetCoreTexture() const { return m_texture; }

#endif

   void Unbind();
   void UpdateTexture(std::shared_ptr<const BaseTexture> surf, const bool force_linear_rgb);

   int GetWidth() const { return m_width; }
   int GetHeight() const { return m_height; }

   const SurfaceType m_type;
   const string m_name;

private:
   const bool m_ownTexture;
   RenderDevice* const m_rd;
   unsigned int m_width;
   unsigned int m_height;

#if defined(ENABLE_BGFX)
   bgfx::TextureFormat::Enum m_bgfx_format = bgfx::TextureFormat::Enum::Count;
   bgfx::TextureHandle m_nomipsTexture = BGFX_INVALID_HANDLE; // The texture without any mipmaps
   bgfx::TextureHandle m_mipsTexture = BGFX_INVALID_HANDLE;
   std::mutex m_textureUpdateMutex;
   bool m_isTextureUpdateLinear;
   const bgfx::Memory* m_textureUpdate = nullptr;
   uintptr_t m_texture_override = 0;
#elif defined(ENABLE_OPENGL)
   GLenum m_texTarget = 0;
   GLuint m_texture = 0;
   GLuint CreateTexture(std::shared_ptr<const BaseTexture> surf, unsigned int Levels, colorFormat Format, int stereo);
public:
   struct SamplerBinding
   {
      int unit;
      int use_rank;
      std::shared_ptr<const Sampler> sampler;
      SamplerFilter filter;
      SamplerAddressMode clamp_u;
      SamplerAddressMode clamp_v;
   };
   mutable ankerl::unordered_dense::set<SamplerBinding*> m_bindings;
#elif defined(ENABLE_DX9)
   IDirect3DTexture9* m_texture = nullptr;
   IDirect3DTexture9* CreateSystemTexture(std::shared_ptr<const BaseTexture> surf, const bool force_linear_rgb, colorFormat& texformat);
#endif
};
