#pragma once

#ifdef ENABLE_SDL
 #include <GL/glew.h>
 //#include <glad/glad.h>
 #include <sdl2/SDL_opengl.h>
 #include <sdl2/SDL.h>
 #include <sdl2/SDL_ttf.h>
#else
 #include <d3d9.h>
 #include <d3dx9.h>
#endif

#ifdef ENABLE_VR
 #include "openvr.h"
#endif

#ifdef ENABLE_SDL

enum colorFormat {
   GREY = GL_RED,
   GREY_ALPHA = GL_RG8,
   RGB = GL_RGB8,
   RGB5 = GL_RGB5,
   RGB8 = GL_RGB8,
   RGB10 = GL_RGB10_A2,
   RGB32 = GL_RGB32F,
   RGBA16 = GL_RGBA16F,
   RGBA32 = GL_RGBA32F,
   RGBA = GL_RGBA8,
   RGBA8 = GL_RGBA8,
   RGBA10 = GL_RGB10_A2,
   DXT5 = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
};

enum textureUsage {
   RENDERTARGET_VR = 16,
   RENDERTARGET = 8,
   RENDERTARGET_DEPTH = 12,
   DEPTH = 4,
   MIPMAP = 2,
   STATIC = 0,
   DYNAMIC = 1
};

struct RenderTarget {
   colorFormat format;
   textureUsage usage;
   GLuint texture = 0, zTexture = 0, framebuffer = 0, zBuffer = 0;
   GLuint width = 0, height = 0;
   GLint slot = -1;//Current slot for caching
   int stereo = 0;
};

typedef RenderTarget D3DTexture;//It's easier to have them equal than saving 8 bytes and have a lot of trouble.

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
typedef IDirect3DTexture9 D3DTexture;
typedef D3DVIEWPORT9 ViewPort;
typedef IDirect3DSurface9 RenderTarget;
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
