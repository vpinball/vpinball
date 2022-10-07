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
   SHADER_TECHNIQUE(basic_without_texture_n_mirror),
   SHADER_TECHNIQUE(basic_with_texture_n_mirror),
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
   SHADER_TECHNIQUE(stereo_Anaglyph),
   SHADER_TECHNIQUE(stereo_AMD_DEBUG),
   SHADER_TECHNIQUE_COUNT,
   SHADER_TECHNIQUE_INVALID
};
#undef SHADER_TECHNIQUE

// Declaration of all uniforms and samplers used in the shaders
// When changed, this list must also be copied unchanged to Shader.cpp (for its implementation)
// Samplers defines how to sample a texture. For DX9, they are defined in the effect files, only the texture reference is set through the API.
// Otherwise, the sampler states can be directly overriden through DX9Device->SetSamplerState (per tex unit), being carefull that the effect
// framework will also apply the ones defined in the effect file during Technique->Begin call (so either don't define them, or reapply).
#define SHADER_UNIFORM(name) SHADER_##name
#define SHADER_TEXTURE(name) SHADER_##name
#define SHADER_SAMPLER(name, legacy_name, texture_ref, default_tex_unit, default_clampu, default_clampv, default_filter) SHADER_##name
enum ShaderUniforms
{
   // -- Floats --
   SHADER_UNIFORM(RenderBall),
   SHADER_UNIFORM(blend_modulate_vs_add),
   SHADER_UNIFORM(alphaTestValue),
   SHADER_UNIFORM(eye),
   SHADER_UNIFORM(fKickerScale),
   // -- Vectors and Float Arrays --
   SHADER_UNIFORM(Roughness_WrapL_Edge_Thickness),
   SHADER_UNIFORM(cBase_Alpha),
   SHADER_UNIFORM(lightCenter_maxRange),
   SHADER_UNIFORM(lightColor2_falloff_power),
   SHADER_UNIFORM(lightColor_intensity),
   SHADER_UNIFORM(matrixBlock),
   SHADER_UNIFORM(fenvEmissionScale_TexWidth),
   SHADER_UNIFORM(invTableRes_playfield_height_reflection),
   SHADER_UNIFORM(lightEmission),
   SHADER_UNIFORM(lightPos),
   SHADER_UNIFORM(orientation),
   SHADER_UNIFORM(cAmbient_LightRange),
   SHADER_UNIFORM(cClearcoat_EdgeAlpha),
   SHADER_UNIFORM(cGlossy_ImageLerp),
   SHADER_UNIFORM(fDisableLighting_top_below),
   SHADER_UNIFORM(backBoxSize),
   SHADER_UNIFORM(vColor_Intensity),
   SHADER_UNIFORM(w_h_height),
   SHADER_UNIFORM(alphaTestValueAB_filterMode_addBlend),
   SHADER_UNIFORM(amount_blend_modulate_vs_add_flasherMode),
   SHADER_UNIFORM(staticColor_Alpha),
   SHADER_UNIFORM(ms_zpd_ya_td),
   SHADER_UNIFORM(Anaglyph_DeSaturation_Contrast),
   SHADER_UNIFORM(vRes_Alpha_time),
   SHADER_UNIFORM(mirrorFactor),
   SHADER_UNIFORM(SSR_bumpHeight_fresnelRefl_scale_FS),
   SHADER_UNIFORM(AO_scale_timeblur),
   SHADER_UNIFORM(clip_planes),
   SHADER_UNIFORM(cWidth_Height_MirrorAmount),
   // -- Integer and Bool --
   SHADER_UNIFORM(ignoreStereo),
   SHADER_UNIFORM(disableLighting),
   SHADER_UNIFORM(lightSources),
   SHADER_UNIFORM(doNormalMapping),
   SHADER_UNIFORM(is_metal),
   SHADER_UNIFORM(color_grade),
   SHADER_UNIFORM(do_bloom),
   SHADER_UNIFORM(lightingOff),
   SHADER_UNIFORM(objectSpaceNormalMap),
   SHADER_UNIFORM(do_dither),
   SHADER_UNIFORM(imageBackglassMode),
   // FIXME -- Legacy Textures (to be removed) --
   SHADER_TEXTURE(Texture0),
   SHADER_TEXTURE(Texture1),
   SHADER_TEXTURE(Texture2),
   SHADER_TEXTURE(Texture3),
   SHADER_TEXTURE(Texture4),
   SHADER_TEXTURE(edgesTex2D),
   SHADER_TEXTURE(blendTex2D),
   SHADER_TEXTURE(areaTex2D),
   SHADER_TEXTURE(searchTex2D),
   // -- Samplers (a texture reference with sampling configuration) --
   // DMD shader
   SHADER_SAMPLER(tex_dmd, texSampler0, Texture0, 0, SA_MIRROR, SA_MIRROR, SF_NONE), // DMD
   SHADER_SAMPLER(tex_sprite, texSampler1, Texture0, 0, SA_MIRROR, SA_MIRROR, SF_TRILINEAR), // Sprite
   // Flasher shader
   SHADER_SAMPLER(tex_flasher_A, texSampler0, Texture0, 0, SA_REPEAT, SA_REPEAT, SF_TRILINEAR), // base texture
   SHADER_SAMPLER(tex_flasher_B, texSampler1, Texture1, 1, SA_REPEAT, SA_REPEAT, SF_TRILINEAR), // texB
   // FB shader
   SHADER_SAMPLER(tex_fb_unfiltered, texSampler4, Texture0, 0, SA_CLAMP, SA_CLAMP, SF_POINT), // Framebuffer (unfiltered)
   SHADER_SAMPLER(tex_fb_filtered, texSampler5, Texture0, 0, SA_CLAMP, SA_CLAMP, SF_BILINEAR), // Framebuffer (filtered)
   SHADER_SAMPLER(tex_mirror, texSamplerMirror, Texture0, 0, SA_CLAMP, SA_CLAMP, SF_BILINEAR), // base mirror texture
   SHADER_SAMPLER(tex_bloom, texSamplerBloom, Texture1, 1, SA_CLAMP, SA_CLAMP, SF_BILINEAR), // Bloom
   SHADER_SAMPLER(tex_ao, texSampler3, Texture3, 2, SA_CLAMP, SA_CLAMP, SF_BILINEAR), // AO Result
   SHADER_SAMPLER(tex_depth, texSamplerDepth, Texture3, 2, SA_CLAMP, SA_CLAMP, SF_NONE), // Depth
   SHADER_SAMPLER(tex_color_lut, texSampler6, Texture4, 2, SA_CLAMP, SA_CLAMP, SF_BILINEAR), // Color grade LUT
   SHADER_SAMPLER(tex_ao_dither, texSamplerAOdither, Texture4, 3, SA_REPEAT, SA_REPEAT, SF_NONE), // AO dither
   // Ball shader
   SHADER_SAMPLER(tex_ball_color, texSampler0, Texture0, 0, SA_REPEAT, SA_REPEAT, SF_TRILINEAR), // base texture
   SHADER_SAMPLER(tex_ball_playfield, texSampler1, Texture1, 1, SA_REPEAT, SA_REPEAT, SF_TRILINEAR), // playfield
   //SHADER_SAMPLER(tex_diffuse_env, texSampler2, Texture2, 2, SA_REPEAT, SA_CLAMP, SF_BILINEAR), // diffuse environment contribution/radiance [Shared with basic]
   SHADER_SAMPLER(tex_ball_decal, texSampler7, Texture3, 3, SA_REPEAT, SA_REPEAT, SF_TRILINEAR), // ball decal
   // Basic shader
   SHADER_SAMPLER(tex_base_color, texSampler0, Texture0, 0, SA_REPEAT, SA_REPEAT, SF_TRILINEAR), // base texture
   SHADER_SAMPLER(tex_env, texSampler1, Texture1, 1, SA_REPEAT, SA_CLAMP, SF_TRILINEAR), // environment
   SHADER_SAMPLER(tex_diffuse_env, texSampler2, Texture2, 2, SA_REPEAT, SA_CLAMP, SF_BILINEAR), // diffuse environment contribution/radiance
   SHADER_SAMPLER(tex_base_transmission, texSamplerBL, Texture3, 3, SA_CLAMP, SA_CLAMP, SF_BILINEAR), // bulb light/transmission buffer texture
   SHADER_SAMPLER(tex_base_normalmap, texSamplerN, Texture4, 4, SA_REPEAT, SA_REPEAT, SF_TRILINEAR), // normal map texture
   // Classic light shader
   SHADER_SAMPLER(tex_light_color, texSampler0, Texture0, 0, SA_REPEAT, SA_REPEAT, SF_TRILINEAR), // base texture
   // SHADER_SAMPLER(tex_env, texSampler1, Texture1, 1, SA_REPEAT, SA_CLAMP, SF_TRILINEAR), // environment [Shared with basic]
   // SHADER_SAMPLER(tex_diffuse_env, texSampler2, Texture2, 2, SA_REPEAT, SA_CLAMP, SF_BILINEAR), // diffuse environment contribution/radiance [Shared with basic]
   // Stereo shader (VPVR only, combine the 2 rendered eyes into a single one)
   SHADER_SAMPLER(tex_stereo_fb, texSampler0, Texture0, 0, SA_CLAMP, SA_CLAMP, SF_POINT), // Framebuffer (unfiltered)
   // SMAA shader
   // FIXME SMAA shader also use the color and colorGamma samplers. This has to be looked at more deeply for a clean implementation
   SHADER_SAMPLER(colorTex, colorTex, colorTex, 0, SA_CLAMP, SA_CLAMP, SF_TRILINEAR),
   SHADER_SAMPLER(colorGammaTex, colorGammaTex, colorGammaTex, 1, SA_CLAMP, SA_CLAMP, SF_TRILINEAR),
   SHADER_SAMPLER(edgesTex, edgesTex, edgesTex2D, 2, SA_CLAMP, SA_CLAMP, SF_TRILINEAR),
   SHADER_SAMPLER(blendTex, blendTex, blendTex2D, 3, SA_CLAMP, SA_CLAMP, SF_TRILINEAR),
   SHADER_SAMPLER(areaTex, areaTex, areaTex2D, 4, SA_CLAMP, SA_CLAMP, SF_TRILINEAR),
   SHADER_SAMPLER(searchTex, searchTex, searchTex2D, 5, SA_CLAMP, SA_CLAMP, SF_BILINEAR), // Not that this should have a w address mode set to clamp as well
   SHADER_UNIFORM_COUNT,
   SHADER_UNIFORM_INVALID
};
#undef SHADER_UNIFORM
#undef SHADER_TEXTURE
#undef SHADER_SAMPLER

