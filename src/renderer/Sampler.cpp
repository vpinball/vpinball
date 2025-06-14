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
#include <bgfx/platform.h>
#endif

Sampler::Sampler(RenderDevice* rd, string name, std::shared_ptr<const BaseTexture> surf, const bool force_linear_rgb)
   : m_name(std::move(name))
   , m_type(SurfaceType::RT_DEFAULT)
   , m_ownTexture(true)
   , m_rd(rd)
   , m_width(surf->width())
   , m_height(surf->height())
{
#if defined(ENABLE_BGFX)
   switch (surf->m_format)
   {
   case BaseTexture::BW: m_bgfx_format = bgfx::TextureFormat::Enum::R8; break;
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
   if (surf)
      UpdateTexture(surf, force_linear_rgb);

#elif defined(ENABLE_OPENGL)
   m_rd->m_curTextureUpdates++;
   m_texTarget = GL_TEXTURE_2D;
   colorFormat format;
   if (surf->m_format == BaseTexture::RGB)
      format = colorFormat::RGB;
   else if (surf->m_format == BaseTexture::RGBA)
      format = colorFormat::RGBA;
   else if (surf->m_format == BaseTexture::SRGB)
      format = force_linear_rgb ? colorFormat::RGB : colorFormat::SRGB;
   else if (surf->m_format == BaseTexture::SRGBA)
      format = force_linear_rgb ? colorFormat::RGBA : colorFormat::SRGBA;
   else if (surf->m_format == BaseTexture::SRGB565)
      format = colorFormat::RGB5; // FIXME this is incorrect sRGB wise
   else if (surf->m_format == BaseTexture::RGB_FP16)
      format = colorFormat::RGB16F;
   else if (surf->m_format == BaseTexture::RGBA_FP16)
      format = colorFormat::RGBA16F;
   else if (surf->m_format == BaseTexture::RGB_FP32)
      format = colorFormat::RGB32F;
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
      ReportError("Fatal Error: out of VRAM!", hr, __FILE__, __LINE__);

   hr = m_rd->GetCoreDevice()->UpdateTexture(sysTex, m_texture);
   if (FAILED(hr))
      ReportError("Fatal Error: uploading texture failed!", hr, __FILE__, __LINE__);

   SAFE_RELEASE(sysTex);

   if (texformat != colorFormat::DXT5 && m_rd->m_autogen_mipmap)
      m_texture->GenerateMipSubLevels(); // tell driver that now is a good time to generate mipmaps
#endif
}

#if defined(ENABLE_BGFX)
Sampler::Sampler(RenderDevice* rd, string name, SurfaceType type, bgfx::TextureHandle bgfxTexture, unsigned int width, unsigned int height, bool ownTexture)
   : m_type(type)
   , m_name(std::move(name))
   , m_rd(rd)
   , m_ownTexture(ownTexture)
   , m_mipsTexture(bgfxTexture)
   , m_width(width)
   , m_height(height)
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
   case RT_DEFAULT: m_texTarget = GL_TEXTURE_2D; break;
   case RT_STEREO: m_texTarget = GL_TEXTURE_2D_ARRAY; break;
   case RT_CUBEMAP: m_texTarget = GL_TEXTURE_CUBE_MAP; break;
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
   if (bgfx::isValid(m_nomipsTexture))
      bgfx::destroy(m_nomipsTexture);
   if (m_ownTexture && bgfx::isValid(m_mipsTexture))
      bgfx::destroy(m_mipsTexture);

   #elif defined(ENABLE_OPENGL)
   if (m_ownTexture)
      glDeleteTextures(1, &m_texture);

   #elif defined(ENABLE_DX9)
   if (m_ownTexture)
      SAFE_RELEASE(m_texture);
   #endif
}

