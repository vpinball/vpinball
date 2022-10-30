//!! have switch to choose if texture is weighted by diffuse/glossy or is just used raw?

#define NUM_BALL_LIGHTS 0 // just to avoid having too much constant mem allocated

#include "Helpers.fxh"

// transformation matrices
const float4x4 matWorldViewProj : WORLDVIEWPROJ;
const float4x4 matWorldView     : WORLDVIEW;
const float3x4 matWorldViewInverseTranspose;
const float4x3 matView;
//const float4x4 matViewInverseInverseTranspose; // matView used instead and multiplied from other side

texture Texture0; // base texture
texture Texture3; // bulb light buffer
texture Texture4; // normal map
texture Texture5; // reflection probe

sampler2D tex_base_color : TEXUNIT0 = sampler_state // base texture
{
    Texture   = (Texture0);
    //MIPFILTER = LINEAR; //!! HACK: not set here as user can choose to override trilinear by anisotropic
    //MAGFILTER = LINEAR;
    //MINFILTER = LINEAR;
    //ADDRESSU  = Wrap; //!! ?
    //ADDRESSV  = Wrap;
    SRGBTexture = true;
};

sampler2D tex_base_transmission : TEXUNIT3 = sampler_state // bulb light/transmission buffer texture
{
    Texture   = (Texture3);
    MIPFILTER = NONE; //!! ??
    MAGFILTER = LINEAR;
    MINFILTER = LINEAR;
    ADDRESSU  = Clamp;
    ADDRESSV  = Clamp;
};
 
sampler2D tex_base_normalmap : TEXUNIT4 = sampler_state // normal map texture
{
    Texture = (Texture4);
    //MIPFILTER = LINEAR; //!! HACK: not set here as user can choose to override trilinear by anisotropic //!! disallow and always use normal bilerp only? not even mipmaps?
    //MAGFILTER = LINEAR;
    //MINFILTER = LINEAR;
    //ADDRESSU  = Wrap; //!! ?
    //ADDRESSV  = Wrap;
};

sampler2D tex_reflection : TEXUNIT5 = sampler_state // reflection probe
{
   Texture = (Texture5);
   MIPFILTER = NONE;
   MAGFILTER = LINEAR;
   MINFILTER = NONE;
   ADDRESSU = Clamp;
   ADDRESSV = Clamp;
   SRGBTexture = true;
};

const bool objectSpaceNormalMap;

#include "Material.fxh"

const float4 cClearcoat_EdgeAlpha;
const float4 cGlossy_ImageLerp;
//!! No value is under 0.02
//!! Non-metals value are un-intuitively low: 0.02-0.08
//!! Gemstones are 0.05-0.17
//!! Metals have high specular reflectance: 0.5-1.0

const float4 staticColor_Alpha;
const float alphaTestValue;

const float4 cWidth_Height_MirrorAmount;
const float3 mirrorNormal;

struct VS_OUTPUT 
{ 
   float4 pos      : POSITION;
   float4 tex01    : TEXCOORD0; // pack tex0 and tex1 into one float4
   float3 worldPos : TEXCOORD1;
   float3 normal   : TEXCOORD2;
};

struct VS_NOTEX_OUTPUT 
{
   float4 pos      : POSITION;
   float4 worldPos_t1x : TEXCOORD0; // pack tex1 into w component of the float4s
   float4 normal_t1y : TEXCOORD1;
};

struct VS_DEPTH_ONLY_NOTEX_OUTPUT 
{ 
   float4 pos      : POSITION; 
};

struct VS_DEPTH_ONLY_TEX_OUTPUT
{
   float4 pos      : POSITION;
   float2 tex0     : TEXCOORD0;
};

float3x3 TBN_trafo(const float3 N, const float3 V, const float2 uv, const float3 dpx, const float3 dpy)
{
   // derivatives: edge vectors for tri-pos and tri-uv
   const float2 duvx = ddx(uv);
   const float2 duvy = ddy(uv);

   // solve linear system
   const float3 dp2perp = cross(N, dpy);
   const float3 dp1perp = cross(dpx, N);
   const float3 T = dp2perp * duvx.x + dp1perp * duvy.x;
   const float3 B = dp2perp * duvx.y + dp1perp * duvy.y;

   // construct scale-invariant transformation
   return float3x3(T, B, N * sqrt( max(dot(T,T), dot(B,B)) )); // inverse scale, as will be normalized anyhow later-on (to save some mul's)
}

