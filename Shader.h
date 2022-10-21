#pragma once

#ifdef _DEBUG
//Writes all compile/parse errors/warnings to a file. (0=never, 1=only errors, 2=warnings, 3=info)
#define DEBUG_LEVEL_LOG 1
//Writes all shaders that are compiled to separate files (e.g. ShaderName_Technique_Pass.vs and .fs) (0=never, 1=only if compile failed, 2=always)
#define WRITE_SHADER_FILES 1
#else 
#define DEBUG_LEVEL_LOG 0
#define WRITE_SHADER_FILES 1
#endif

// Attempt to speed up STL which is very CPU costly, maybe we should look into using EASTL instead? http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2271.html https://github.com/electronicarts/EASTL
#define _SECURE_SCL 0
#define _HAS_ITERATOR_DEBUGGING 0

#include <string>

// Declaration of all available techniques (shader program)
// When changed, this list must also be copied unchanged to Shader.cpp (for its implementation)
#define SHADER_TECHNIQUE(name) SHADER_TECHNIQUE_##name
enum ShaderTechniques
{
   SHADER_TECHNIQUE(RenderBall),
   SHADER_TECHNIQUE(RenderBall_DecalMode),
   SHADER_TECHNIQUE(RenderBall_CabMode),
   SHADER_TECHNIQUE(RenderBall_CabMode_DecalMode),
   SHADER_TECHNIQUE(RenderBallTrail),
   SHADER_TECHNIQUE(basic_without_texture),
   SHADER_TECHNIQUE(basic_with_texture),
   SHADER_TECHNIQUE(basic_with_texture_normal),
   SHADER_TECHNIQUE(basic_without_texture_isMetal),
   SHADER_TECHNIQUE(basic_with_texture_isMetal),
   SHADER_TECHNIQUE(basic_with_texture_normal_isMetal),
   SHADER_TECHNIQUE(playfield_without_texture),
   SHADER_TECHNIQUE(playfield_with_texture),
   SHADER_TECHNIQUE(playfield_with_texture_normal),
   SHADER_TECHNIQUE(playfield_without_texture_isMetal),
   SHADER_TECHNIQUE(playfield_with_texture_isMetal),
   SHADER_TECHNIQUE(playfield_with_texture_normal_isMetal),
   SHADER_TECHNIQUE(playfield_refl_without_texture),
   SHADER_TECHNIQUE(playfield_refl_with_texture),
   SHADER_TECHNIQUE(basic_depth_only_without_texture),
   SHADER_TECHNIQUE(basic_depth_only_with_texture),
   SHADER_TECHNIQUE(bg_decal_without_texture),
   SHADER_TECHNIQUE(bg_decal_with_texture),
   SHADER_TECHNIQUE(kickerBoolean),
   SHADER_TECHNIQUE(kickerBoolean_isMetal),
   SHADER_TECHNIQUE(light_with_texture),
   SHADER_TECHNIQUE(light_with_texture_isMetal),
   SHADER_TECHNIQUE(light_without_texture),
   SHADER_TECHNIQUE(light_without_texture_isMetal),
   SHADER_TECHNIQUE(basic_DMD),
   SHADER_TECHNIQUE(basic_DMD_ext),
   SHADER_TECHNIQUE(basic_DMD_world),
   SHADER_TECHNIQUE(basic_DMD_world_ext),
   SHADER_TECHNIQUE(basic_noDMD),
   SHADER_TECHNIQUE(basic_noDMD_world),
   SHADER_TECHNIQUE(basic_noDMD_notex),
   SHADER_TECHNIQUE(AO),
   SHADER_TECHNIQUE(NFAA),
   SHADER_TECHNIQUE(DLAA_edge),
   SHADER_TECHNIQUE(DLAA),
   SHADER_TECHNIQUE(FXAA1),
   SHADER_TECHNIQUE(FXAA2),
   SHADER_TECHNIQUE(FXAA3),
   SHADER_TECHNIQUE(fb_tonemap),
   SHADER_TECHNIQUE(fb_bloom),
   SHADER_TECHNIQUE(fb_AO),
   SHADER_TECHNIQUE(fb_tonemap_AO),
   SHADER_TECHNIQUE(fb_tonemap_AO_static),
   SHADER_TECHNIQUE(fb_tonemap_no_filterRGB),
   SHADER_TECHNIQUE(fb_tonemap_no_filterRG),
   SHADER_TECHNIQUE(fb_tonemap_no_filterR),
   SHADER_TECHNIQUE(fb_tonemap_AO_no_filter),
   SHADER_TECHNIQUE(fb_tonemap_AO_no_filter_static),
   SHADER_TECHNIQUE(fb_bloom_horiz9x9),
   SHADER_TECHNIQUE(fb_bloom_vert9x9),
   SHADER_TECHNIQUE(fb_bloom_horiz19x19),
   SHADER_TECHNIQUE(fb_bloom_vert19x19),
   SHADER_TECHNIQUE(fb_bloom_horiz19x19h),
   SHADER_TECHNIQUE(fb_bloom_vert19x19h),
   SHADER_TECHNIQUE(fb_bloom_horiz39x39),
   SHADER_TECHNIQUE(fb_bloom_vert39x39),
   SHADER_TECHNIQUE(fb_mirror),
   SHADER_TECHNIQUE(CAS),
   SHADER_TECHNIQUE(BilateralSharp_CAS),
   SHADER_TECHNIQUE(SSReflection),
   SHADER_TECHNIQUE(basic_noLight),
   SHADER_TECHNIQUE(bulb_light),
   SHADER_TECHNIQUE(SMAA_ColorEdgeDetection),
   SHADER_TECHNIQUE(SMAA_BlendWeightCalculation),
   SHADER_TECHNIQUE(SMAA_NeighborhoodBlending),
   SHADER_TECHNIQUE(stereo),
   SHADER_TECHNIQUE(stereo_Int),
   SHADER_TECHNIQUE(stereo_Flipped_Int),
   SHADER_TECHNIQUE(stereo_anaglyph),
   SHADER_TECHNIQUE(stereo_AMD_DEBUG),
   SHADER_TECHNIQUE_COUNT,
   SHADER_TECHNIQUE_INVALID
};
#undef SHADER_TECHNIQUE