#ifndef ENABLE_SDL

//Float
#define SHADER_blend_modulate_vs_add "blend_modulate_vs_add"
#define SHADER_alphaTestValue "alphaTestValue"
#define SHADER_eye "eye"
#define SHADER_fKickerScale "fKickerScale"

//Vectors and Float Arrays
#define SHADER_Roughness_WrapL_Edge_Thickness "Roughness_WrapL_Edge_Thickness"
#define SHADER_cBase_Alpha "cBase_Alpha"
#define SHADER_lightCenter_maxRange "lightCenter_maxRange"
#define SHADER_lightColor2_falloff_power "lightColor2_falloff_power"
#define SHADER_lightColor_intensity "lightColor_intensity"
#define SHADER_matrixBlock "matrixBlock"
#define SHADER_fenvEmissionScale_TexWidth "fenvEmissionScale_TexWidth"
#define SHADER_invTableRes_playfield_height_reflection "invTableRes_playfield_height_reflection"
#define SHADER_lightEmission "lightEmission"
#define SHADER_lightPos "lightPos"
#define SHADER_orientation "orientation"
#define SHADER_cAmbient_LightRange "cAmbient_LightRange"
#define SHADER_cClearcoat_EdgeAlpha "cClearcoat_EdgeAlpha"
#define SHADER_cGlossy_ImageLerp "cGlossy_ImageLerp"
#define SHADER_fDisableLighting_top_below "fDisableLighting_top_below"
#define SHADER_backBoxSize "backBoxSize"
#define SHADER_quadOffsetScale "quadOffsetScale"
#define SHADER_quadOffsetScaleTex "quadOffsetScaleTex"
#define SHADER_vColor_Intensity "vColor_Intensity"
#define SHADER_w_h_height "w_h_height"
#define SHADER_alphaTestValueAB_filterMode_addBlend "alphaTestValueAB_filterMode_addBlend"
#define SHADER_amount_blend_modulate_vs_add_flasherMode "amount_blend_modulate_vs_add_flasherMode"
#define SHADER_staticColor_Alpha "staticColor_Alpha"
#define SHADER_width_height_rotated_flipLR "width_height_rotated_flipLR"
#define SHADER_vRes_Alpha_time "vRes_Alpha_time"
#define SHADER_mirrorFactor "mirrorFactor"
#define SHADER_SSR_bumpHeight_fresnelRefl_scale_FS "SSR_bumpHeight_fresnelRefl_scale_FS"
#define SHADER_AO_scale_timeblur "AO_scale_timeblur"

