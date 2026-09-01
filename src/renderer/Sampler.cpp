// license:GPLv3+

#include "core/stdafx.h"
#include "Sampler.h"
#include "RenderDevice.h"

#if defined(ENABLE_BGFX)
#include <bx/allocator.h>
#include <bx/readerwriter.h>
#include <bx/endian.h>
#include <bx/math.h>
#include <bimg/decode.h>
#endif

Sampler::Sampler(RenderDevice* rd, string name, std::shared_ptr<const BaseTexture> surf, const bool force_linear_rgb)
   : m_type(SurfaceType::RT_DEFAULT)
   , m_name(std::move(name))
   , m_ownTexture(true)
   , m_rd(rd)
   , m_width(surf ? surf->width() : 0)
   , m_height(surf ? surf->height() : 0)
{
   assert(surf != nullptr);
   if (surf == nullptr)
   {
      PLOGE << "Texture '" << m_name << "' has no data and can not be uploaded";
      return;
   }

#if defined(ENABLE_BGFX)
   switch (surf->m_format)
   {
   case BaseTexture::BW: m_bgfx_format = bgfx::TextureFormat::Enum::R8; break;
   case BaseTexture::BW_FP32: m_bgfx_format = bgfx::TextureFormat::Enum::R32F; break;
   case BaseTexture::RGB: m_bgfx_format = bgfx::TextureFormat::Enum::RGB8; break;
   case BaseTexture::SRGB: m_bgfx_format = bgfx::TextureFormat::Enum::RGBA8; break;
   case BaseTexture::RGBA: m_bgfx_format = bgfx::TextureFormat::Enum::RGBA8; break;
   case BaseTexture::SRGBA: m_bgfx_format = bgfx::TextureFormat::Enum::RGBA8; break;
   case BaseTexture::SRGB565: m_bgfx_format = bgfx::TextureFormat::Enum::RGBA8; break;
   case BaseTexture::RGB_FP16: m_bgfx_format = bgfx::TextureFormat::Enum::RGBA16F; break;
   case BaseTexture::RGBA_FP16: m_bgfx_format = bgfx::TextureFormat::Enum::RGBA16F; break;
   case BaseTexture::RGB_FP32: m_bgfx_format = bgfx::TextureFormat::Enum::RGBA32F; break;
   case BaseTexture::RGBA_FP32: m_bgfx_format = bgfx::TextureFormat::Enum::RGBA32F; break;
   default: assert(false); // Unsupported texture format
   }
   UpdateTexture(surf, force_linear_rgb);

#elif defined(ENABLE_OPENGL)
   m_rd->m_curTextureUpdates++;
   m_texTarget = GL_TEXTURE_2D;
   // GL has no sRGB565 internal format: GL_RGB5 is decoded as linear, and OpenGL ES has no 5.6.5 internal format
   // at all. So widen to 8 bits per channel here and let GL_SRGB8_ALPHA8 decode, which is matching the BGFX backend.
   // Costs twice the texture memory, the same price BGFX pays, and it is the only way
   // to get the gamma right. Doing it before the dispatch below leaves a single conversion for both upload paths
   if (surf->m_format == BaseTexture::SRGB565)
      surf = surf->Convert(BaseTexture::SRGBA);
   colorFormat format;
   if (surf->m_format == BaseTexture::RGB)
      format = colorFormat::RGB;
   else if (surf->m_format == BaseTexture::RGBA)
      format = colorFormat::RGBA;
   else if (surf->m_format == BaseTexture::SRGB)
      format = force_linear_rgb ? colorFormat::RGB : colorFormat::SRGB;
   else if (surf->m_format == BaseTexture::SRGBA)
      format = force_linear_rgb ? colorFormat::RGBA : colorFormat::SRGBA;
   else if (surf->m_format == BaseTexture::RGB_FP16)
      format = colorFormat::RGB16F;
   else if (surf->m_format == BaseTexture::RGBA_FP16)
      format = colorFormat::RGBA16F;
   else if (surf->m_format == BaseTexture::RGB_FP32)
      format = colorFormat::RGB32F;
   else if (surf->m_format == BaseTexture::BW_FP32)
      format = colorFormat::RED32F;
   else if (surf->m_format == BaseTexture::BW)
      format = colorFormat::GREY8;
   else
      assert(false); // Unsupported image format
   m_texture = CreateTexture(surf, 0, format, 0);

#elif defined(ENABLE_DX9)
   m_rd->m_curTextureUpdates++;
   colorFormat texformat;
   IDirect3DTexture9* sysTex = CreateSystemTexture(surf, force_linear_rgb, texformat);

   HRESULT hr = m_rd->GetCoreDevice()->CreateTexture(m_width, m_height, (texformat != colorFormat::DXT5 && m_rd->m_autogen_mipmap) ? 0 : sysTex->GetLevelCount(),
      (texformat != colorFormat::DXT5 && m_rd->m_autogen_mipmap) ? textureUsage::AUTOMIPMAP : 0, (D3DFORMAT)texformat, (D3DPOOL)memoryPool::DEFAULT, &m_texture, nullptr);
   if (FAILED(hr))
      ReportError("Fatal Error: out of VRAM!"s, hr, __FILE__, __LINE__);

   hr = m_rd->GetCoreDevice()->UpdateTexture(sysTex, m_texture);
   if (FAILED(hr))
      ReportError("Fatal Error: uploading texture failed!"s, hr, __FILE__, __LINE__);

   SAFE_RELEASE(sysTex);

   if (texformat != colorFormat::DXT5 && m_rd->m_autogen_mipmap)
      m_texture->GenerateMipSubLevels(); // tell driver that now is a good time to generate mipmaps
#endif
}

