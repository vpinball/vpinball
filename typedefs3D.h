#pragma once

#ifdef ENABLE_SDL
#ifndef __OPENGLES__
 #include <glad/glad.h>
#else
 #include <glad/gles2.h>
#endif
 #include <sdl2/SDL_opengl.h>
 #include <sdl2/SDL.h>
 #include <sdl2/SDL_ttf.h>
#else
 #include "inc/minid3d9.h"
 #include <d3dx9.h>
#endif

#ifdef ENABLE_VR
 #include "openvr.h"
#endif

enum deviceNumber {
   PRIMARY_DEVICE,
   SECONDARY_DEVICE
};

enum StereoMode
{
   STEREO_OFF = 0, // Disabled
   STEREO_TB = 1, // TB (Top / Bottom)
   STEREO_INT = 2, // Interlaced (e.g. LG TVs)
   STEREO_FLIPPED_INT = 3, // Flipped Interlaced (e.g. LG TVs)
   STEREO_SBS = 4, // SBS (Side by Side)
   STEREO_ANAGLYPH_RC = 5, // Anaglyph Red/Cyan
   STEREO_ANAGLYPH_GM = 6, // Anaglyph Green/Magenta
   STEREO_ANAGLYPH_DUBOIS_RC = 7, // Anaglyph Dubois Red/Cyan
   STEREO_ANAGLYPH_DUBOIS_GM = 8, // Anaglyph Dubois Green/Magenta
   STEREO_ANAGLYPH_DEGHOSTED_RC = 9, // Anaglyph Deghosted Red/Cyan
   STEREO_ANAGLYPH_DEGHOSTED_GM = 10, // Anaglyph Deghosted Green/Magenta
   STEREO_ANAGLYPH_BA = 11, // Anaglyph Blue/Amber
   STEREO_ANAGLYPH_CR = 12, // Anaglyph Cyan/Red
   STEREO_ANAGLYPH_MR, // Anaglyph Magenta/Green
   STEREO_ANAGLYPH_DUBOIS_CR = 14, // Anaglyph Dubois Cyan/Red
   STEREO_ANAGLYPH_DUBOIS_MG = 15, // Anaglyph Dubois Magenta/Green
   STEREO_ANAGLYPH_DEGHOSTED_CR = 16, // Anaglyph Deghosted Cyan/Red
   STEREO_ANAGLYPH_DEGHOSTED_MG = 17, // Anaglyph Deghosted Magenta/Green
   STEREO_ANAGLYPH_AB = 18, // Anaglyph Amber/Blue
   STEREO_VR = 19, // Hardware VR set (not supported by DX9)
};

#ifdef ENABLE_SDL

#define MAX_DEVICE_IDENTIFIER_STRING 512
#define D3DADAPTER_DEFAULT 0

#ifndef GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM
#define GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM 0x8E8D
#endif

enum colorFormat {
   GREY8 = GL_RED,
   RED16F = GL_R16F,

   GREY_ALPHA = GL_RG8,
   RG16F = GL_RG16F,

   RGB5 = GL_RGB5,
   RGB = GL_RGB8,
   RGB8 = GL_RGB8,
   RGB10 = GL_RGB10_A2,
   RGB16F = GL_RGB16F,
   RGB32F = GL_RGB32F,

   SRGB = GL_SRGB8,
   SRGB8 = GL_SRGB8,

   RGBA16F = GL_RGBA16F,
   RGBA32F = GL_RGBA32F,
   RGBA = GL_RGBA8,
   RGBA8 = GL_RGBA8,
   RGBA10 = GL_RGB10_A2,
   DXT5 = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,
#ifdef GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT
   BC6S = GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT,
   BC6U = GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT,
   BC7 = GL_COMPRESSED_RGBA_BPTC_UNORM,
#else
   BC6S = GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB,
   BC6U = GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB,
   BC7 = GL_COMPRESSED_RGBA_BPTC_UNORM_ARB,
#endif

   SRGBA = GL_SRGB8_ALPHA8,
   SRGBA8 = GL_SRGB8_ALPHA8,
   SDXT5 = 0x8C4F, // GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT,
   SBC7 = GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM
};

enum textureUsage {
   RENDERTARGET = 8,
   RENDERTARGET_DEPTH = 12,
   RENDERTARGET_MSAA = 14,
   RENDERTARGET_MSAA_DEPTH = 16,
   DEPTH = 4,
   AUTOMIPMAP = 2,
   STATIC = 0,
   DYNAMIC = 1
};

struct ViewPort  {
   union {
      struct {
         SDL_Rect sdl_rect;
         float front;
         float rear;
      };
      struct {
         DWORD X;
         DWORD Y;
         DWORD Width;
         DWORD Height;
         float MinZ;
         float MaxZ;
      };
   };
};

struct VertexElement {
   GLint size;
   GLenum type;
   GLboolean normalized;
   GLsizei stride;
   const GLchar* name;
};

typedef VertexElement* VertexDeclaration;

enum memoryPool {
   SYSTEM = 0,
   DEFAULT = 1
};

enum clearType {
   ZBUFFER = GL_DEPTH_BUFFER_BIT,
   TARGET = GL_COLOR_BUFFER_BIT
};

#else

typedef LPD3DXFONT FontHandle;
typedef D3DVIEWPORT9 ViewPort;
typedef D3DVERTEXELEMENT9 VertexElement;
typedef IDirect3DVertexDeclaration9 VertexDeclaration;

enum colorFormat {
   GREY8 = D3DFMT_L8,
   GREYA8 = D3DFMT_A8L8,
   RED16F = D3DFMT_R16F,
   RG16F = D3DFMT_G16R16F,
   RGB5 = D3DFMT_R5G6B5,
   RGB8 = D3DFMT_X8R8G8B8,
   //RGB10 = D3DFMT_A2R10G10B10,
   //RGB16F = would be more appropriate for our use cases, but does not exist in DX9 :/
   //RGB32F = would be more appropriate for our use cases, but does not exist in DX9 :/
   RGBA16F = D3DFMT_A16B16G16R16F,
   RGBA32F = D3DFMT_A32B32G32R32F,
   RGBA8 = D3DFMT_A8R8G8B8,
   RGBA10 = D3DFMT_A2R10G10B10,
   DXT5 = D3DFMT_DXT5
};

enum memoryPool {
   SYSTEM = D3DPOOL_SYSTEMMEM,
   DEFAULT = D3DPOOL_DEFAULT
};

enum clearType {
   ZBUFFER = D3DCLEAR_ZBUFFER,
   TARGET = D3DCLEAR_TARGET
};

enum textureUsage {
   RENDERTARGET = D3DUSAGE_RENDERTARGET,
   RENDERTARGET_DEPTH = D3DUSAGE_RENDERTARGET,
   DEPTH = D3DUSAGE_DEPTHSTENCIL,
   AUTOMIPMAP = D3DUSAGE_AUTOGENMIPMAP,
   //STATIC = D3DUSAGE_WRITEONLY,
   //DYNAMIC = D3DUSAGE_DYNAMIC
};

#endif
