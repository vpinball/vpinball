#pragma once

// only used for loading and saving
struct SaveMaterial
{
   char szName[32];
   COLORREF cBase; // can be overriden by texture on object itself
   COLORREF cGlossy; // specular of glossy layer
   COLORREF cClearcoat; // specular of clearcoat layer
   float fWrapLighting; // wrap/rim lighting factor (0(off)..1(full))
   bool  bIsMetal; // is a metal material or not
   float fRoughness; // roughness of glossy layer (0(diffuse)..1(specular))
   bool  bUnused1;
   float fEdge; // edge weight/brightness for glossy and clearcoat (0(dark edges)..1(full fresnel))
   bool  bUnused2;
   float fOpacity; // opacity (0..1)
   bool  bOpacityActive;
};

inline D3DXVECTOR4 convertColor(const COLORREF c)
{
	const float r = (float)(c & 255) * (float)(1.0/255.0);
	const float g = (float)(c & 65280) * (float)(1.0/65280.0);
	const float b = (float)(c & 16711680) * (float)(1.0/16711680.0);
	D3DXVECTOR4 d(r,g,b,1.0f);
	return d;
}

inline COLORREF convertColor(const D3DXVECTOR4& d)
{
	return ((unsigned int)(d.x*255.0f)) | (((unsigned int)(d.y*255.0f))<<8) | (((unsigned int)(d.z*255.0f))<<16);
}

class Material
{
public:

	inline Material()
	{
      m_fWrapLighting = 0.5f;
      m_fRoughness = 0.0f;
      m_fEdge = 1.0f;
      m_fOpacity = 1.0f;
      m_cBase = 0;
      m_cGlossy = 0;
      m_cClearcoat = 0;
      m_bIsMetal = false;
      m_bOpacityActive = false;

      memset( m_szName, 0, 32 );
      strcat_s( m_szName, "dummyMaterial");
	}

	char m_szName[32];
	float m_fWrapLighting;
	float m_fRoughness;
	float m_fEdge;
	float m_fOpacity;
	COLORREF m_cBase;
	COLORREF m_cGlossy;
	COLORREF m_cClearcoat;
	bool m_bIsMetal;
	bool m_bOpacityActive;
};
