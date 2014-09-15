#pragma once

//!! remove as soon as all is changed to shader based material
struct BaseMaterial
{
   union {
      D3DCOLORVALUE   diffuse;        /* Diffuse color RGBA */
      __m128          d;
   };
   union {
      D3DCOLORVALUE   ambient;        /* Ambient color RGB */
      __m128          a;
   };
   union {
      D3DCOLORVALUE   specular;       /* Specular 'shininess' */
      __m128          s;
   };
   union {
      D3DCOLORVALUE   emissive;       /* Emissive color RGB */
      __m128          e;
   };
   union {
      D3DVALUE        power;          /* Sharpness if specular highlight */
      __m128          p;
   };
};

// only used for loading and saving
struct SaveMaterial
{
   char szName[32];
   COLORREF diffuseColor;
   COLORREF glossyColor;
   COLORREF specularColor;
   float fDiffuse;
   bool  bDiffuseActive;
   float fGlossy;
   bool  bGlossyActive;
   float fSpecular;
   bool  bSpecularActive;
};

class RenderDevice;

class Material
{
public:

   inline Material()
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

      m_fDiffuse=0.8f;
      m_fGlossy=0.0f;
      m_fSpecular=0.0f;
      m_diffuseColor=0xFFFFFFFF;
   }

   inline Material( const D3DCOLORVALUE _diffuse, const D3DCOLORVALUE _ambient, const D3DCOLORVALUE _specular, const D3DCOLORVALUE _emissive, const D3DVALUE _power )
   {
      mat.diffuse = _diffuse;
      mat.ambient = _ambient;
      mat.specular = _specular;
      mat.emissive = _emissive;
      mat.power = _power;
   }

   inline D3DCOLORVALUE getDiffuse() const
   {
      return mat.diffuse;
   }
   inline D3DCOLORVALUE getAmbient() const
   {
      return mat.ambient;
   }
   inline D3DCOLORVALUE getSpecular() const
   {
      return mat.specular;
   }
   inline D3DCOLORVALUE getEmissive() const
   {
      return mat.emissive;
   }
   inline D3DVALUE getPower() const
   {
      return mat.power;
   }

   inline void setDiffuse( const D3DCOLORVALUE &_diffuse )
   {
      mat.diffuse = _diffuse;
   }
   inline void setDiffuse( const D3DVALUE a, const D3DVALUE r, const D3DVALUE g, const D3DVALUE b )
   {
      mat.diffuse.r = r;
      mat.diffuse.g = g;
      mat.diffuse.b = b;
      mat.diffuse.a = a;
   }
   inline void setDiffuse( const D3DVALUE a, const COLORREF _color )
   {
      mat.diffuse.r = (float)(_color & 255) * (float)(1.0/255.0);
      mat.diffuse.g = (float)(_color & 65280) * (float)(1.0/65280.0);
      mat.diffuse.b = (float)(_color & 16711680) * (float)(1.0/16711680.0);
	  mat.diffuse.a = a;
   }
   inline void setAmbient( const D3DCOLORVALUE &_ambient )
   {
      mat.ambient = _ambient;
   }
   inline void setAmbient( const D3DVALUE a, const D3DVALUE r, const D3DVALUE g, const D3DVALUE b )
   {
      mat.ambient.r = r;
      mat.ambient.g = g;
      mat.ambient.b = b;
	  mat.ambient.a = a;
   }
   inline void setAmbient( const D3DVALUE a, const COLORREF _color )
   {
      mat.ambient.r = (float)(_color & 255) * (float)(1.0/255.0);
      mat.ambient.g = (float)(_color & 65280) * (float)(1.0/65280.0);
      mat.ambient.b = (float)(_color & 16711680) * (float)(1.0/16711680.0);
      mat.ambient.a = a;
   }
   inline void setEmissive( const D3DCOLORVALUE &_emissive )
   {
      mat.emissive = _emissive;
   }
   inline void setEmissive( const D3DVALUE a, const D3DVALUE r, const D3DVALUE g, const D3DVALUE b )
   {
      mat.emissive.r = r;
      mat.emissive.g = g;
      mat.emissive.b = b;
      mat.emissive.a = a;
   }
   inline void setSpecular( const D3DCOLORVALUE &_specular )
   {
      mat.specular = _specular;
   }
   inline void setSpecular( const D3DVALUE a, const D3DVALUE r, const D3DVALUE g, const D3DVALUE b )
   {
      mat.specular.r = r;
      mat.specular.g = g;
      mat.specular.b = b;
      mat.specular.a = a;
   }
   inline void setPower( const D3DVALUE _power )
   {
      mat.power = _power;
   }
   inline void setColor( const float a, const float r, const float g, const float b ) //!! meh
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
   inline void setColor( const float a, const COLORREF _color ) //!! meh
   {
      setColor( a, (float)(_color & 255) * (float)(1.0/255.0),
                   (float)(_color & 65280) * (float)(1.0/65280.0),
                   (float)(_color & 16711680) * (float)(1.0/16711680.0) );
   }
   inline void setBaseMaterial( const BaseMaterial &_base )
   {
      mat = _base;
   }
   const BaseMaterial& getBaseMaterial() const
   {
       return mat;
   }
   inline void setDiffuse( float fValue ) 
   {
       m_fDiffuse = fValue;
   }
   inline void setGlossy( float fValue )
   {
       m_fGlossy = fValue;
   }
   inline void setSpecular( float fValue )
   {
       m_fSpecular = fValue;
   }

   inline D3DXVECTOR4 getDiffuseColor()
   {
       const float r = (float)(m_diffuseColor & 255) * (float)(1.0/255.0);
       const float g = (float)(m_diffuseColor & 65280) * (float)(1.0/65280.0);
       const float b = (float)(m_diffuseColor & 16711680) * (float)(1.0/16711680.0);
       D3DXVECTOR4 matColor(r,g,b,1.0f);   
       return matColor; 
   }
   inline D3DXVECTOR4 getGlossyColor()
   {
       const float r = (float)(m_glossyColor & 255) * (float)(1.0/255.0);
       const float g = (float)(m_glossyColor & 65280) * (float)(1.0/65280.0);
       const float b = (float)(m_glossyColor & 16711680) * (float)(1.0/16711680.0);
       D3DXVECTOR4 matColor(r,g,b,1.0f);   
       return matColor; 
   }
   inline D3DXVECTOR4 getSpecularColor()
   {
       const float r = (float)(m_specularColor & 255) * (float)(1.0/255.0);
       const float g = (float)(m_specularColor & 65280) * (float)(1.0/65280.0);
       const float b = (float)(m_specularColor & 16711680) * (float)(1.0/16711680.0);
       D3DXVECTOR4 matColor(r,g,b,1.0f);   
       return matColor; 
   }

   union {
	struct{
	char m_szName[32];
	float m_fDiffuse;
	float m_fGlossy;
	float m_fSpecular;
   bool m_bDiffuseActive;
   bool m_bGlossyActive;
   bool m_bSpecularActive;
   bool m_reserved;
	COLORREF m_diffuseColor;
	COLORREF m_glossyColor;
	COLORREF m_specularColor;
	};
	struct{
	__m128 i0,i1,i2,i3; //!! remove as soon as BaseMaterial vanishes
	};
   };
private:
   BaseMaterial mat;
};