float3 normal_map(const float3 N, const float3 V, const float2 uv)
{
   float3 tn = tex2D(tex_base_normalmap, uv).xyz * (255./127.) - (128./127.); // Note that Blender apparently does export tangent space normalmaps for Z (Blue) at full range, so 0..255 -> 0..1, which misses an option for here!

   const float3 dpx = ddx(V); //!! these 2 are declared here instead of TBN_trafo() to workaround a compiler quirk
   const float3 dpy = ddy(V);

   [branch] if (objectSpaceNormalMap)
   {
      tn.z = -tn.z; // this matches the object space, +X +Y +Z, export/baking in Blender with our trafo setup
      return normalize( mul(tn, matWorldViewInverseTranspose).xyz );
   } else // tangent space
      return normalize( mul(TBN_trafo(N, V, uv, dpx, dpy),
                            tn) );
}

// Compute reflections from reflection probe (screen space coordinates)
float3 compute_reflection(const float2 screenSpace, const float3 N)
{
   // Only apply to faces pointing in the direction of the probe (normal = [0,0,-1])
   // the smoothstep values are *magic* values taken from visual tests
   // dot(mirrorNormal, N) does not really needs to be done per pixel and could be moved to the vertx shader
   // Offset by half a texel to use GPU filtering for some blur
   return smoothstep(0.5, 0.9, dot(mirrorNormal, N)) * cWidth_Height_MirrorAmount.z * tex2D(tex_reflection, (screenSpace + float2(0.5, 0.5)) / cWidth_Height_MirrorAmount.xy).rgb;
}

//------------------------------------
//
// Standard Materials
//

VS_OUTPUT vs_main (const in float4 vPosition : POSITION0,
                   const in float3 vNormal   : NORMAL0,
                   const in float2 tc        : TEXCOORD0)
{
   // trafo all into worldview space (as most of the weird trafos happen in view, world is identity so far)
   const float3 P = mul(vPosition, matWorldView).xyz;
   const float3 N = normalize(mul(vNormal, matWorldViewInverseTranspose).xyz);

   VS_OUTPUT Out;
   Out.pos = mul(vPosition, matWorldViewProj);
   Out.tex01 = float4(tc, /*(cBase_Alpha.a < 1.0) ?*/Out.pos.xy/Out.pos.w);
   Out.worldPos = P;
   Out.normal = N;
   return Out; 
}

VS_NOTEX_OUTPUT vs_notex_main (const in float4 vPosition : POSITION0,
                               const in float3 vNormal   : NORMAL0,
                               const in float2 tc        : TEXCOORD0)
{
   // trafo all into worldview space (as most of the weird trafos happen in view, world is identity so far)
   const float3 P = mul(vPosition, matWorldView).xyz;
   const float3 N = normalize(mul(vNormal, matWorldViewInverseTranspose).xyz);

   VS_NOTEX_OUTPUT Out;
   Out.pos = mul(vPosition, matWorldViewProj);
   //if(cBase_Alpha.a < 1.0)
   {
      Out.worldPos_t1x.w = Out.pos.x/Out.pos.w;
      Out.normal_t1y.w = Out.pos.y/Out.pos.w;
   }
   Out.worldPos_t1x.xyz = P;
   Out.normal_t1y.xyz = N;
   return Out; 
}

VS_DEPTH_ONLY_NOTEX_OUTPUT vs_depth_only_main_without_texture(const in float4 vPosition : POSITION0)
{
   VS_DEPTH_ONLY_NOTEX_OUTPUT Out;

   Out.pos = mul(vPosition, matWorldViewProj);
   
   return Out; 
}

VS_DEPTH_ONLY_TEX_OUTPUT vs_depth_only_main_with_texture(const in float4 vPosition : POSITION0,
                                                         const in float2 tc : TEXCOORD0)
{
   VS_DEPTH_ONLY_TEX_OUTPUT Out;

   Out.pos = mul(vPosition, matWorldViewProj);
   Out.tex0 = tc;

   return Out;
}

