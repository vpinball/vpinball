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
   unsigned char fGlossyImageLerp; // use image also for the glossy layer (0(no tinting at all)..1(use image)), stupid quantization because of legacy loading/saving
   float fEdge; // edge weight/brightness for glossy and clearcoat (0(dark edges)..1(full fresnel))
   bool  bUnused2;
   float fOpacity; // opacity (0..1)
   unsigned char bOpacityActive_fEdgeAlpha; // lowest bit = on/off, upper 7bits = edge weight for fresnel (0(no opacity change)..1(full fresnel)), stupid encoding because of legacy loading/saving
};

struct SavePhysicsMaterial
{
    char szName[32];
    float fElasticity;
    float fElasticityFallOff;
    float fFriction;
    float fScatterAngle;
};

class Material
{
public:

   inline Material()
   {
      m_fWrapLighting = 0.0f;
      m_fRoughness = 0.0f;
      m_fGlossyImageLerp = 1.0f;
      m_fEdge = 1.0f;
      m_fEdgeAlpha = 1.0f;
      m_fOpacity = 1.0f;
      m_cBase = 0xB469FF;
      m_cGlossy = 0;
      m_cClearcoat = 0;
      m_bIsMetal = false;
      m_bOpacityActive = false;

      m_fElasticity = 0.0f;
      m_fElasticityFalloff = 0.0f;
      m_fFriction = 0.0f;
      m_fScatterAngle = 0.0f;

      memset(m_szName, 0, 32);
      strcat_s(m_szName, "dummyMaterial");
   }