//Integer
#define SHADER_ignoreStereo "ignoreStereo"
#define SHADER_disableLighting "disableLighting"
#define SHADER_lightSources "lightSources"
#define SHADER_doNormalMapping "doNormalMapping"
#define SHADER_is_metal "is_metal"
#define SHADER_color_grade "color_grade"
#define SHADER_do_bloom "do_bloom"
#define SHADER_lightingOff "lightingOff"
#define SHADER_objectSpaceNormalMap "objectSpaceNormalMap"
#define SHADER_do_dither "do_dither"

//Textures
#define SHADER_Texture0 "Texture0"
#define SHADER_Texture1 "Texture1"
#define SHADER_Texture2 "Texture2"
#define SHADER_Texture3 "Texture3"
#define SHADER_Texture4 "Texture4"
#define SHADER_edgesTex2D "edgesTex2D"
#define SHADER_blendTex2D "blendTex2D"
#define SHADER_areaTex2D "areaTex2D"
#define SHADER_searchTex2D "searchTex2D"

//Attributes
#define SHADER_ATTRIBUTE_POS "vPosition"
#define SHADER_ATTRIBUTE_NORM "vNormal"
#define SHADER_ATTRIBUTE_TC "tc"
#define SHADER_ATTRIBUTE_TEX "tex0"