float4 ps_main(const in VS_NOTEX_OUTPUT IN, float2 screenSpace : VPOS, uniform bool is_metal, uniform bool doReflections) : COLOR
{
   const float3 diffuse  = cBase_Alpha.xyz;
   const float3 glossy   = is_metal ? cBase_Alpha.xyz : cGlossy_ImageLerp.xyz*0.08;
   const float3 specular = cClearcoat_EdgeAlpha.xyz*0.08;
   const float  edge     = is_metal ? 1.0 : Roughness_WrapL_Edge_Thickness.z;

   const float3 V = normalize(/*camera=0,0,0,1*/-IN.worldPos_t1x.xyz);
   const float3 N = normalize(IN.normal_t1y.xyz);

   //return float4((N+1.0)*0.5,1.0); // visualize normals

   float4 result = float4(
      lightLoop(IN.worldPos_t1x.xyz, N, V, diffuse, glossy, specular, edge, true, is_metal), //!! have a "real" view vector instead that mustn't assume that viewer is directly in front of monitor? (e.g. cab setup) -> viewer is always relative to playfield and/or user definable
      cBase_Alpha.a);

   [branch] if (cBase_Alpha.a < 1.0) {
      result.a = GeometricOpacity(dot(N,V),result.a,cClearcoat_EdgeAlpha.w,Roughness_WrapL_Edge_Thickness.w);

      if (fDisableLighting_top_below.y < 1.0)
         // add light from "below" from user-flagged bulb lights, pre-rendered/blurred in previous renderpass //!! sqrt = magic
         result.xyz += lerp(sqrt(diffuse)*tex2Dlod(tex_base_transmission, float4(float2(0.5*IN.worldPos_t1x.w,-0.5*IN.normal_t1y.w)+0.5, 0.,0.)).xyz*result.a, 0., fDisableLighting_top_below.y); //!! depend on normal of light (unknown though) vs geom normal, too?
   }

   [branch] if (doReflections)
      result.rgb += compute_reflection(screenSpace, N);

   return result * staticColor_Alpha;
}

float4 ps_main_texture(const in VS_OUTPUT IN, float2 screenSpace : VPOS, uniform bool is_metal, uniform bool doNormalMapping, uniform bool doReflections) : COLOR
{
   float4 pixel = tex2D(tex_base_color, IN.tex01.xy);

   clip(pixel.a <= alphaTestValue ? - 1 : 1); // stop the pixel shader if alpha test should reject pixel

   pixel.a *= cBase_Alpha.a;
   if (fDisableLighting_top_below.x < 1.0) // if there is lighting applied, make sure to clamp the values (as it could be coming from a HDR tex)
      pixel.xyz = saturate(pixel.xyz);

   const float3 t = /*InvGamma*/(pixel.xyz); // uses automatic sRGB trafo instead in sampler! also by now e.g. primitives allow for HDR textures for lightmaps

   const float3 diffuse  = t*cBase_Alpha.xyz;
   const float3 glossy   = is_metal ? diffuse : (t*cGlossy_ImageLerp.w + (1.0-cGlossy_ImageLerp.w))*cGlossy_ImageLerp.xyz*0.08; //!! use AO for glossy? specular?
   const float3 specular = cClearcoat_EdgeAlpha.xyz*0.08;
   const float  edge     = is_metal ? 1.0 : Roughness_WrapL_Edge_Thickness.z;

   const float3 V = normalize(/*camera=0,0,0,1*/-IN.worldPos);
   float3 N = normalize(IN.normal);

   [branch] if (doNormalMapping)
      N = normal_map(N, V, IN.tex01.xy);
   
   //!! return float4((N+1.0)*0.5,1.0); // visualize normals

   float4 result = float4(
      lightLoop(IN.worldPos, N, V, diffuse, glossy, specular, edge, !doNormalMapping, is_metal),
      pixel.a);

   [branch] if (cBase_Alpha.a < 1.0 && result.a < 1.0) {
      result.a = GeometricOpacity(dot(N,V),result.a,cClearcoat_EdgeAlpha.w,Roughness_WrapL_Edge_Thickness.w);

      if (fDisableLighting_top_below.y < 1.0)
         // add light from "below" from user-flagged bulb lights, pre-rendered/blurred in previous renderpass //!! sqrt = magic
         result.xyz += lerp(sqrt(diffuse)*tex2Dlod(tex_base_transmission, float4(float2(0.5*IN.tex01.z,-0.5*IN.tex01.w)+0.5, 0., 0.)).xyz*result.a, 0., fDisableLighting_top_below.y); //!! depend on normal of light (unknown though) vs geom normal, too?
   }

   [branch] if (doReflections)
      result.rgb += compute_reflection(screenSpace, N);

   return result * staticColor_Alpha;
}

