#pragma once

// only used for backward compatibility loading and saving (VPX version < 10.8)
struct SaveMaterial //!! could still squeeze some stuff in here, due to implicit padding after the bool & unsigned chars!
{
   SaveMaterial() { memset(this,0,sizeof(SaveMaterial)); }

   char szName[MAXNAMEBUFFER];
   COLORREF cBase; // can be overridden by texture on object itself
   COLORREF cGlossy; // specular of glossy layer
   COLORREF cClearcoat; // specular of clearcoat layer
   float fWrapLighting; // wrap/rim lighting factor (0(off)..1(full))
   bool  bIsMetal; // is a metal material or not
   float fRoughness; // roughness of glossy layer (0(diffuse)..1(specular))
   unsigned char fGlossyImageLerp; // use image also for the glossy layer (0(no tinting at all)..1(use image)), stupid quantization because of legacy loading/saving
   float fEdge; // edge weight/brightness for glossy and clearcoat (0(dark edges)..1(full fresnel))
   unsigned char fThickness; // thickness for transparent materials (0(paper thin)..1(maximum)), stupid quantization because of legacy loading/saving
   float fOpacity; // opacity (0..1)
   unsigned char bOpacityActive_fEdgeAlpha; // lowest bit = on/off, upper 7bits = edge weight for fresnel (0(no opacity change)..1(full fresnel)), stupid encoding because of legacy loading/saving
};

struct SavePhysicsMaterial
{
    SavePhysicsMaterial() { memset(this,0,sizeof(SavePhysicsMaterial)); }

    char szName[MAXNAMEBUFFER];
    float fElasticity;
    float fElasticityFallOff;
    float fFriction;
    float fScatterAngle;
};

#define MATERIAL_VERSION 1 // for im/export

class Material final : ILoadable
{
public:

   enum MaterialType
   {
      BASIC,
      METAL,
   };

   inline Material() :
         m_type(MaterialType::BASIC)
       , m_fWrapLighting(0.0f)
       , m_fRoughness(0.0f)
       , m_fGlossyImageLerp(1.0f)
       , m_fThickness(0.05f)
       , m_fEdge(1.0f)
       , m_fEdgeAlpha(1.0f)
       , m_fOpacity(1.0f)
       , m_cBase(0xB469FF)
       , m_cGlossy(0)
       , m_cClearcoat(0)
       , m_bOpacityActive(false)
       , m_fElasticity(0.0f)
       , m_fElasticityFalloff(0.0f)
       , m_fFriction(0.0f)
       , m_fScatterAngle(0.0f)
       , m_cRefractionTint(0xFFFFFF)
   {
      m_szName = "dummyMaterial";
   }

   inline Material(MaterialType type, float wrapLighting, float roughness, float glossyImageLerp, float thickness, float edge, float edgeAlpha, float opacity,
       COLORREF base, COLORREF glossy, COLORREF clearcoat, bool opacityActive,
       float elasticity, float elasticityFalloff, float friction, float scatterAngle, COLORREF refractionTint) :
         m_type(type)
       , m_fWrapLighting(wrapLighting)
       , m_fRoughness(roughness)
       , m_fGlossyImageLerp(glossyImageLerp)
       , m_fThickness(thickness)
       , m_fEdge(edge)
       , m_fEdgeAlpha(edgeAlpha)
       , m_fOpacity(opacity)
       , m_cBase(base)
       , m_cGlossy(glossy)
       , m_cClearcoat(clearcoat)
       , m_bOpacityActive(opacityActive)
       , m_fElasticity(elasticity)
       , m_fElasticityFalloff(elasticityFalloff)
       , m_fFriction(friction)
       , m_fScatterAngle(scatterAngle)
       , m_cRefractionTint(refractionTint)
   {
      m_szName = "dummyMaterial";
   }