enum ShaderUniformType
{
   SUT_Bool,
   SUT_Int,
   SUT_Float,
   SUT_Float2,
   SUT_Float3,
   SUT_Float4,
   SUT_Float4v,
   SUT_Float3x4,
   SUT_Float4x3,
   SUT_Float4x4,
   SUT_DataBlock,
   SUT_Sampler,
   SUT_INVALID
};

// Declaration of all uniforms and samplers used in the shaders
// When changed, this list must also be copied unchanged to Shader.cpp (for its implementation)
// Samplers defines how to sample a texture. For DX9, they are defined in the effect files, only the texture reference is set through the API.
// Otherwise, the sampler states can be directly overriden through DX9Device->SetSamplerState (per tex unit), being carefull that the effect
// framework will also apply the ones defined in the effect file during Technique->Begin call (so either don't define them, or reapply).
#define SHADER_UNIFORM(type, name) SHADER_##name
#define SHADER_SAMPLER(name, legacy_name, texture_ref, default_tex_unit, default_clampu, default_clampv, default_filter) SHADER_##name
enum ShaderUniforms
{
   // -- Matrices --
   SHADER_UNIFORM(SUT_DataBlock, matrixBlock), // OpenGL only, matrices as a float block
   SHADER_UNIFORM(SUT_Float4x4, matWorldViewProj), // DX9 only
   SHADER_UNIFORM(SUT_Float4x4, matWorldView), // DX9 only
   SHADER_UNIFORM(SUT_Float4x3, matWorldViewInverse), // DX9 only
   SHADER_UNIFORM(SUT_Float3x4, matWorldViewInverseTranspose), // DX9 only
   SHADER_UNIFORM(SUT_Float4x3, matView), // DX9 only
   SHADER_UNIFORM(SUT_Float4x3, orientation),
   // -- Floats --
   SHADER_UNIFORM(SUT_Float, RenderBall),
   SHADER_UNIFORM(SUT_Float, blend_modulate_vs_add),
   SHADER_UNIFORM(SUT_Float, alphaTestValue),
   SHADER_UNIFORM(SUT_Float, eye),
   SHADER_UNIFORM(SUT_Float, fKickerScale),
   SHADER_UNIFORM(SUT_Float, fSceneScale),
   SHADER_UNIFORM(SUT_Float, mirrorFactor),
   // -- Vectors and Float Arrays --
   SHADER_UNIFORM(SUT_Float4, Roughness_WrapL_Edge_Thickness),
   SHADER_UNIFORM(SUT_Float4, cBase_Alpha),
   SHADER_UNIFORM(SUT_Float4, lightCenter_maxRange),
   SHADER_UNIFORM(SUT_Float4, lightColor2_falloff_power),
   SHADER_UNIFORM(SUT_Float4, lightColor_intensity),
   SHADER_UNIFORM(SUT_Float2, fenvEmissionScale_TexWidth),
   SHADER_UNIFORM(SUT_Float4, invTableRes_playfield_height_reflection),
   SHADER_UNIFORM(SUT_Float4, cAmbient_LightRange),
   SHADER_UNIFORM(SUT_Float4, cClearcoat_EdgeAlpha),
   SHADER_UNIFORM(SUT_Float4, cGlossy_ImageLerp),
   SHADER_UNIFORM(SUT_Float2, fDisableLighting_top_below),
   SHADER_UNIFORM(SUT_Float4, backBoxSize),
   SHADER_UNIFORM(SUT_Float4, vColor_Intensity),
   SHADER_UNIFORM(SUT_Float4, w_h_height),
   SHADER_UNIFORM(SUT_Float4, alphaTestValueAB_filterMode_addBlend),
   SHADER_UNIFORM(SUT_Float3, amount_blend_modulate_vs_add_flasherMode),
   SHADER_UNIFORM(SUT_Float4, staticColor_Alpha),
   SHADER_UNIFORM(SUT_Float4, ms_zpd_ya_td),
   SHADER_UNIFORM(SUT_Float2, Anaglyph_DeSaturation_Contrast),
   SHADER_UNIFORM(SUT_Float4, vRes_Alpha_time),
   SHADER_UNIFORM(SUT_Float4, SSR_bumpHeight_fresnelRefl_scale_FS),
   SHADER_UNIFORM(SUT_Float2, AO_scale_timeblur),
   SHADER_UNIFORM(SUT_Float4, cWidth_Height_MirrorAmount),
   SHADER_UNIFORM(SUT_Float4v, clip_planes), // OpenGL only
   SHADER_UNIFORM(SUT_Float4v, lightEmission),
   SHADER_UNIFORM(SUT_Float4v, lightPos),
   SHADER_UNIFORM(SUT_Float4v, packedLights), // DX9 only
   // -- Integer and Bool --
   SHADER_UNIFORM(SUT_Bool, ignoreStereo),
   SHADER_UNIFORM(SUT_Bool, disableLighting),
   SHADER_UNIFORM(SUT_Int, lightSources),
   SHADER_UNIFORM(SUT_Bool, doNormalMapping),
   SHADER_UNIFORM(SUT_Bool, is_metal),
   SHADER_UNIFORM(SUT_Bool, color_grade),
   SHADER_UNIFORM(SUT_Bool, do_bloom),
   SHADER_UNIFORM(SUT_Bool, objectSpaceNormalMap),
   SHADER_UNIFORM(SUT_Bool, do_dither),
   SHADER_UNIFORM(SUT_Bool, lightingOff), // DX9 only
   SHADER_UNIFORM(SUT_Float2, imageBackglassMode), // OpenGL only
   // -- Samplers (a texture reference with sampling configuration) --
   // DMD shader
   SHADER_SAMPLER(tex_dmd, texSampler0, Texture0, 0, SA_CLAMP, SA_CLAMP, SF_NONE), // DMD
   SHADER_SAMPLER(tex_sprite, texSampler1, Texture0, 0, SA_MIRROR, SA_MIRROR, SF_TRILINEAR), // Sprite
   // Flasher shader
   SHADER_SAMPLER(tex_flasher_A, texSampler0, Texture0, 0, SA_REPEAT, SA_REPEAT, SF_TRILINEAR), // base texture
   SHADER_SAMPLER(tex_flasher_B, texSampler1, Texture1, 1, SA_REPEAT, SA_REPEAT, SF_TRILINEAR), // texB
   // FB shader
   SHADER_SAMPLER(tex_fb_unfiltered, texSampler4, Texture0, 0, SA_CLAMP, SA_CLAMP, SF_NONE), // Framebuffer (unfiltered)
   SHADER_SAMPLER(tex_fb_filtered, texSampler5, Texture0, 0, SA_CLAMP, SA_CLAMP, SF_BILINEAR), // Framebuffer (filtered)
   SHADER_SAMPLER(tex_mirror, texSamplerMirror, Texture0, 0, SA_CLAMP, SA_CLAMP, SF_BILINEAR), // base mirror texture
   SHADER_SAMPLER(tex_bloom, texSamplerBloom, Texture1, 1, SA_CLAMP, SA_CLAMP, SF_BILINEAR), // Bloom
   SHADER_SAMPLER(tex_ao, texSampler3, Texture3, 2, SA_CLAMP, SA_CLAMP, SF_BILINEAR), // AO Result
   SHADER_SAMPLER(tex_depth, texSamplerDepth, Texture3, 2, SA_CLAMP, SA_CLAMP, SF_NONE), // Depth
   SHADER_SAMPLER(tex_color_lut, texSampler6, Texture4, 2, SA_CLAMP, SA_CLAMP, SF_BILINEAR), // Color grade LUT
   SHADER_SAMPLER(tex_ao_dither, texSamplerAOdither, Texture4, 3, SA_REPEAT, SA_REPEAT, SF_NONE), // AO dither
   // Ball shader
   SHADER_SAMPLER(tex_ball_color, texSampler0, Texture0, 0, SA_REPEAT, SA_REPEAT, SF_TRILINEAR), // base texture
   SHADER_SAMPLER(tex_ball_playfield, texSampler1, Texture1, 1, SA_CLAMP, SA_CLAMP, SF_TRILINEAR), // playfield
   //SHADER_SAMPLER(tex_diffuse_env, texSampler2, Texture2, 2, SA_REPEAT, SA_CLAMP, SF_BILINEAR), // diffuse environment contribution/radiance [Shared with basic]
   SHADER_SAMPLER(tex_ball_decal, texSampler7, Texture3, 3, SA_REPEAT, SA_REPEAT, SF_TRILINEAR), // ball decal
   // Basic shader
   SHADER_SAMPLER(tex_base_color, texSampler0, Texture0, 0, SA_REPEAT, SA_REPEAT, SF_TRILINEAR), // base texture
   SHADER_SAMPLER(tex_env, texSampler1, Texture1, 1, SA_REPEAT, SA_CLAMP, SF_TRILINEAR), // environment
   SHADER_SAMPLER(tex_diffuse_env, texSampler2, Texture2, 2, SA_REPEAT, SA_CLAMP, SF_BILINEAR), // diffuse environment contribution/radiance
   SHADER_SAMPLER(tex_base_transmission, texSamplerBL, Texture3, 3, SA_CLAMP, SA_CLAMP, SF_BILINEAR), // bulb light/transmission buffer texture
   SHADER_SAMPLER(tex_playfield_reflection, texSamplerPFReflections, Texture3, 3, SA_CLAMP, SA_CLAMP, SF_NONE), // playfield reflection
   SHADER_SAMPLER(tex_base_normalmap, texSamplerN, Texture4, 4, SA_REPEAT, SA_REPEAT, SF_TRILINEAR), // normal map texture
   // Classic light shader
   SHADER_SAMPLER(tex_light_color, texSampler0, Texture0, 0, SA_REPEAT, SA_REPEAT, SF_TRILINEAR), // base texture
   // SHADER_SAMPLER(tex_env, texSampler1, Texture1, 1, SA_REPEAT, SA_CLAMP, SF_TRILINEAR), // environment [Shared with basic]
   // SHADER_SAMPLER(tex_diffuse_env, texSampler2, Texture2, 2, SA_REPEAT, SA_CLAMP, SF_BILINEAR), // diffuse environment contribution/radiance [Shared with basic]
   // Stereo shader (VPVR only, combine the 2 rendered eyes into a single one)
   SHADER_SAMPLER(tex_stereo_fb, texSampler0, Texture0, 0, SA_CLAMP, SA_CLAMP, SF_NONE), // Framebuffer (unfiltered)
   // SMAA shader
   SHADER_SAMPLER(colorTex, colorTex, colorTex, 0, SA_CLAMP, SA_CLAMP, SF_BILINEAR),
   SHADER_SAMPLER(colorGammaTex, colorGammaTex, colorGammaTex, 1, SA_CLAMP, SA_CLAMP, SF_BILINEAR),
   SHADER_SAMPLER(edgesTex2D, edgesTex, edgesTex2D, 2, SA_CLAMP, SA_CLAMP, SF_BILINEAR),
   SHADER_SAMPLER(blendTex2D, blendTex, blendTex2D, 3, SA_CLAMP, SA_CLAMP, SF_BILINEAR),
   SHADER_SAMPLER(areaTex2D, areaTex, areaTex2D, 4, SA_CLAMP, SA_CLAMP, SF_BILINEAR),
   SHADER_SAMPLER(searchTex2D, searchTex, searchTex2D, 5, SA_CLAMP, SA_CLAMP, SF_NONE), // Note that this should have a w address mode set to clamp as well
   SHADER_UNIFORM_COUNT,
   SHADER_UNIFORM_INVALID
};
#undef SHADER_UNIFORM
#undef SHADER_SAMPLER