#if defined(ENABLE_BGFX)
Sampler::Sampler(RenderDevice* rd, string name, SurfaceType type, bgfx::TextureHandle bgfxTexture, bgfx::TextureFormat::Enum bgfxFormat, unsigned int width, unsigned int height, bool ownTexture)
   : m_type(type)
   , m_name(std::move(name))
   , m_rd(rd)
   , m_ownTexture(ownTexture)
   , m_nomipsTexture(bgfxTexture)
   , m_width(width)
   , m_height(height)
   , m_bgfx_format(bgfxFormat)
{
   assert(bgfx::isValid(bgfxTexture));
   bgfx::setName(bgfxTexture, m_name.c_str());
}

#elif defined(ENABLE_OPENGL)
Sampler::Sampler(RenderDevice* rd, string name, SurfaceType type, GLuint glTexture, bool ownTexture)
   : m_type(type)
   , m_name(std::move(name))
   , m_rd(rd)
   , m_ownTexture(ownTexture)
{
   switch (m_type)
   {
   case SurfaceType::RT_DEFAULT: m_texTarget = GL_TEXTURE_2D; break;
   case SurfaceType::RT_STEREO: m_texTarget = GL_TEXTURE_2D_ARRAY; break;
   case SurfaceType::RT_CUBEMAP: m_texTarget = GL_TEXTURE_CUBE_MAP; break;
   default: assert(false);
   }
#ifndef __OPENGLES__
   glGetTexLevelParameteriv(m_texTarget, 0, GL_TEXTURE_WIDTH, (GLint*)&m_width);
   glGetTexLevelParameteriv(m_texTarget, 0, GL_TEXTURE_HEIGHT, (GLint*)&m_height);
#else
   m_width = 0;
   m_height = 0;
#endif
   int internal_format;
#ifndef __OPENGLES__
   glGetTexLevelParameteriv(m_texTarget, 0, GL_TEXTURE_INTERNAL_FORMAT, &internal_format);
#else
   internal_format = SRGBA;
#endif
   m_texture = glTexture;
#ifndef __OPENGLES__
   if (GLAD_GL_VERSION_4_3)
      glObjectLabel(GL_TEXTURE, m_texture, (GLsizei)m_name.length(), m_name.c_str());
#endif
}

#elif defined(ENABLE_DX9)
Sampler::Sampler(RenderDevice* rd, string name, IDirect3DTexture9* dx9Texture, bool ownTexture)
   : m_name(std::move(name))
   , m_type(SurfaceType::RT_DEFAULT)
   , m_ownTexture(ownTexture)
   , m_rd(rd)
{
   D3DSURFACE_DESC desc;
   dx9Texture->GetLevelDesc(0, &desc);
   m_width = desc.Width;
   m_height = desc.Height;
   m_texture = dx9Texture;
}
#endif

#if defined(ENABLE_BGFX)
// BGFX does not expose the release function in its API so we access it directly somewhat hackily
// The cleaner way would be to declare our own memory allocator and use it to release unused texture update (AllocatorStub in bgfx.cpp)
namespace bgfx { extern void release(const bgfx::Memory* _mem); }
#endif

Sampler::~Sampler()
{
   #if defined(ENABLE_BGFX)
   if (m_textureUpdate)
      bgfx::release(m_textureUpdate);
   if (m_ownTexture)
   {
      if (bgfx::isValid(m_nomipsTexture))
         bgfx::destroy(m_nomipsTexture);
      if (bgfx::isValid(m_mipsTexture))
         bgfx::destroy(m_mipsTexture);
   }

   #elif defined(ENABLE_OPENGL)
   if (m_ownTexture)
      glDeleteTextures(1, &m_texture);

   #elif defined(ENABLE_DX9)
   if (m_ownTexture)
      SAFE_RELEASE(m_texture);
   #endif
}

#if defined(ENABLE_BGFX)

#include "shaders/bgfx_mipmap.h"

