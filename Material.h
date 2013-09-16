#include "stdafx.h"
#pragma once


typedef struct  
{
   union {
      D3DCOLORVALUE   diffuse;        /* Diffuse color RGBA */
      D3DCOLORVALUE   dcvDiffuse;
      __m128          d;
   };
   union {
      D3DCOLORVALUE   ambient;        /* Ambient color RGB */
      D3DCOLORVALUE   dcvAmbient;
      __m128          a;
   };
   union {
      D3DCOLORVALUE   specular;       /* Specular 'shininess' */
      D3DCOLORVALUE   dcvSpecular;
      __m128          s;
   };
   union {
      D3DCOLORVALUE   emissive;       /* Emissive color RGB */
      D3DCOLORVALUE   dcvEmissive;
      __m128          e;
   };
   union {
      D3DVALUE        power;          /* Sharpness if specular highlight */
      D3DVALUE        dvPower;
   };
}BaseMaterial;

class RenderDevice;

class Material
{
public:

   __forceinline Material()
   {
      mat.specular.r = 0.0f;
      mat.specular.g = 0.0f;
      mat.specular.b = 0.0f;
      mat.specular.a = 0.0f;
      mat.emissive.r = 0.0f;
      mat.emissive.g = 0.0f;
      mat.emissive.b = 0.0f;
      mat.emissive.a = 0.0f;
      mat.power = 0.0f;
      mat.diffuse.r = 1.0f;
      mat.diffuse.g = 1.0f;
      mat.diffuse.b = 1.0f;
      mat.diffuse.a = 1.0f;
      mat.ambient.r = 1.0f;
      mat.ambient.g = 1.0f;
      mat.ambient.b = 1.0f;      
      mat.ambient.a = 1.0f;
   }

   __forceinline Material( const D3DCOLORVALUE _diffuse, const D3DCOLORVALUE _ambient, const D3DCOLORVALUE _specular, const D3DCOLORVALUE _emissive, const D3DVALUE _power )
   {
      mat.diffuse = _diffuse;
      mat.ambient = _ambient;
      mat.specular = _specular;
      mat.emissive = _emissive;
      mat.power = _power;
   }

   __forceinline D3DCOLORVALUE getDiffuse() const
   {
      return mat.diffuse;
   }
   __forceinline D3DCOLORVALUE getAmbient() const
   {
      return mat.ambient;
   }
   __forceinline D3DCOLORVALUE getSpecular() const
   {
      return mat.specular;
   }
   __forceinline D3DCOLORVALUE getEmissive() const
   {
      return mat.emissive;
   }
   __forceinline D3DVALUE getPower() const
   {
      return mat.power;
   }

   __forceinline void setDiffuse( const D3DCOLORVALUE _diffuse )
   {
      mat.diffuse = _diffuse;
   }
   __forceinline void setDiffuse( const D3DVALUE a, const D3DVALUE r, const D3DVALUE g, const D3DVALUE b )
   {
      mat.diffuse.r = r;
      mat.diffuse.g = g;
      mat.diffuse.b = b;
      mat.diffuse.a = a;
   }
   __forceinline void setDiffuse( const D3DVALUE a, const COLORREF _color )
   {
      mat.diffuse.r = (float)(_color & 255) * (float)(1.0/255.0);
      mat.diffuse.g = (float)(_color & 65280) * (float)(1.0/65280.0);
      mat.diffuse.b = (float)(_color & 16711680) * (float)(1.0/16711680.0);
	  mat.diffuse.a = a;
   }
   __forceinline void setAmbient( const D3DCOLORVALUE _ambient )
   {
      mat.ambient = _ambient;
   }
   __forceinline void setAmbient( const D3DVALUE a, const D3DVALUE r, const D3DVALUE g, const D3DVALUE b )
   {
      mat.ambient.r = r;
      mat.ambient.g = g;
      mat.ambient.b = b;
	  mat.ambient.a = a;
   }
   __forceinline void setAmbient( const D3DVALUE a, const COLORREF _color )
   {
      mat.ambient.r = (float)(_color & 255) * (float)(1.0/255.0);
      mat.ambient.g = (float)(_color & 65280) * (float)(1.0/65280.0);
      mat.ambient.b = (float)(_color & 16711680) * (float)(1.0/16711680.0);
      mat.ambient.a = a;
   }
   __forceinline void setEmissive( const D3DCOLORVALUE _emissive )
   {
      mat.emissive = _emissive;
   }
   __forceinline void setEmissive( const D3DVALUE a, const D3DVALUE r, const D3DVALUE g, const D3DVALUE b )
   {
      mat.emissive.r = r;
      mat.emissive.g = g;
      mat.emissive.b = b;
      mat.emissive.a = a;
   }
   __forceinline void setSpecular( const D3DCOLORVALUE _specular )
   {
      mat.specular = _specular;
   }
   __forceinline void setSpecular( const D3DVALUE a, const D3DVALUE r, const D3DVALUE g, const D3DVALUE b )
   {
      mat.specular.r = r;
      mat.specular.g = g;
      mat.specular.b = b;
      mat.specular.a = a;
   }
   __forceinline void setPower( const D3DVALUE _power )
   {
      mat.power = _power;
   }
   __forceinline void setColor( const float a, const float r, const float g, const float b )
   {
      mat.specular.r = 0.0f;
      mat.specular.g = 0.0f;
      mat.specular.b = 0.0f;
      mat.specular.a = 0.0f;
      mat.emissive.r = 0.0f;
      mat.emissive.g = 0.0f;
      mat.emissive.b = 0.0f;
      mat.emissive.a = 0.0f;
      mat.power = 0.0f;
      mat.diffuse.r = r;
      mat.diffuse.g = g;
      mat.diffuse.b = b;
      mat.diffuse.a = a;
      mat.ambient.r = r;
      mat.ambient.g = g;
      mat.ambient.b = b;
      mat.ambient.a = a;
   }
   __forceinline void setColor( const float a, const COLORREF _color )
   {
      setColor( a, (float)(_color & 255) * (float)(1.0/255.0),
                   (float)(_color & 65280) * (float)(1.0/65280.0),
                   (float)(_color & 16711680) * (float)(1.0/16711680.0) );
   }
   __forceinline void setBaseMaterial( const BaseMaterial &_base )
   {
      mat = _base;
   }

   void set();

   static void setRenderDevice( RenderDevice *_device );


private:
   BaseMaterial mat;

   static RenderDevice *renderDevice;
};