   inline Material(const Material * const pmat) :
         m_type(pmat->m_type)
       , m_fWrapLighting(pmat->m_fWrapLighting)
       , m_fRoughness(pmat->m_fRoughness)
       , m_fGlossyImageLerp(pmat->m_fGlossyImageLerp)
       , m_fThickness(pmat->m_fThickness)
       , m_fEdge(pmat->m_fEdge)
       , m_fEdgeAlpha(pmat->m_fEdgeAlpha)
       , m_fOpacity(pmat->m_fOpacity)
       , m_cBase(pmat->m_cBase)
       , m_cGlossy(pmat->m_cGlossy)
       , m_cClearcoat(pmat->m_cClearcoat)
       , m_bOpacityActive(pmat->m_bOpacityActive)
       , m_fElasticity(pmat->m_fElasticity)
       , m_fElasticityFalloff(pmat->m_fElasticityFalloff)
       , m_fFriction(pmat->m_fFriction)
       , m_fScatterAngle(pmat->m_fScatterAngle)
       , m_cRefractionTint(pmat->m_cRefractionTint)
   {
      m_szName = pmat->m_szName;
   }

   inline unsigned long long hash() const
   {
      union {
         struct {
            unsigned char uc[8];
         };
         unsigned long long ull;
      } h;
      h.uc[0] = ((unsigned int)m_type << 6) | ((unsigned int)m_bOpacityActive << 8) | (const unsigned int)(((size_t)this / sizeof(Material)) & 63); //!! meh
      h.uc[1] = (unsigned char)(clamp(m_fWrapLighting, 0.f, 1.f)*255.0f);
      h.uc[2] = (unsigned char)(clamp(m_fRoughness, 0.f, 1.f)*255.0f);
      h.uc[3] = (((unsigned char)(clamp(m_fEdge, 0.f, 1.f)*255.0f)) >> 4) | (m_bOpacityActive ? ((((unsigned char)(clamp(m_fEdgeAlpha, 0.f, 1.f)*255.0f)) >> 4) << 4) : 0);
      h.uc[4] = ((m_cBase & 255) >> 5) | ((((m_cBase >> 8) & 255) >> 5) << 3) | ((((m_cBase >> 16) & 255) >> 6) << 6);
      h.uc[5] = m_type == MaterialType::BASIC ? ((m_cGlossy & 255) >> 5) | ((((m_cGlossy >> 8) & 255) >> 5) << 3) | ((((m_cGlossy >> 16) & 255) >> 6) << 6) : 0;
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

   string m_szName;
   
   // shading properties
   MaterialType m_type;
   float m_fWrapLighting;
   float m_fRoughness;
   float m_fGlossyImageLerp;
   float m_fThickness;
   float m_fEdge;
   float m_fEdgeAlpha;
   float m_fOpacity;
   COLORREF m_cBase;
   COLORREF m_cGlossy;
   COLORREF m_cClearcoat;
   bool m_bOpacityActive;

   // physic properties
   float m_fElasticity;
   float m_fElasticityFalloff;
   float m_fFriction;
   float m_fScatterAngle;
   COLORREF m_cRefractionTint; // 10.8+ only
   
   size_t GetSaveSize() const
   {
      size_t size = 0;
      size += 2 * sizeof(int) + sizeof(int); // TYPE
      size += 2 * sizeof(int) + sizeof(int) + m_szName.length(); // NAME
      size += 2 * sizeof(int) + sizeof(float); // WLIG
      size += 2 * sizeof(int) + sizeof(float); // ROUG
      size += 2 * sizeof(int) + sizeof(float); // GIML
      size += 2 * sizeof(int) + sizeof(float); // THCK
      size += 2 * sizeof(int) + sizeof(float); // EDGE
      size += 2 * sizeof(int) + sizeof(float); // EALP
      size += 2 * sizeof(int) + sizeof(float); // OPAC
      size += 2 * sizeof(int) + sizeof(int); // BASE
      size += 2 * sizeof(int) + sizeof(int); // GLOS
      size += 2 * sizeof(int) + sizeof(int); // COAT
      size += 2 * sizeof(int) + sizeof(int); // RTNT
      size += 2 * sizeof(int) + sizeof(int); // EOPA
      size += 2 * sizeof(int) + sizeof(float); // ELAS
      size += 2 * sizeof(int) + sizeof(float); // ELFO
      size += 2 * sizeof(int) + sizeof(float); // FRIC
      size += 2 * sizeof(int) + sizeof(float); // SCAT
      size += 2 * sizeof(int); // ENDB
      return size;
   }

   HRESULT SaveData(IStream* pstm, HCRYPTHASH hcrypthash, const bool saveForUndo)
   {
      BiffWriter bw(pstm, hcrypthash);
      bw.WriteInt(FID(TYPE), m_type);
      bw.WriteString(FID(NAME), m_szName);
      bw.WriteFloat(FID(WLIG), m_fWrapLighting);
      bw.WriteFloat(FID(ROUG), m_fRoughness);
      bw.WriteFloat(FID(GIML), m_fGlossyImageLerp);
      bw.WriteFloat(FID(THCK), m_fThickness);
      bw.WriteFloat(FID(EDGE), m_fEdge);
      bw.WriteFloat(FID(EALP), m_fEdgeAlpha);
      bw.WriteFloat(FID(OPAC), m_fOpacity);
      bw.WriteInt(FID(BASE), m_cBase);
      bw.WriteInt(FID(GLOS), m_cGlossy);
      bw.WriteInt(FID(COAT), m_cClearcoat);
      bw.WriteInt(FID(RTNT), m_cRefractionTint);
      bw.WriteBool(FID(EOPA), m_bOpacityActive);
      bw.WriteFloat(FID(ELAS), m_fElasticity);
      bw.WriteFloat(FID(ELFO), m_fElasticityFalloff);
      bw.WriteFloat(FID(FRIC), m_fFriction);
      bw.WriteFloat(FID(SCAT), m_fScatterAngle);
      bw.WriteTag(FID(ENDB));
      return S_OK;
   }

   HRESULT LoadData(IStream* pstm, PinTable* ppt, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
   {
      BiffReader br(pstm, this, ppt, version, hcrypthash, hcryptkey);
      br.Load();
      return S_OK;
   }

   bool LoadToken(const int id, BiffReader* const pbr) override
   {
      switch (id)
      {
      case FID(TYPE): pbr->GetInt(&m_type); break;
      case FID(NAME): pbr->GetString(m_szName); break;
      case FID(WLIG): pbr->GetFloat(m_fWrapLighting); break;
      case FID(ROUG): pbr->GetFloat(m_fRoughness); break;
      case FID(GIML): pbr->GetFloat(m_fGlossyImageLerp); break;
      case FID(THCK): pbr->GetFloat(m_fThickness); break;
      case FID(EDGE): pbr->GetFloat(m_fEdge); break;
      case FID(EALP): pbr->GetFloat(m_fEdgeAlpha); break;
      case FID(OPAC): pbr->GetFloat(m_fOpacity); break;
      case FID(BASE): pbr->GetInt(m_cBase); break;
      case FID(GLOS): pbr->GetInt(m_cGlossy); break;
      case FID(COAT): pbr->GetInt(m_cClearcoat); break;
      case FID(RTNT): pbr->GetInt(m_cRefractionTint); break;
      case FID(EOPA): pbr->GetBool(m_bOpacityActive); break;
      case FID(ELAS): pbr->GetFloat(m_fElasticity); break;
      case FID(ELFO): pbr->GetFloat(m_fElasticityFalloff); break;
      case FID(FRIC): pbr->GetFloat(m_fFriction); break;
      case FID(SCAT): pbr->GetFloat(m_fScatterAngle); break;
      }
      return true;
   }
};
