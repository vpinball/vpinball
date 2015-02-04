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

class Material
{
public:

	inline Material()
	{
      m_fWrapLighting = 0.0f;
      m_fRoughness = 0.0f;
      m_fEdge = 1.0f;
      m_fOpacity = 1.0f;
      m_cBase = 0xB469FF;
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