float4 ps_main_reflection_only_without_texture(const in VS_NOTEX_OUTPUT IN, float2 screenSpace : VPOS) : COLOR
{
   float4 result;
   float3 N = normalize(IN.normal_t1y.xyz);
   result.rgb = compute_reflection(screenSpace, N);
   result.a = 1.0;
   return result * staticColor_Alpha;
}

float4 ps_main_reflection_only_with_texture(const in VS_OUTPUT IN, float2 screenSpace : VPOS) : COLOR
{
   float4 result;

   result.a = tex2D(tex_base_color, IN.tex01.xy).a;

   clip(result.a <= alphaTestValue ? -1 : 1); // stop the pixel shader if alpha test should reject pixel

   result.a *= cBase_Alpha.a;

   float3 N = normalize(IN.normal);
   result.rgb = compute_reflection(screenSpace, N);

   // Maybe overkill for just the additive reflections
   [branch] if (cBase_Alpha.a < 1.0 && result.a < 1.0)
   {
      const float3 V = normalize(/*camera=0,0,0,1*/ -IN.worldPos);
      result.a = GeometricOpacity(dot(N, V), result.a, cClearcoat_EdgeAlpha.w, Roughness_WrapL_Edge_Thickness.w);
   }

   return result * staticColor_Alpha;
}

float4 ps_main_depth_only_without_texture(const in VS_DEPTH_ONLY_NOTEX_OUTPUT IN) : COLOR
{
    return float4(0.,0.,0.,1.);
}

float4 ps_main_depth_only_with_texture(const in VS_DEPTH_ONLY_TEX_OUTPUT IN) : COLOR
{
   clip(tex2D(tex_base_color, IN.tex0).a <= alphaTestValue ? -1 : 1); // stop the pixel shader if alpha test should reject pixel

   return float4(0., 0., 0., 1.);
}

//------------------------------------------
// BG-Decal

float4 ps_main_bg_decal(const in VS_NOTEX_OUTPUT IN) : COLOR
{
   return float4(InvToneMap(cBase_Alpha.xyz), cBase_Alpha.a);
}

float4 ps_main_bg_decal_texture(const in VS_OUTPUT IN) : COLOR
{
   float4 pixel = tex2D(tex_base_color, IN.tex01.xy);

   clip(pixel.a <= alphaTestValue ? - 1 : 1); // stop the pixel shader if alpha test should reject pixel

   pixel.a *= cBase_Alpha.a;
   const float3 t = /*InvGamma*/(pixel.xyz); // uses automatic sRGB trafo instead in sampler!

   return float4(InvToneMap(t*cBase_Alpha.xyz), pixel.a);
}

//------------------------------------------
// Kicker boolean vertex shader

const float fKickerScale;

VS_NOTEX_OUTPUT vs_kicker (const in float4 vPosition : POSITION0,
                           const in float3 vNormal   : NORMAL0,
                           const in float2 tc        : TEXCOORD0)
{
    const float3 P = mul(vPosition, matWorldView).xyz;
    const float3 N = normalize(mul(vNormal, matWorldViewInverseTranspose).xyz);

    VS_NOTEX_OUTPUT Out;
    Out.pos.xyw = mul(vPosition, matWorldViewProj).xyw;
    float4 P2 = vPosition;
    P2.z -= 30.0*fKickerScale; //!!
    Out.pos.z = mul(P2, matWorldViewProj).z;
    //if(cBase_Alpha.a < 1.0)
    {
        Out.worldPos_t1x.w = Out.pos.x/Out.pos.w; //!! not necessary
        Out.normal_t1y.w = Out.pos.y/Out.pos.w; //!! not necessary
    }
    Out.worldPos_t1x.xyz = P;
    Out.normal_t1y.xyz = N;
    return Out; 
}

//------------------------------------
// Techniques
//

//
// Standard Materials
//

technique basic_without_texture_isMetal
{ 
   pass P0 
   { 
      VertexShader = compile vs_3_0 vs_notex_main(); 
      PixelShader  = compile ps_3_0 ps_main(1, 0);
   } 
}

technique basic_without_texture
{ 
   pass P0 
   { 
      VertexShader = compile vs_3_0 vs_notex_main(); 
      PixelShader  = compile ps_3_0 ps_main(0, 0);
   } 
}