bgfx::TextureHandle Sampler::GetCoreTexture(bool withMipmaps)
{
   // If this sampler is the resolved view of a MSAA render target, then resolve it before use
   if (m_msaaDepthResolve)
      m_msaaDepthResolve->ResolveMSAADepth();

   // If this is an external render target texture, just return it (no update or mipmap generation allowed)
   if (!m_ownTexture)
      return m_nomipsTexture;

   // Flag to keep a variant without mipmaps (discarded otherwise when a texture is both used with and without mipmap sampling)
   m_useNoMip |= !withMipmaps;

   // Handle texture initial creation as well as later updates on BGFX API thread
   if (m_textureUpdate)
   {
      const std::lock_guard lock(m_textureUpdateMutex);
      if (!bgfx::isValid(m_nomipsTexture))
      {
         m_nomipsTexture = bgfx::createTexture2D(m_width, m_height, false, 1, m_bgfx_format, m_isTextureUpdateLinear ? BGFX_TEXTURE_NONE : BGFX_TEXTURE_SRGB);
         bgfx::setName(m_nomipsTexture, (m_name + ".NoMipMap").c_str());
      }
      bgfx::updateTexture2D(m_nomipsTexture, 0, 0, 0, 0, m_width, m_height, m_textureUpdate);
      m_textureUpdate = nullptr;
      m_pendingMipMapGen = true;
   }

   if (withMipmaps && m_pendingMipMapGen)
   {
      // Hardware or BGFX internal implementation support ?
      if ((bgfx::getCaps()->formats[m_bgfx_format] & BGFX_CAPS_FORMAT_TEXTURE_MIP_AUTOGEN) == 0)
      {
         m_pendingMipMapGen = false; // Mipmaps will never be generated for this format, so don't keep the request pending
         return m_nomipsTexture;
      }

      // Defer mipmap generation if we are approaching BGFX limits or it is not supported
      if (m_rd->m_activeViewId < 2 // Check that we have enough views available to perform the resolution mipmapping
         || m_rd->m_activeViewId >= static_cast<int>(bgfx::getCaps()->limits.maxFrameBuffers) - 16 // We approximate the number of framebuffer used by the view index
         || m_rd->m_activeViewId >= static_cast<int>(bgfx::getCaps()->limits.maxViews) - 32)
         return m_nomipsTexture;

      if (bgfx::getRendererType() == bgfx::RendererType::Enum::Direct3D12 && !m_isTextureUpdateLinear)
      {
         // BGFX does not implement mipmap generation for Dirext3D12 sRGB texture (only linear formats), so we have to use a custom implementation
         assert(bgfx::getCaps()->formats[m_bgfx_format] & BGFX_CAPS_FORMAT_TEXTURE_IMAGE_WRITE);
         if (!bgfx::isValid(m_rd->m_srgbMipmapProgram))
         {
            bgfx::RendererType::Enum type = bgfx::getRendererType();
            static const bgfx::EmbeddedShader shaders[] = { BGFX_EMBEDDED_SHADER(cs_mipmap_srgba8), BGFX_EMBEDDED_SHADER_END() };
            m_rd->m_srgbMipmapProgram = bgfx::createProgram(bgfx::createEmbeddedShader(shaders, type, "cs_mipmap_srgba8"), true);
         }
         const bgfx::ProgramHandle program = m_rd->m_srgbMipmapProgram;

         if (!bgfx::isValid(m_mipsTexture))
         {
            m_mipsTexture = bgfx::createTexture2D(m_width, m_height, true, 1, m_bgfx_format, BGFX_TEXTURE_SRGB | BGFX_TEXTURE_BLIT_DST);
            bgfx::setName(m_mipsTexture, m_name.c_str());
         }

         // Generate mipmap
         const int numMipLevels = static_cast<int>(floor(log2(max(m_width, m_height)))) + 1;
         bgfx::TextureHandle csTexture = bgfx::createTexture2D(m_width, m_height, true, 1, m_bgfx_format, BGFX_TEXTURE_COMPUTE_WRITE | BGFX_TEXTURE_BLIT_DST);
         bgfx::setName(csTexture, (m_name + ".RGB").c_str());
         {
            bgfx::TextureRegion src;
            src.init(m_nomipsTexture);
            bgfx::TextureRegion dst;
            dst.init(csTexture);
            bgfx::blit(m_rd->m_activeViewId, dst, src);
         }
         for (uint8_t mip = 1; mip < numMipLevels; ++mip)
         {
            bgfx::setImage(0, csTexture, mip - 1, bgfx::Access::Read, m_bgfx_format);
            bgfx::setImage(1, csTexture, mip, bgfx::Access::Write, m_bgfx_format);
            bgfx::dispatch(m_rd->m_activeViewId, program, (std::max(1u, m_width >> mip) + 7) / 8, (std::max(1u, m_height >> mip) + 7) / 8);
         }

         // Blit to an sRGB texture (require a new view as BGFX order is blit -> compute -> draw)
         m_rd->NextView();
         for (uint8_t mip = 0; mip < numMipLevels; ++mip)
         {
            {
               bgfx::TextureRegion src;
               src.init(csTexture);
               src.mip = mip;
               bgfx::TextureRegion dst;
               dst.init(m_mipsTexture);
               dst.mip = mip;
               bgfx::blit(m_rd->m_activeViewId, dst, src);
            }
         }

         // Get back to the rendering view
         RenderTarget* activeRT = RenderTarget::GetCurrentRenderTarget();
         RenderTarget::OnFrameFlushed();
         if (activeRT)
            activeRT->Activate();

         bgfx::destroy(csTexture);
      }
      else
      {
         // Create the mipmapped texture and blit first mip level
         if (!bgfx::isValid(m_mipsTexture))
         {
            m_mipsTexture = bgfx::createTexture2D(m_width, m_height, true, 1, m_bgfx_format, (m_isTextureUpdateLinear ? BGFX_TEXTURE_NONE : BGFX_TEXTURE_SRGB) | BGFX_TEXTURE_RT | BGFX_TEXTURE_BLIT_DST);
            bgfx::setName(m_mipsTexture, m_name.c_str());
         }
         {
            bgfx::TextureRegion src;
            src.init(m_nomipsTexture);
            bgfx::TextureRegion dst;
            dst.init(m_mipsTexture);
            bgfx::blit(m_rd->m_activeViewId, dst, src);
         }

         // Create a frame buffer with the mipmap texture and force its resolution, in turns causing mipmap generation
         m_rd->NextView();
         bgfx::FrameBufferHandle mipsFramebuffer = bgfx::createFrameBuffer(1, &m_mipsTexture);
         bgfx::setViewFrameBuffer(m_rd->m_activeViewId, mipsFramebuffer);

         // Get back to the rendering view
         RenderTarget* activeRT = RenderTarget::GetCurrentRenderTarget();
         RenderTarget::OnFrameFlushed();
         if (activeRT)
            activeRT->Activate();

         // Mipmaps have been generated, we can release the framebuffer and base version of the texture (on a view processed after the one actually generating the mipmaps, to ensure correct command execution order)
         bgfx::destroy(mipsFramebuffer);
      }

      m_pendingMipMapGen = false;
      if (!m_useNoMip)
      {
         bgfx::destroy(m_nomipsTexture);
         m_nomipsTexture = BGFX_INVALID_HANDLE;
      }
      assert(m_rd->m_activeViewId >= 1);
   }

   if (withMipmaps && !m_pendingMipMapGen && bgfx::isValid(m_mipsTexture))
      return m_mipsTexture;

   if (bgfx::isValid(m_nomipsTexture))
      return m_nomipsTexture;

   // The no mip variant may have been discarded, so recreate it (simple blit)
   m_nomipsTexture
      = bgfx::createTexture2D(m_width, m_height, false, 1, m_bgfx_format, (m_isTextureUpdateLinear ? BGFX_TEXTURE_NONE : BGFX_TEXTURE_SRGB) | BGFX_TEXTURE_RT | BGFX_TEXTURE_BLIT_DST);
   bgfx::setName(m_nomipsTexture, (m_name + ".NoMipMap").c_str());
   {
      bgfx::TextureRegion src;
      src.init(m_mipsTexture);
      bgfx::TextureRegion dst;
      dst.init(m_nomipsTexture);
      bgfx::blit(m_rd->m_activeViewId, dst, src);
   }
   return m_nomipsTexture;
}