// When changed, this list must also be copied unchanged to Shader.cpp (for its implementation)
#define SHADER_ATTRIBUTE(name, shader_name) SHADER_ATTRIBUTE_##name
enum ShaderAttributes
{
   SHADER_ATTRIBUTE(POS, vPosition),
   SHADER_ATTRIBUTE(NORM, vNormal),
   SHADER_ATTRIBUTE(TC, tc),
   SHADER_ATTRIBUTE(TEX, tex0),
   SHADER_ATTRIBUTE_COUNT,
   SHADER_ATTRIBUTE_INVALID
};
#undef SHADER_ATTRIBUTE

class Shader final
{
public:
   Shader(RenderDevice *renderDevice);
   ~Shader();

   bool Load(const std::string name, const BYTE* code, UINT codeSize);

   void Begin();
   void End();

   void SetTexture(const ShaderUniforms texelName, BaseTexture* texel, const SamplerFilter filter = SF_UNDEFINED, const SamplerAddressMode clampU = SA_UNDEFINED, const SamplerAddressMode clampV = SA_UNDEFINED, const bool force_linear_rgb = false);
   void SetTexture(const ShaderUniforms texelName, Texture* texel, const SamplerFilter filter = SF_UNDEFINED, const SamplerAddressMode clampU = SA_UNDEFINED, const SamplerAddressMode clampV = SA_UNDEFINED, const bool force_linear_rgb = false);
   void SetTexture(const ShaderUniforms texelName, Sampler* texel);
   void SetTextureNull(const ShaderUniforms texelName);