technique basic_with_texture_isMetal
{ 
   pass P0 
   { 
      VertexShader = compile vs_3_0 vs_main(); 
      PixelShader  = compile ps_3_0 ps_main_texture(1, 0, 0);
   } 
}

technique basic_with_texture
{ 
   pass P0 
   { 
      VertexShader = compile vs_3_0 vs_main(); 
      PixelShader  = compile ps_3_0 ps_main_texture(0, 0, 0);
   } 
}

technique basic_with_texture_normal_isMetal
{
   pass P0
   {
      VertexShader = compile vs_3_0 vs_main();
      PixelShader  = compile ps_3_0 ps_main_texture(1, 1, 0);
   }
}

technique basic_with_texture_normal
{
   pass P0
   {
      VertexShader = compile vs_3_0 vs_main();
      PixelShader  = compile ps_3_0 ps_main_texture(0, 1, 0);
   }
}


technique basic_with_refl_without_texture_isMetal
{
   pass P0
   {
      VertexShader = compile vs_3_0 vs_notex_main();
      PixelShader = compile ps_3_0 ps_main(1, 1);
   }
}

technique basic_with_refl_without_texture
{
   pass P0
   {
      VertexShader = compile vs_3_0 vs_notex_main();
      PixelShader = compile ps_3_0 ps_main(0, 1);
   }
}

technique basic_with_refl_with_texture_isMetal
{
   pass P0
   {
      VertexShader = compile vs_3_0 vs_main();
      PixelShader = compile ps_3_0 ps_main_texture(1, 0, 1);
   }
}

technique basic_with_refl_with_texture
{
   pass P0
   {
      VertexShader = compile vs_3_0 vs_main();
      PixelShader = compile ps_3_0 ps_main_texture(0, 0, 1);
   }
}

technique basic_with_refl_with_texture_normal_isMetal
{
   pass P0
   {
      VertexShader = compile vs_3_0 vs_main();
      PixelShader = compile ps_3_0 ps_main_texture(1, 1, 1);
   }
}

technique basic_with_refl_with_texture_normal
{
   pass P0
   {
      VertexShader = compile vs_3_0 vs_main();
      PixelShader = compile ps_3_0 ps_main_texture(0, 1, 1);
   }
}

technique basic_refl_only_without_texture
{
   pass P0
   {
      VertexShader = compile vs_3_0 vs_notex_main();
      PixelShader = compile ps_3_0 ps_main_reflection_only_without_texture();
   }
}

technique basic_refl_only_with_texture
{
   pass P0
   {
      VertexShader = compile vs_3_0 vs_main();
      PixelShader = compile ps_3_0 ps_main_reflection_only_with_texture();
   }
}

technique basic_depth_only_without_texture
{
   pass P0
   {
      VertexShader = compile vs_3_0 vs_depth_only_main_without_texture();
      PixelShader  = compile ps_3_0 ps_main_depth_only_without_texture();
   }
}

technique basic_depth_only_with_texture
{ 
   pass P0
   {
      VertexShader = compile vs_3_0 vs_depth_only_main_with_texture(); 
      PixelShader  = compile ps_3_0 ps_main_depth_only_with_texture();
   }
}

//
// BG-Decal
//

technique bg_decal_without_texture
{ 
   pass P0
   {
      VertexShader = compile vs_3_0 vs_notex_main(); 
      PixelShader  = compile ps_3_0 ps_main_bg_decal();
   }
}

technique bg_decal_with_texture
{ 
   pass P0
   {
      VertexShader = compile vs_3_0 vs_main();
      PixelShader  = compile ps_3_0 ps_main_bg_decal_texture();
   }
}


//
// Kicker
//

technique kickerBoolean_isMetal
{ 
   pass P0
   {
      //ZWriteEnable=TRUE;
      VertexShader = compile vs_3_0 vs_kicker();
      PixelShader  = compile ps_3_0 ps_main(1, 0);
   }
}

technique kickerBoolean
{ 
   pass P0
   {
      //ZWriteEnable=TRUE;
      VertexShader = compile vs_3_0 vs_kicker();
      PixelShader  = compile ps_3_0 ps_main(0, 0);
   }
}

#ifndef SEPARATE_CLASSICLIGHTSHADER
 #include "ClassicLightShader.hlsl"
#endif