   inline unsigned long long hash() const
   {
      union {
         struct {
            unsigned char uc[8];
         };
         unsigned long long ull;
      } h;

      h.uc[0] = ((unsigned int)m_bIsMetal << 6) | ((unsigned int)m_bOpacityActive << 7) | (((size_t)this / sizeof(Material)) & 63); //!! meh
      h.uc[1] = (unsigned char)(clamp(m_fWrapLighting, 0.f, 1.f)*255.0f);
      h.uc[2] = (unsigned char)(clamp(m_fRoughness, 0.f, 1.f)*255.0f);
      h.uc[3] = (((unsigned char)(clamp(m_fEdge, 0.f, 1.f)*255.0f)) >> 4) | (m_bOpacityActive ? ((((unsigned char)(clamp(m_fEdgeAlpha, 0.f, 1.f)*255.0f)) >> 4) << 4) : 0);
      h.uc[4] = ((m_cBase & 255) >> 5) | ((((m_cBase >> 8) & 255) >> 5) << 3) | ((((m_cBase >> 16) & 255) >> 6) << 6);
      h.uc[5] = !m_bIsMetal ? ((m_cGlossy & 255) >> 5) | ((((m_cGlossy >> 8) & 255) >> 5) << 3) | ((((m_cGlossy >> 16) & 255) >> 6) << 6) : 0;
      h.uc[6] = ((m_cClearcoat & 255) >> 5) | ((((m_cClearcoat >> 8) & 255) >> 5) << 3) | ((((m_cClearcoat >> 16) & 255) >> 6) << 6);
      h.uc[7] = m_bOpacityActive ? (unsigned char)(clamp(m_fOpacity, 0.f, 1.f)*255.0f) : 0;

      return h.ull;

      /*return ((unsigned long long)(h.uc[0]&  1)<< 0) | ((unsigned long long)(h.uc[1]&  1)<< 1) | ((unsigned long long)(h.uc[2]&  1)<< 2) | ((unsigned long long)(h.uc[3]&  1)<< 3) | ((unsigned long long)(h.uc[4]&  1)<< 4) | ((unsigned long long)(h.uc[5]&  1)<< 5) | ((unsigned long long)(h.uc[6]&  1)<< 6) | ((unsigned long long)(h.uc[7]&  1)<< 7) |
        ((unsigned long long)(h.uc[0]&  2)<< 8) | ((unsigned long long)(h.uc[1]&  2)<< 9) | ((unsigned long long)(h.uc[2]&  2)<<10) | ((unsigned long long)(h.uc[3]&  2)<<11) | ((unsigned long long)(h.uc[4]&  2)<<12) | ((unsigned long long)(h.uc[5]&  2)<<13) | ((unsigned long long)(h.uc[6]&  2)<<14) | ((unsigned long long)(h.uc[7]&  2)<<15) |
        ((unsigned long long)(h.uc[0]&  4)<<16) | ((unsigned long long)(h.uc[1]&  4)<<17) | ((unsigned long long)(h.uc[2]&  4)<<18) | ((unsigned long long)(h.uc[3]&  4)<<19) | ((unsigned long long)(h.uc[4]&  4)<<20) | ((unsigned long long)(h.uc[5]&  4)<<21) | ((unsigned long long)(h.uc[6]&  4)<<22) | ((unsigned long long)(h.uc[7]&  4)<<23) |
        ((unsigned long long)(h.uc[0]&  8)<<24) | ((unsigned long long)(h.uc[1]&  8)<<25) | ((unsigned long long)(h.uc[2]&  8)<<26) | ((unsigned long long)(h.uc[3]&  8)<<27) | ((unsigned long long)(h.uc[4]&  8)<<28) | ((unsigned long long)(h.uc[5]&  8)<<29) | ((unsigned long long)(h.uc[6]&  8)<<30) | ((unsigned long long)(h.uc[7]&  8)<<31) |
        ((unsigned long long)(h.uc[0]& 16)<<32) | ((unsigned long long)(h.uc[1]& 16)<<33) | ((unsigned long long)(h.uc[2]& 16)<<34) | ((unsigned long long)(h.uc[3]& 16)<<35) | ((unsigned long long)(h.uc[4]& 16)<<36) | ((unsigned long long)(h.uc[5]& 16)<<37) | ((unsigned long long)(h.uc[6]& 16)<<38) | ((unsigned long long)(h.uc[7]& 16)<<39) |
        ((unsigned long long)(h.uc[0]& 32)<<40) | ((unsigned long long)(h.uc[1]& 32)<<41) | ((unsigned long long)(h.uc[2]& 32)<<42) | ((unsigned long long)(h.uc[3]& 32)<<43) | ((unsigned long long)(h.uc[4]& 32)<<44) | ((unsigned long long)(h.uc[5]& 32)<<45) | ((unsigned long long)(h.uc[6]& 32)<<46) | ((unsigned long long)(h.uc[7]& 32)<<47) |
        ((unsigned long long)(h.uc[0]& 64)<<48) | ((unsigned long long)(h.uc[1]& 64)<<49) | ((unsigned long long)(h.uc[2]& 64)<<50) | ((unsigned long long)(h.uc[3]& 64)<<51) | ((unsigned long long)(h.uc[4]& 64)<<52) | ((unsigned long long)(h.uc[5]& 64)<<53) | ((unsigned long long)(h.uc[6]& 64)<<54) | ((unsigned long long)(h.uc[7]& 64)<<55) |
        ((unsigned long long)(h.uc[0]&128)<<56) | ((unsigned long long)(h.uc[1]&128)<<57) | ((unsigned long long)(h.uc[2]&128)<<58) | ((unsigned long long)(h.uc[3]&128)<<59) | ((unsigned long long)(h.uc[4]&128)<<60) | ((unsigned long long)(h.uc[5]&128)<<61) | ((unsigned long long)(h.uc[6]&128)<<62) | ((unsigned long long)(h.uc[7]&128)<<63);*/

      /*const unsigned char* const p = reinterpret_cast<const unsigned char*>( &(this->m_fWrapLighting) );
      unsigned int h = 2166136261;

      for (unsigned int i = 0; i <= (unsigned int)(&(this->m_bOpacityActive))-(unsigned int)(&(this->m_fWrapLighting)); ++i)
      h = (h * 16777619) ^ p[i];

      return (unsigned long long)h;*/
   }

   char m_szName[32];
   float m_fWrapLighting;
   float m_fRoughness;
   float m_fGlossyImageLerp;
   float m_fEdge;
   float m_fEdgeAlpha;
   float m_fOpacity;
   COLORREF m_cBase;
   COLORREF m_cGlossy;
   COLORREF m_cClearcoat;
   bool m_bIsMetal;
   bool m_bOpacityActive;

   //physics
   float m_fElasticity;
   float m_fElasticityFalloff;
   float m_fFriction;
   float m_fScatterAngle;
};