   void SetMaterial(const Material * const mat, const bool has_alpha = true);

   void SetDisableLighting(const float value); // only set top
   void SetDisableLighting(const vec4& value); // sets the two top and below lighting flags, z and w unused
   void SetAlphaTestValue(const float value);
   void SetFlasherColorAlpha(const vec4& color);
   vec4 GetCurrentFlasherColorAlpha() const;
   void SetFlasherData(const vec4& c1, const vec4& c2);
   void SetLightColorIntensity(const vec4& color);
   void SetLightColor2FalloffPower(const vec4& color);
   void SetLightData(const vec4& color);
   void SetLightImageBackglassMode(const bool imageMode, const bool backglassMode);

   //

   void SetTechnique(const ShaderTechniques technique);
   void SetTechniqueMetal(const ShaderTechniques technique, const bool isMetal);
   ShaderTechniques GetCurrentTechnique() { return m_technique; }

   void SetMatrix(const ShaderUniforms hParameter, const D3DXMATRIX* pMatrix);
   void SetMatrix(const ShaderUniforms hParameter, const Matrix3D* pMatrix);
   void SetVector(const ShaderUniforms hParameter, const vec4* pVector);
   void SetVector(const ShaderUniforms hParameter, const float x, const float y, const float z, const float w);
   void SetFloat(const ShaderUniforms hParameter, const float f);
   void SetInt(const ShaderUniforms hParameter, const int i);
   void SetBool(const ShaderUniforms hParameter, const bool b);
   void SetFloat4v(const ShaderUniforms hParameter, const vec4* pData, const unsigned int count);
   void SetUniformBlock(const ShaderUniforms hParameter, const float* pMatrix, const size_t size);