uintptr_t Sampler::GetNativeTexture()
{
   if (m_texture_override == 0)
   {
      // Lazily create a texture override
      bgfx::TextureHandle handle = GetCoreTexture(false);
      m_texture_override = bgfx::overrideInternal(handle, m_width, m_height, 0, m_bgfx_format, BGFX_TEXTURE_BLIT_DST | BGFX_SAMPLER_NONE);
   }
   return m_texture_override;
}
#endif

void Sampler::Unbind()
{
#ifdef ENABLE_OPENGL
   for (auto binding : m_bindings)
   {
      binding->sampler = nullptr;
      glActiveTexture(GL_TEXTURE0 + binding->unit);
      glBindTexture(m_texTarget, 0);
   }
   m_bindings.clear();
#endif
}

void Sampler::UpdateTexture(std::shared_ptr<const BaseTexture> surf, const bool force_linear_rgb)
{
   assert(m_ownTexture);
   assert(surf != nullptr);
   if (surf == nullptr) // then keep whatever was uploaded before
   {
      PLOGE << "Texture '" << m_name << "' has no data, previous content is kept";
      return;
   }
   m_rd->m_curTextureUpdates++;

#if defined(ENABLE_BGFX)
   const std::lock_guard lock(m_textureUpdateMutex);
   if (m_textureUpdate)
   {
      bgfx::release(m_textureUpdate);
      m_textureUpdate = nullptr;
   }

   // Instead of copying the data, we hold a strong reference on them until they are uploaded to the GPU (that's the reason we want a shared_ptr)
   struct SurfRef
   {
      Sampler* me;
      std::shared_ptr<const BaseTexture> surf;
   };
   auto ref = new SurfRef();
   ref->me = this;
   ref->surf = surf;
   auto releaseFn = [](void* _ptr, void* _userData)
   {
      SurfRef* ref = static_cast<SurfRef*>(_userData);
      delete ref;
   };

   m_isTextureUpdateLinear = BaseTexture::IsLinearFormat(surf->m_format) || force_linear_rgb;
   switch (surf->m_format)
   {
   case BaseTexture::BW: assert(m_bgfx_format == bgfx::TextureFormat::Enum::R8); break;
   case BaseTexture::BW_FP32: assert(m_bgfx_format == bgfx::TextureFormat::Enum::R32F); break;
   case BaseTexture::RGB: assert(m_bgfx_format == bgfx::TextureFormat::Enum::RGB8); break;
   case BaseTexture::RGBA: assert(m_bgfx_format == bgfx::TextureFormat::Enum::RGBA8); break;
   case BaseTexture::SRGB:
      if (m_bgfx_format == bgfx::TextureFormat::Enum::RGBA8)
         ref->surf = surf->Convert(BaseTexture::SRGBA);
      else
         assert(m_bgfx_format == bgfx::TextureFormat::Enum::RGB8);
      break;
   case BaseTexture::SRGBA: assert(m_bgfx_format == bgfx::TextureFormat::Enum::RGBA8); break;
   case BaseTexture::SRGB565:
      if (m_bgfx_format == bgfx::TextureFormat::Enum::RGBA8)
         ref->surf = surf->Convert(BaseTexture::SRGBA);
      else
         assert(m_bgfx_format == bgfx::TextureFormat::Enum::R5G6B5);
      break;
   case BaseTexture::RGB_FP16:
      assert(m_bgfx_format == bgfx::TextureFormat::Enum::RGBA16F);
      ref->surf = surf->Convert(BaseTexture::RGBA_FP16);
      break;
   case BaseTexture::RGBA_FP16: assert(m_bgfx_format == bgfx::TextureFormat::Enum::RGBA16F); break;
   case BaseTexture::RGB_FP32:
      assert(m_bgfx_format == bgfx::TextureFormat::Enum::RGBA32F);
      ref->surf = surf->Convert(BaseTexture::RGBA_FP32);
      break;
   case BaseTexture::RGBA_FP32: assert(m_bgfx_format == bgfx::TextureFormat::Enum::RGBA32F); break;
   default: assert(false); break;
   }

   assert(bgfx::isTextureValid(1, false, 1, m_bgfx_format, m_isTextureUpdateLinear ? BGFX_TEXTURE_NONE : BGFX_TEXTURE_SRGB));
   assert(bgfx::isTextureValid(1, false, 1, m_bgfx_format, (m_isTextureUpdateLinear ? BGFX_TEXTURE_NONE : BGFX_TEXTURE_SRGB) | BGFX_TEXTURE_RT | BGFX_TEXTURE_BLIT_DST));

   m_textureUpdate = bgfx::makeRef(ref->surf->datac(), ref->surf->height() * ref->surf->pitch(), releaseFn, ref);

#elif defined(ENABLE_OPENGL)
   // Widened to 8 bits per channel for the reason given in the constructor (GL cannot decode a 5.6.5 texture as sRGB)
   if (surf->m_format == BaseTexture::SRGB565)
      surf = surf->Convert(BaseTexture::SRGBA);
   colorFormat format;
   if (surf->m_format == BaseTexture::RGB)
      format = colorFormat::RGB;
   else if (surf->m_format == BaseTexture::RGBA)
      format = colorFormat::RGBA;
   else if (surf->m_format == BaseTexture::SRGB)
      format = colorFormat::SRGB;
   else if (surf->m_format == BaseTexture::SRGBA)
      format = colorFormat::SRGBA;
   else if (surf->m_format == BaseTexture::RGB_FP16)
      format = colorFormat::RGB16F;
   else if (surf->m_format == BaseTexture::RGBA_FP16)
      format = colorFormat::RGBA16F;
   else if (surf->m_format == BaseTexture::RGB_FP32)
      format = colorFormat::RGB32F;
   else if (surf->m_format == BaseTexture::BW_FP32)
      format = colorFormat::RED32F;
   else if (surf->m_format == BaseTexture::BW)
      format = colorFormat::GREY8;
   else
      assert(false); // Unsupported image format
   if (force_linear_rgb)
   {
      if (format == colorFormat::SRGB)
         format = colorFormat::RGB;
      else if (format == colorFormat::SRGBA)
         format = colorFormat::RGBA;
   }
   const GLuint col_type = ((format == RGBA32F) || (format == RGB32F)) ? GL_FLOAT : ((format == RGB16F) || (format == RGBA16F)) ? GL_HALF_FLOAT : (format == RGB5) ? GL_UNSIGNED_SHORT_5_6_5 : GL_UNSIGNED_BYTE;
   const GLuint col_format = ((format == GREY8) || (format == RED16F))                                                                                                      ? GL_RED
      : ((format == GREY_ALPHA) || (format == RG16F))                                                                                                                       ? GL_RG
      : ((format == RGB) || (format == RGB8) || (format == SRGB) || (format == SRGB8) || (format == RGB5) || (format == RGB10) || (format == RGB16F) || (format == RGB32F)) ? GL_RGB
                                                                                                                                                                            : GL_RGBA;
   // Update bind cache
   auto tex_unit = m_rd->m_samplerBindings.back();
   if (tex_unit->sampler != nullptr)
      tex_unit->sampler->m_bindings.erase(tex_unit);
   tex_unit->sampler = nullptr;
   glActiveTexture(GL_TEXTURE0 + tex_unit->unit);

   glBindTexture(m_texTarget, m_texture);
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   glTexSubImage2D(m_texTarget, 0, 0, 0, surf->width(), surf->height(), col_format, col_type, surf->datac());
   glGenerateMipmap(m_texTarget);
   glBindTexture(m_texTarget, 0);

#elif defined(ENABLE_DX9)
   colorFormat texformat;
   IDirect3DTexture9* sysTex = CreateSystemTexture(surf, force_linear_rgb, texformat);
   CHECKD3D(m_rd->GetCoreDevice()->UpdateTexture(sysTex, m_texture));
   SAFE_RELEASE(sysTex);

#endif
}