typedef D3DXHANDLE SHADER_UNIFORM_HANDLE;
#endif

class Shader final
{
public:
   Shader(RenderDevice *renderDevice);
   ~Shader();

#ifdef ENABLE_SDL
   bool Load(const char* shaderCodeName, UINT codeSize);
#else
   bool Load(const BYTE* shaderCodeName, UINT codeSize);
   ID3DXEffect *Core() const { return m_shader; }
#endif
   void Unload();

   void Begin();
   void End();

   void SetTexture(const ShaderUniforms texelName, BaseTexture* texel, const SamplerFilter filter = SF_UNDEFINED, const SamplerAddressMode clampU = SA_UNDEFINED, const SamplerAddressMode clampV = SA_UNDEFINED, const bool force_linear_rgb = false);
   void SetTexture(const ShaderUniforms texelName, Texture* texel, const SamplerFilter filter = SF_UNDEFINED, const SamplerAddressMode clampU = SA_UNDEFINED, const SamplerAddressMode clampV = SA_UNDEFINED, const bool force_linear_rgb = false);
   void SetTexture(const ShaderUniforms texelName, Sampler* texel);
   void SetTextureNull(const ShaderUniforms texelName);
   void SetMaterial(const Material * const mat, const bool has_alpha = true);

   void SetDisableLighting(const vec4& value); // sets the two top and below lighting flags, z and w unused
   void SetAlphaTestValue(const float value);
   void SetFlasherColorAlpha(const vec4& color);
   vec4 GetCurrentFlasherColorAlpha();
   void SetFlasherData(const vec4& c1, const vec4& c2);
   void SetLightColorIntensity(const vec4& color);
   void SetLightColor2FalloffPower(const vec4& color);
   void SetLightData(const vec4& color);
   void SetLightImageBackglassMode(const bool imageMode, const bool backglassMode);

   //

   void SetTechnique(const ShaderTechniques technique);
   void SetTechniqueMetal(const ShaderTechniques technique, const bool isMetal);
   ShaderTechniques GetCurrentTechnique() { return m_technique; }