   static void SetDefaultSamplerFilter(const ShaderUniforms sampler, const SamplerFilter sf);

   static Shader* GetCurrentShader();

   // state of what is actually bound per technique, and what is expected for the next begin/end
   struct UniformCache
   {
      size_t count = -1; // number of elements for uniform blocks and float vectors
      union UniformValue
      {
         int i; // integer and boolean
         float f; // float value
         float fv[16]; // float vectors and matrices
         Sampler* sampler; // texture samplers
         void* data = nullptr; // uniform blocks or float vector block
      } val;
   };
   uint32_t CopyUniformCache(const bool copyTo, const ShaderTechniques technique, UniformCache (&m_uniformCache)[SHADER_UNIFORM_COUNT]);

private:
   RenderDevice *m_renderDevice;
   static Shader* current_shader;
   ShaderTechniques m_technique;
   string m_shaderCodeName;

   // caches:

   Material currentMaterial;

   vec4 currentDisableLighting; // x and y: top and below, z and w unused

   static constexpr DWORD TEXTURESET_STATE_CACHE_SIZE = 5; // current convention: SetTexture gets "TextureX", where X 0..4
   BaseTexture *currentTexture[TEXTURESET_STATE_CACHE_SIZE];
   float currentAlphaTestValue;

   vec4 currentFlasherColor; // all flasher only-data
   vec4 currentFlasherData;
   vec4 currentFlasherData2; // w unused

