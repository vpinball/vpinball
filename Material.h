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

   Material()
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

   Material(  D3DCOLORVALUE _diffuse,  D3DCOLORVALUE _ambient,  D3DCOLORVALUE _specular,  D3DCOLORVALUE _emissive,  D3DVALUE _power )
   {
      mat.diffuse = _diffuse;
      mat.ambient = _ambient;
      mat.specular = _specular;
      mat.emissive = _emissive;
      mat.power = _power;
   }

   D3DCOLORVALUE getDiffuse() 
   {
      return mat.diffuse;
   }
   D3DCOLORVALUE getAmbient() 
   {
      return mat.ambient;
   }
   D3DCOLORVALUE getSpecular() 
   {
      return mat.specular;
   }
   D3DCOLORVALUE getEmissive() 
   {
      return mat.emissive;
   }
   D3DVALUE getPower() 
   {
      return mat.power;
   }

   void setDiffuse(  D3DCOLORVALUE _diffuse )
   {
      mat.diffuse = _diffuse;
   }
   void setDiffuse(  D3DVALUE a,  D3DVALUE r,  D3DVALUE g,  D3DVALUE b )
   {
      mat.diffuse.r = r;
      mat.diffuse.g = g;
      mat.diffuse.b = b;
      mat.diffuse.a = a;
   }
   void setDiffuse(  D3DVALUE a,  COLORREF _color )
   {
      mat.diffuse.r = (float)(_color & 255) * (float)(1.0/255.0);
      mat.diffuse.g = (float)(_color & 65280) * (float)(1.0/65280.0);
      mat.diffuse.b = (float)(_color & 16711680) * (float)(1.0/16711680.0);
	  mat.diffuse.a = a;
   }
   void setAmbient(  D3DCOLORVALUE _ambient )
   {
      mat.ambient = _ambient;
   }
   void setAmbient(  D3DVALUE a,  D3DVALUE r,  D3DVALUE g,  D3DVALUE b )
   {
      mat.ambient.r = r;
      mat.ambient.g = g;
      mat.ambient.b = b;
	  mat.ambient.a = a;
   }
   void setAmbient(  D3DVALUE a,  COLORREF _color )
   {
      mat.ambient.r = (float)(_color & 255) * (float)(1.0/255.0);
      mat.ambient.g = (float)(_color & 65280) * (float)(1.0/65280.0);
      mat.ambient.b = (float)(_color & 16711680) * (float)(1.0/16711680.0);
      mat.ambient.a = a;
   }
   void setEmissive(  D3DCOLORVALUE _emissive )
   {
      mat.emissive = _emissive;
   }
   void setEmissive(  D3DVALUE a,  D3DVALUE r,  D3DVALUE g,  D3DVALUE b )
   {
      mat.emissive.r = r;
      mat.emissive.g = g;
      mat.emissive.b = b;
      mat.emissive.a = a;
   }
   void setSpecular(  D3DCOLORVALUE _specular )
   {
      mat.specular = _specular;
   }
   void setSpecular(  D3DVALUE a,  D3DVALUE r,  D3DVALUE g,  D3DVALUE b )
   {
      mat.specular.r = r;
      mat.specular.g = g;
      mat.specular.b = b;
      mat.specular.a = a;
   }
   void setPower(  D3DVALUE _power )
   {
      mat.power = _power;
   }
   void setColor(  float a,  float r,  float g,  float b )
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
   void setColor(  float a,  COLORREF _color )
   {
      setColor( a, (float)(_color & 255) * (float)(1.0/255.0),
                   (float)(_color & 65280) * (float)(1.0/65280.0),
                   (float)(_color & 16711680) * (float)(1.0/16711680.0) );
   }
   void setBaseMaterial(  BaseMaterial &_base )
   {
      mat = _base;
   }

   void set();

   static void setRenderDevice( RenderDevice *_device );


private:
   BaseMaterial mat;

   static RenderDevice *renderDevice;
};