   void SetMatrix(const SHADER_UNIFORM_HANDLE hParameter, const D3DXMATRIX* pMatrix);
   void SetVector(const SHADER_UNIFORM_HANDLE hParameter, const vec4* pVector);
   void SetFloat(const SHADER_UNIFORM_HANDLE hParameter, const float f);
   void SetInt(const SHADER_UNIFORM_HANDLE hParameter, const int i);
   void SetBool(const SHADER_UNIFORM_HANDLE hParameter, const bool b);
   void SetValue(const SHADER_UNIFORM_HANDLE hParameter, const void* pData, const unsigned int Bytes);

   static void SetDefaultSamplerFilter(const ShaderUniforms sampler, const SamplerFilter sf);

   static Shader* GetCurrentShader();

#ifdef ENABLE_SDL
   // state of what is actually bound per technique, and what is expected for the next begin/end
   struct UniformCache
   {
      size_t count; // number of elements for uniform blocks and float vectors
      size_t capacity; // size of the datablock
      float* data; // uniform blocks & large float vectors data block
      union UniformValue
      {
         int i; // integer and boolean
         float f; // float value
         float fv[16]; // float vectors and matrices
         Sampler* sampler; // texture samplers
      } val;
   };
   uint32_t CopyUniformCache(const bool copyTo, const ShaderTechniques technique, UniformCache (&m_uniformCache)[SHADER_UNIFORM_COUNT]);
#endif

private:
   RenderDevice *m_renderDevice;
   static Shader* current_shader;
   ShaderTechniques m_technique;

   // caches:

   Material currentMaterial;

   vec4 currentDisableLighting; // x and y: top and below, z and w unused

   static constexpr DWORD TEXTURESET_STATE_CACHE_SIZE = 5; // current convention: SetTexture gets "TextureX", where X 0..4
   BaseTexture *currentTexture[TEXTURESET_STATE_CACHE_SIZE];
   float currentAlphaTestValue;
   char  currentTechnique[64];

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
      bool is_sampler;
      string name;
      string legacy_name;
      string texture_ref;
      int default_tex_unit;
      SamplerAddressMode default_clampu;
      SamplerAddressMode default_clampv;
      SamplerFilter default_filter;
   };
   static const string shaderTechniqueNames[SHADER_TECHNIQUE_COUNT];
   static ShaderUniform shaderUniformNames[SHADER_UNIFORM_COUNT];
   ShaderUniforms getUniformByName(const string& name);
   ShaderTechniques getTechniqueByName(const string& name);

#ifdef ENABLE_SDL
   string m_shaderCodeName;

   struct attributeLoc
   {
      GLenum type;
      int location;
      int size;
   };
   struct uniformLoc
   {
      GLenum type;
      GLint location;
      int size;
      GLuint blockBuffer;
   };
   struct ShaderTechnique
   {
      int index;
      string& name;
      GLuint program;
      attributeLoc attributeLocation[SHADER_ATTRIBUTE_COUNT];
      uniformLoc uniformLocation[SHADER_UNIFORM_COUNT];
   };

   std::ofstream* logFile;
#if DEBUG_LEVEL_LOG > 0
   void LOG(const int level, const string& fileNameRoot, const string& message);
#endif
   bool parseFile(const string& fileNameRoot, const string& fileName, int level, robin_hood::unordered_map<string, string>& values, const string& parentMode);
   string analyzeFunction(const char* shaderCodeName, const string& technique, const string& functionName, const robin_hood::unordered_map<string, string>& values);
   ShaderTechnique* compileGLShader(const ShaderTechniques technique, const string& fileNameRoot, string& shaderCodeName, const string& vertex, const string& geometry, const string& fragment);

   void ApplyUniform(const ShaderUniforms uniformName);

   std::vector<ShaderUniforms> m_uniforms[SHADER_TECHNIQUE_COUNT];
   bool m_isCacheValid[SHADER_TECHNIQUE_COUNT];
   UniformCache m_uniformCache[SHADER_TECHNIQUE_COUNT + 1][SHADER_UNIFORM_COUNT];
   ShaderTechnique* m_techniques[SHADER_TECHNIQUE_COUNT];
   ShaderTechniques m_technique;
   static Matrix3D mWorld, mView, mProj[2];

public:
   void setAttributeFormat(const DWORD fvf);

   static string shaderPath;
   static string Defines;

#else
   ID3DXEffect * m_shader;
#endif
};