   vec4 currentLightColor; // all light only-data
   vec4 currentLightColor2;
   vec4 currentLightData;
   unsigned int currentLightImageMode;
   unsigned int currentLightBackglassMode;

   struct ShaderUniform
   {
      ShaderUniformType type;
      string name;
      string legacy_name;
      string texture_ref;
      int default_tex_unit;
      SamplerAddressMode default_clampu;
      SamplerAddressMode default_clampv;
      SamplerFilter default_filter;
   };
   static const string shaderTechniqueNames[SHADER_TECHNIQUE_COUNT];
   static const string shaderAttributeNames[SHADER_ATTRIBUTE_COUNT];
   static ShaderUniform shaderUniformNames[SHADER_UNIFORM_COUNT];
   ShaderUniforms getUniformByName(const string& name);
   ShaderAttributes getAttributeByName(const string& name);
   ShaderTechniques getTechniqueByName(const string& name);

   void ApplyUniform(const ShaderUniforms uniformName);

   std::vector<ShaderUniforms> m_uniforms[SHADER_TECHNIQUE_COUNT];
   bool m_isCacheValid[SHADER_TECHNIQUE_COUNT];
   UniformCache m_uniformCache[SHADER_TECHNIQUE_COUNT + 1][SHADER_UNIFORM_COUNT];

   struct UniformDesc
   {
      ShaderUniform uniform;
      int count;
#ifdef ENABLE_SDL
      GLint location;
      GLuint blockBuffer;
#else
      D3DXHANDLE handle;
#endif
   };

#if DEBUG_LEVEL_LOG > 0
   void LOG(const int level, const string& fileNameRoot, const string& message);
#endif

#ifdef ENABLE_SDL

   struct attributeLoc
   {
      GLenum type;
      int location;
      int size;
   };
   struct ShaderTechnique
   {
      int index;
      string& name;
      GLuint program;
      attributeLoc attributeLocation[SHADER_ATTRIBUTE_COUNT];
      UniformDesc uniform_desc[SHADER_UNIFORM_COUNT];
   };

   std::ofstream* logFile;
   bool parseFile(const string& fileNameRoot, const string& fileName, int level, robin_hood::unordered_map<string, string>& values, const string& parentMode);
   string analyzeFunction(const string& shaderCodeName, const string& technique, const string& functionName, const robin_hood::unordered_map<string, string>& values);
   ShaderTechnique* compileGLShader(const ShaderTechniques technique, const string& fileNameRoot, string& shaderCodeName, const string& vertex, const string& geometry, const string& fragment);

   ShaderTechnique* m_techniques[SHADER_TECHNIQUE_COUNT];
   static Matrix3D mWorld, mView, mProj[2];

public:
   void setAttributeFormat(const DWORD fvf);

   static void SetTransform(const TransformStateType p1, const Matrix3D* p2, const int count);
   static void GetTransform(const TransformStateType p1, Matrix3D* p2, const int count);

   static string shaderPath;
   static string Defines;

#else
   ID3DXEffect * m_shader;
   UniformDesc m_uniform_desc[SHADER_UNIFORM_COUNT];
   ShaderTechniques m_bound_technique = ShaderTechniques::SHADER_TECHNIQUE_INVALID;

public:
   ID3DXEffect* Core() const { return m_shader; }
#endif
};