#if defined(ENABLE_BGFX)

#elif defined(ENABLE_OPENGL)
GLuint Sampler::CreateTexture(const std::shared_ptr<const BaseTexture>& surf, unsigned int Levels, colorFormat Format, int stereo)
{
   const unsigned int Width = surf->width();
   const unsigned int Height = surf->height();

   const GLuint col_type = ((Format == RGBA32F) || (Format == RGB32F)) ? GL_FLOAT : ((Format == RGB16F) || (Format == RGBA16F)) ? GL_HALF_FLOAT : GL_UNSIGNED_BYTE;
   const GLuint col_format = ((Format == GREY8) || (Format == RED16F))                                                                                                      ? GL_RED
      : ((Format == GREY_ALPHA) || (Format == RG16F))                                                                                                                       ? GL_RG
      : ((Format == RGB) || (Format == RGB8) || (Format == SRGB) || (Format == SRGB8) || (Format == RGB5) || (Format == RGB10) || (Format == RGB16F) || (Format == RGB32F)) ? GL_RGB
                                                                                                                                                                            : GL_RGBA;
   const bool col_is_linear = (Format == GREY8) || (Format == RED16F) || (Format == GREY_ALPHA) || (Format == RG16F) || (Format == RGB5) || (Format == RGB) || (Format == RGB8)
      || (Format == RGB10) || (Format == RGB16F) || (Format == RGB32F) || (Format == RGBA16F) || (Format == RGBA32F) || (Format == RGBA) || (Format == RGBA8) || (Format == RGBA10)
      || (Format == DXT5) || (Format == BC6U) || (Format == BC6S) || (Format == BC7);

   // Update bind cache
   auto tex_unit = m_rd->m_samplerBindings.back();
   if (tex_unit->sampler != nullptr)
      tex_unit->sampler->m_bindings.erase(tex_unit);
   tex_unit->sampler = nullptr;
   glActiveTexture(GL_TEXTURE0 + tex_unit->unit);

   GLuint texture;
   glGenTextures(1, &texture);
   glBindTexture(m_texTarget, texture);

   if (Format == GREY8)
   { //Hack so that GL_RED behaves as GL_GREY
      glTexParameteri(m_texTarget, GL_TEXTURE_SWIZZLE_R, GL_RED);
      glTexParameteri(m_texTarget, GL_TEXTURE_SWIZZLE_G, GL_RED);
      glTexParameteri(m_texTarget, GL_TEXTURE_SWIZZLE_B, GL_RED);
      glTexParameteri(m_texTarget, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);
      Format = RGB8;
   }
   else if (Format == GREY_ALPHA)
   { //Hack so that GL_RG behaves as GL_GREY_ALPHA
      glTexParameteri(m_texTarget, GL_TEXTURE_SWIZZLE_R, GL_RED);
      glTexParameteri(m_texTarget, GL_TEXTURE_SWIZZLE_G, GL_RED);
      glTexParameteri(m_texTarget, GL_TEXTURE_SWIZZLE_B, GL_RED);
      glTexParameteri(m_texTarget, GL_TEXTURE_SWIZZLE_A, GL_GREEN);
      Format = RGB8;
   }
   else
   { //Default
      glTexParameteri(m_texTarget, GL_TEXTURE_SWIZZLE_R, GL_RED);
      glTexParameteri(m_texTarget, GL_TEXTURE_SWIZZLE_G, GL_GREEN);
      glTexParameteri(m_texTarget, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
      glTexParameteri(m_texTarget, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);
   }

   colorFormat comp_format = Format;
   if (m_rd->m_compressTextures && ((Width & 3) == 0) && ((Height & 3) == 0) && (Width > 256) && (Height > 256))
   {
      if (col_type == GL_FLOAT || col_type == GL_HALF_FLOAT)
      {
#ifndef __OPENGLES__
         if (GLAD_GL_ARB_texture_compression_bptc)
            comp_format = colorFormat::BC6S;
#endif
      }
#ifndef __OPENGLES__
      else if (GLAD_GL_ARB_texture_compression_bptc)
         comp_format = col_is_linear ? colorFormat::BC7 : colorFormat::SBC7;
#endif
      else
         comp_format = col_is_linear ? colorFormat::DXT5 : colorFormat::SDXT5;
   }

   const int num_mips = (int)std::log2(float(max(Width, Height))) + 1;
#ifndef __OPENGLES__
   if (m_rd->getGLVersion() >= 403)
#endif
      glTexStorage2D(m_texTarget, num_mips, comp_format, Width, Height);
#ifndef __OPENGLES__
   else
   { // should never be triggered nowadays
      GLsizei w = Width;
      GLsizei h = Height;
      for (int i = 0; i < num_mips; i++)
      {
         glTexImage2D(m_texTarget, i, comp_format, w, h, 0, col_format, col_type, nullptr);
         w = max(1, (w / 2));
         h = max(1, (h / 2));
      }
   }
#else
   PLOGD.printf("col_is_linear=%d, comp_format: %s (0x%04x), col_format=%s (0x%04x), col_type=%s (0x%04x)",
          col_is_linear, 
          gl_to_string(comp_format), comp_format,
          gl_to_string(col_format), col_format,
          gl_to_string(col_type), col_type);
#endif

#ifndef __OPENGLES__
   if (GLAD_GL_VERSION_4_3)
      glObjectLabel(GL_TEXTURE, texture, (GLsizei)m_name.length(), m_name.c_str());
#endif

   if (surf->datac())
   {
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      // This line causes a false GLIntercept error log on OpenGL >= 403 since the image is initialized through TexStorage and not TexImage (expected by GLIntercept)
      // InterceptImage::SetImageDirtyPost - Flagging an image as dirty when it is not ready/init?
      glTexSubImage2D(m_texTarget, 0, 0, 0, Width, Height, col_format, col_type, surf->datac());
      glGenerateMipmap(m_texTarget); // Generate mip-maps, when using TexStorage will generate same amount as specified in TexStorage, otherwise good idea to limit by GL_TEXTURE_MAX_LEVEL
   }
   return texture;
}

#elif defined(ENABLE_DX9)

IDirect3DTexture9* Sampler::CreateSystemTexture(std::shared_ptr<const BaseTexture> surf, const bool force_linear_rgb, colorFormat& texformat)
{
   const unsigned int texwidth = surf->width();
   const unsigned int texheight = surf->height();
   const BaseTexture::Format basetexformat = surf->m_format;

   switch (basetexformat)
   {
   case BaseTexture::RGB_FP16: texformat = colorFormat::RGBA16F; break;
   case BaseTexture::RGBA_FP16: texformat = colorFormat::RGBA16F; break;
   case BaseTexture::RGB_FP32: texformat = colorFormat::RGBA32F; break;
   case BaseTexture::SRGB565: texformat = colorFormat::RGB5; break;
   case BaseTexture::BW_FP32: texformat = colorFormat::RED32F; break;
   default:
      texformat = colorFormat::RGBA8;
      if (m_rd->m_compressTextures && ((texwidth & 3) == 0) && ((texheight & 3) == 0) && (texwidth > 256) && (texheight > 256))
         texformat = colorFormat::DXT5;
   }

   IDirect3DTexture9* sysTex;
   HRESULT hr = m_rd->GetCoreDevice()->CreateTexture(
      texwidth, texheight, (texformat != colorFormat::DXT5 && m_rd->m_autogen_mipmap) ? 1 : 0, 0, (D3DFORMAT)texformat, (D3DPOOL)memoryPool::SYSTEM, &sysTex, nullptr);
   if (FAILED(hr))
   {
      ReportError("Fatal Error: unable to create texture!"s, hr, __FILE__, __LINE__);
   }

   D3DLOCKED_RECT locked;
   CHECKD3D(sysTex->LockRect(0, &locked, nullptr, 0));

   // copy data into system memory texture
   if (basetexformat == BaseTexture::RGB_FP32 && texformat == colorFormat::RGBA32F)
   {
      float* const __restrict pdest = (float*)locked.pBits;
      const float* const __restrict psrc = (const float*)(surf->datac());
      for (size_t i = 0; i < (size_t)texwidth * texheight; ++i)
      {
         pdest[i * 4 + 0] = psrc[i * 3 + 0];
         pdest[i * 4 + 1] = psrc[i * 3 + 1];
         pdest[i * 4 + 2] = psrc[i * 3 + 2];
         pdest[i * 4 + 3] = 1.f;
      }
   }
   else if (basetexformat == BaseTexture::RGB_FP16 && texformat == colorFormat::RGBA16F)
   {
      uint16_t* const __restrict pdest = (uint16_t*)locked.pBits;
      const uint16_t* const __restrict psrc = (const uint16_t*)(surf->datac());
      constexpr uint16_t one16 = float2half_noLUT(1.f);
      for (size_t i = 0; i < (size_t)texwidth * texheight; ++i)
      {
         pdest[i * 4 + 0] = psrc[i * 3 + 0];
         pdest[i * 4 + 1] = psrc[i * 3 + 1];
         pdest[i * 4 + 2] = psrc[i * 3 + 2];
         pdest[i * 4 + 3] = one16;
      }
   }
   else if (basetexformat == BaseTexture::RGBA_FP16 && texformat == colorFormat::RGBA16F)
   {
      uint16_t* const __restrict pdest = (uint16_t*)locked.pBits;
      const uint16_t* const __restrict psrc = (const uint16_t*)(surf->datac());
      memcpy(pdest, psrc, (size_t)texwidth * texheight*4*sizeof(uint16_t));
   }
   else if ((basetexformat == BaseTexture::BW) && texformat == colorFormat::RGBA8)
   {
      uint8_t* const __restrict pdest = (uint8_t*)locked.pBits;
      const uint8_t* const __restrict psrc = (const uint8_t*)surf->datac();
      for (size_t i = 0; i < (size_t)texwidth * texheight; ++i)
      {
         pdest[i * 4 + 0] =
         pdest[i * 4 + 1] =
         pdest[i * 4 + 2] = psrc[i];
         pdest[i * 4 + 3] = 255u;
      }
   }
   else if ((basetexformat == BaseTexture::RGB || basetexformat == BaseTexture::SRGB) && texformat == colorFormat::RGBA8)
   {
      copy_rgb_rgba<true>((unsigned int*)locked.pBits, (const uint8_t*)surf->datac(), (size_t)texwidth * texheight);
   }
   else if ((basetexformat == BaseTexture::RGBA || basetexformat == BaseTexture::SRGBA) && texformat == colorFormat::RGBA8)
   {
      copy_bgra_rgba<false>((unsigned int*)locked.pBits, (const unsigned int*)(surf->datac()), (size_t)texwidth * texheight);

      /* IDirect3DSurface9* sysSurf;
      CHECKD3D(sysTex->GetSurfaceLevel(0, &sysSurf));
      RECT sysRect;
      sysRect.top = 0;
      sysRect.left = 0;
      sysRect.right = texwidth;
      sysRect.bottom = texheight;
      CHECKD3D(D3DXLoadSurfaceFromMemory(sysSurf, nullptr, nullptr, surf->datac(), (D3DFORMAT)colorFormat::RGBA8, surf->pitch(), nullptr, &sysRect, D3DX_FILTER_NONE, 0));
      SAFE_RELEASE_NO_RCC(sysSurf);*/
   }
   else if (basetexformat == BaseTexture::SRGB565 && texformat == colorFormat::RGB5)
   {
      uint16_t* const __restrict pdest = (uint16_t*)locked.pBits;
      const uint16_t* const __restrict psrc = (const uint16_t*)(surf->datac());
      memcpy(pdest, psrc, (size_t)texwidth * texheight * 2);
   }
   else if (basetexformat == BaseTexture::BW_FP32 && texformat == colorFormat::RED32F)
   {
      float* const __restrict pdest = (float*)locked.pBits;
      const float* const __restrict psrc = (const float*)(surf->datac());
      memcpy(pdest, psrc, (size_t)texwidth * texheight * 4);
   }
   else
      assert(false); // Unsupported image format

   CHECKD3D(sysTex->UnlockRect(0));

   if (!(texformat != colorFormat::DXT5 && m_rd->m_autogen_mipmap))
      // normal maps or float textures are already in linear space!
      CHECKD3D(D3DXFilterTexture(sysTex, nullptr, D3DX_DEFAULT,
         (texformat == colorFormat::RGBA16F || texformat == colorFormat::RGBA32F || force_linear_rgb) ? D3DX_FILTER_TRIANGLE : (D3DX_FILTER_TRIANGLE | D3DX_FILTER_SRGB)));

   return sysTex;
}
#endif