#if defined(ENABLE_BGFX)
bgfx::TextureHandle Sampler::GetCoreTexture(bool genMipmaps)
{
   assert(m_textureUpdate || bgfx::isValid(m_nomipsTexture) || bgfx::isValid(m_mipsTexture));
   // Handle texture initial creation loading and updates on BGFX API thread
   if (m_textureUpdate)
   {
      const std::lock_guard<std::mutex> lock(m_textureUpdateMutex);
      if (!bgfx::isValid(m_nomipsTexture))
      {
         m_nomipsTexture = bgfx::createTexture2D(m_width, m_height, false, 1, m_bgfx_format, m_isTextureUpdateLinear ? BGFX_TEXTURE_NONE : BGFX_TEXTURE_SRGB);
         bgfx::setName(m_nomipsTexture, (m_name + ".NoMipMap").c_str());
      }
      bgfx::updateTexture2D(m_nomipsTexture, 0, 0, 0, 0, m_width, m_height, m_textureUpdate);
      m_textureUpdate = nullptr;
   }
   // Handle mipmap generation on BGFX API thread
   if (bgfx::isValid(m_nomipsTexture))
   {
      // Defer mipmap generation if we are approaching BGFX limits (using magic margins) or it is not needed
      if (!genMipmaps
       || m_rd->m_activeViewId < 0
       || m_rd->m_activeViewId >= static_cast<int>(bgfx::getCaps()->limits.maxFrameBuffers) - 16 // We approximate the number of framebuffer used by the view index
       || m_rd->m_activeViewId >= static_cast<int>(bgfx::getCaps()->limits.maxViews) - 32)
         return m_nomipsTexture;
      // TODO BGFX a clean GPU mipmap generation with Kaiser filter would be better than doing a blit to trigger default's driver render target mipmap generation
      // For a simple and readable reference, see (parameters: alpha=4, stretch=1, m_width=filter half width):
      //   https://github.com/castano/nvidia-texture-tools/blob/aeddd65f81d36d8cb7b169b469ef25156666077e/src/nvimage/Filter.cpp#L257
      //   https://github.com/castano/nvidia-texture-tools/blob/aeddd65f81d36d8cb7b169b469ef25156666077e/src/nvimage/Filter.cpp#L64
      // Create a frame buffer and blit texture to it
      if (!bgfx::isValid(m_mipsTexture))
      {
         m_mipsTexture = bgfx::createTexture2D(m_width, m_height, true, 1, m_bgfx_format, (m_isTextureUpdateLinear ? BGFX_TEXTURE_NONE : BGFX_TEXTURE_SRGB) | BGFX_TEXTURE_RT | BGFX_TEXTURE_BLIT_DST);
         bgfx::setName(m_mipsTexture, m_name.c_str());
      }
      bgfx::FrameBufferHandle mipsFramebuffer = bgfx::createFrameBuffer(1, &m_mipsTexture);
      bgfx::blit(m_rd->m_activeViewId, m_mipsTexture, 0, 0, m_nomipsTexture);
      // Force frame buffer resolution, in turns causing mipmap generation
      m_rd->NextView();
      bgfx::setViewFrameBuffer(m_rd->m_activeViewId, mipsFramebuffer);
      // Get back to the rendering view
      RenderTarget* activeRT = RenderTarget::GetCurrentRenderTarget();
      RenderTarget::OnFrameFlushed();
      if (activeRT)
        activeRT->Activate();
      // Mipmaps have been generated, we can release the framebuffer and base version of the texture (on a view processed after the one actually generating the mipmaps, to ensure correct command execution order)
      bgfx::destroy(mipsFramebuffer);
      // TODO if a texture is used with and without mipmaps, then the noMips variant may be in used in this frame (this does not happen in practice)
      bgfx::destroy(m_nomipsTexture);
      m_nomipsTexture = BGFX_INVALID_HANDLE;
   }
   return m_mipsTexture;
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
   m_rd->m_curTextureUpdates++;

#if defined(ENABLE_BGFX)
   const std::lock_guard<std::mutex> lock(m_textureUpdateMutex);
   if (m_textureUpdate)
      bgfx::release(m_textureUpdate);
   m_textureUpdate = nullptr;
   m_isTextureUpdateLinear = BaseTexture::IsLinearFormat(surf->m_format) || force_linear_rgb;
   switch (surf->m_format)
   {
   case BaseTexture::BW: assert(m_bgfx_format == bgfx::TextureFormat::Enum::R8); break;
   case BaseTexture::RGB: assert(m_bgfx_format == bgfx::TextureFormat::Enum::RGB8); break;
   case BaseTexture::RGBA: assert(m_bgfx_format == bgfx::TextureFormat::Enum::RGBA8); break;
   case BaseTexture::SRGB:
      if (m_bgfx_format == bgfx::TextureFormat::Enum::RGBA8)
      {
         BaseTexture* tmp = surf->Convert(BaseTexture::SRGBA);
         auto releaseFn = [](void* _ptr, void* _userData) { delete static_cast<BaseTexture*>(_userData); };
         m_textureUpdate = bgfx::makeRef(tmp->datac(), m_height * tmp->pitch(), releaseFn, (void*)tmp);
      }
      else
         assert(m_bgfx_format == bgfx::TextureFormat::Enum::RGB8);
      break;
   case BaseTexture::SRGBA: assert(m_bgfx_format == bgfx::TextureFormat::Enum::RGBA8); break;
   case BaseTexture::SRGB565:
      if (m_bgfx_format == bgfx::TextureFormat::Enum::RGBA8)
      {
         BaseTexture* tmp = surf->Convert(BaseTexture::SRGBA);
         auto releaseFn = [](void* _ptr, void* _userData) { delete static_cast<BaseTexture*>(_userData); };
         m_textureUpdate = bgfx::makeRef(tmp->datac(), m_height * tmp->pitch(), releaseFn, (void*)tmp);
      }
      else
         assert(m_bgfx_format == bgfx::TextureFormat::Enum::R5G6B5);
      break;
   case BaseTexture::RGB_FP16:
      assert(m_bgfx_format == bgfx::TextureFormat::Enum::RGBA16F);
      {
         BaseTexture* tmp = surf->Convert(BaseTexture::RGBA_FP16);
         auto releaseFn = [](void* _ptr, void* _userData) { delete static_cast<BaseTexture*>(_userData); };
         m_textureUpdate = bgfx::makeRef(tmp->datac(), m_height * tmp->pitch(), releaseFn, (void*)tmp);
      }
      break;
   case BaseTexture::RGBA_FP16: assert(m_bgfx_format == bgfx::TextureFormat::Enum::RGBA16F); break;
   case BaseTexture::RGB_FP32:
      assert(m_bgfx_format == bgfx::TextureFormat::Enum::RGBA32F);
      {
         BaseTexture* tmp = surf->Convert(BaseTexture::RGBA_FP32);
         auto releaseFn = [](void* _ptr, void* _userData) { delete static_cast<BaseTexture*>(_userData); };
         m_textureUpdate = bgfx::makeRef(tmp->datac(), m_height * tmp->pitch(), releaseFn, (void*)tmp);
      }
      break;
   case BaseTexture::RGBA_FP32: assert(m_bgfx_format == bgfx::TextureFormat::Enum::RGBA32F); break;
   default: assert(false); break;
   }

   assert(bgfx::isTextureValid(1, false, 1, m_bgfx_format, m_isTextureUpdateLinear ? BGFX_TEXTURE_NONE : BGFX_TEXTURE_SRGB));
   assert(bgfx::isTextureValid(1, false, 1, m_bgfx_format, (m_isTextureUpdateLinear ? BGFX_TEXTURE_NONE : BGFX_TEXTURE_SRGB) | BGFX_TEXTURE_RT | BGFX_TEXTURE_BLIT_DST));

   if (m_textureUpdate == nullptr)
      m_textureUpdate = bgfx::copy(surf->datac(), static_cast<uint32_t>(surf->height() * surf->pitch()));

#elif defined(ENABLE_OPENGL)
   colorFormat format;
   if (surf->m_format == BaseTexture::RGB)
      format = colorFormat::RGB;
   else if (surf->m_format == BaseTexture::RGBA)
      format = colorFormat::RGBA;
   else if (surf->m_format == BaseTexture::SRGB)
      format = colorFormat::SRGB;
   else if (surf->m_format == BaseTexture::SRGBA)
      format = colorFormat::SRGBA;
   else if (surf->m_format == BaseTexture::SRGB565)
      format = colorFormat::RGB5;
   else if (surf->m_format == BaseTexture::RGB_FP16)
      format = colorFormat::RGB16F;
   else if (surf->m_format == BaseTexture::RGBA_FP16)
      format = colorFormat::RGBA16F;
   else if (surf->m_format == BaseTexture::RGB_FP32)
      format = colorFormat::RGB32F;
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
   glGenerateMipmap(m_texTarget); // Generate mip-maps
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
GLuint Sampler::CreateTexture(std::shared_ptr<const BaseTexture> surf, unsigned int Levels, colorFormat Format, int stereo)
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
      glObjectLabel(GL_TEXTURE, m_texture, (GLsizei)m_name.length(), m_name.c_str());
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

   if (basetexformat == BaseTexture::RGB_FP16 || basetexformat == BaseTexture::RGBA_FP16)
   {
      texformat = colorFormat::RGBA16F;
   }
   else if (basetexformat == BaseTexture::RGB_FP32)
   {
      texformat = colorFormat::RGBA32F;
   }
   else if (basetexformat == BaseTexture::SRGB565)
   {
      texformat = colorFormat::RGB5;
   }
   else
   {
      texformat = colorFormat::RGBA8;
      if (m_rd->m_compressTextures && ((texwidth & 3) == 0) && ((texheight & 3) == 0) && (texwidth > 256) && (texheight > 256))
         texformat = colorFormat::DXT5;
   }

   IDirect3DTexture9* sysTex;
   HRESULT hr = m_rd->GetCoreDevice()->CreateTexture(
      texwidth, texheight, (texformat != colorFormat::DXT5 && m_rd->m_autogen_mipmap) ? 1 : 0, 0, (D3DFORMAT)texformat, (D3DPOOL)memoryPool::SYSTEM, &sysTex, nullptr);
   if (FAILED(hr))
   {
      ReportError("Fatal Error: unable to create texture!", hr, __FILE__, __LINE__);
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
      unsigned short* const __restrict pdest = (unsigned short*)locked.pBits;
      const unsigned short* const __restrict psrc = (const unsigned short*)(surf->datac());
      const unsigned short one16 = float2half_noLUT(1.f);
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
      unsigned short* const __restrict pdest = (unsigned short*)locked.pBits;
      const unsigned short* const __restrict psrc = (const unsigned short*)(surf->datac());
      memcpy(pdest, psrc, (size_t)texwidth * texheight*4*sizeof(unsigned short));
   }
   else if ((basetexformat == BaseTexture::BW) && texformat == colorFormat::RGBA8)
   {
      BYTE* const __restrict pdest = (BYTE*)locked.pBits;
      const BYTE* const __restrict psrc = surf->datac();
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
      copy_rgb_rgba<true>((unsigned int*)locked.pBits, surf->datac(), (size_t)texwidth * texheight);
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
      unsigned short* const __restrict pdest = (unsigned short*)locked.pBits;
      const unsigned short* const __restrict psrc = (const unsigned short*)(surf->datac());
      memcpy(pdest, psrc, (size_t)texwidth * texheight * 2);
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
