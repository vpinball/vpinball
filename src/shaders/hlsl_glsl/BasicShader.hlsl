// license:GPLv3+

//!! have switch to choose if texture is weighted by diffuse/glossy or is just used raw?

#define NUM_BALL_LIGHTS 0 // just to avoid having too much constant mem allocated

#include "Helpers.fxh"

// transformation matrices
const float4x4 matWorldViewProj : WORLDVIEWPROJ;
const float4x4 matWorldView     : WORLDVIEW;
const float3x4 matWorldViewInverseTranspose;
const float4x4 matWorld;
const float4x3 matView;
const float4x4 matProj;
//const float4x4 matViewInverseInverseTranspose; // matView used instead and multiplied from other side

const float4 lightCenter_doShadow;

texture Texture0; // base texture
texture Texture3; // bulb light buffer
texture Texture4; // normal map
texture Texture5; // reflection probe
texture Texture6; // refraction probe
texture Texture7; // depth probe


sampler2D tex_base_color : TEXUNIT0 = sampler_state // base texture
{
   Texture     = (Texture0);
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
   Texture     = (Texture4);
   //MIPFILTER = LINEAR; //!! HACK: not set here as user can choose to override trilinear by anisotropic //!! disallow and always use normal bilerp only? not even mipmaps?
   //MAGFILTER = LINEAR;
   //MINFILTER = LINEAR;
   //ADDRESSU  = Wrap; //!! ?
   //ADDRESSV  = Wrap;
};

sampler2D tex_reflection : TEXUNIT5 = sampler_state // reflection probe
{
   Texture     = (Texture5);
   MIPFILTER   = NONE;
   MAGFILTER   = LINEAR;
   MINFILTER   = LINEAR;
   ADDRESSU    = Clamp;
   ADDRESSV    = Clamp;
   SRGBTexture = false;
};

sampler2D tex_refraction : TEXUNIT6 = sampler_state // refraction probe
{
   Texture     = (Texture6);
   MIPFILTER   = NONE;
   MAGFILTER   = LINEAR;
   MINFILTER   = LINEAR;
   ADDRESSU    = Clamp;
   ADDRESSV    = Clamp;
   SRGBTexture = false;
};

sampler2D tex_probe_depth : TEXUNIT7 = sampler_state // depth probe
{
   Texture     = (Texture7);
   MIPFILTER   = NONE;
   MAGFILTER   = LINEAR;
   MINFILTER   = LINEAR;
   ADDRESSU    = Clamp;
   ADDRESSV    = Clamp;
   SRGBTexture = false;
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

const float4 w_h_height;

const float4 mirrorNormal_factor;
#define mirrorNormal (mirrorNormal_factor.xyz)
#define mirrorFactor (mirrorNormal_factor.w)

// DX9 needs these to be defined outside of the shader (unless, you get a X4014 error), so use the effect framework for these
//uniform bool is_metal;
//uniform bool doNormalMapping;
//const bool doReflections;
//const bool doRefractions;

const float4 refractionTint_thickness;
#define refractionTint (refractionTint_thickness.rgb)
#define refractionThickness (refractionTint_thickness.w)

struct VS_OUTPUT 
{
   float4 pos      : POSITION;  // projected position (in camera space, normalized, not divided by w, vertex position * world matrix * view matrix * projection matrix)
   float3 worldPos : TEXCOORD0; // position in view space (vertex position * world matrix * view matrix)
   float3 tablePos : TEXCOORD1; // position in table space (vertex position * world matrix)
   float3 normal   : TEXCOORD2;
   float2 tex0     : TEXCOORD3;
};

struct VS_NOTEX_OUTPUT 
{
   float4 pos      : POSITION;  // projected position (in camera space, normalized, not divided by w, vertex position * world matrix * view matrix * projection matrix)
   float3 worldPos : TEXCOORD0; // position in view space (vertex position * world matrix * view matrix)
   float3 tablePos : TEXCOORD1; // position in table space (vertex position * world matrix)
   float3 normal   : TEXCOORD2;
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

struct VS_UNSHADED_NOTEX_SHADOW_OUTPUT
{
    float4 pos      : POSITION;
    float3 tablePos : TEXCOORD1; // position in table space (vertex position * world matrix)
};

struct VS_UNSHADED_TEX_SHADOW_OUTPUT
{
    float4 pos      : POSITION;
    float2 tex0     : TEXCOORD0;
    float3 tablePos : TEXCOORD1; // position in table space (vertex position * world matrix)
};

#include "BallShadows.fxh"

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
   const float3 dpx = ddx(V); //!! these 2 are declared here instead of TBN_trafo() to workaround a compiler quirk
   const float3 dpy = ddy(V);

   const float3 tn = tex2D(tex_base_normalmap, uv).xyz * (255. / 127.) - (128. / 127.);
   BRANCH if (objectSpaceNormalMap)
   { // Object space: this matches the object space, +X +Y +Z, export/baking in Blender with our trafo setup
      return normalize(mul(float3(tn.x, tn.y, -tn.z), matWorldViewInverseTranspose).xyz);
   }
   else
   { // Tangent space
      return normalize(mul(tn, TBN_trafo(N, V, uv, dpx, dpy)));
   }
}

// Compute reflections from reflection probe (screen space coordinates)
// This is a simplified reflection model where reflected light is added instead of being mixed according to the fresnel coefficient (stronger reflection hiding the object's color at grazing angles)
float3 compute_reflection(const float2 screenSpace, const float3 N)
{
   // Only apply to faces pointing in the direction of the probe (normal = [0,0,-1])
   // the smoothstep values are *magic* values taken from visual tests
   // dot(mirrorNormal, N) does not really needs to be done per pixel and could be moved to the vertx shader
   // Offset by half a texel to use GPU filtering for some blur
   return smoothstep(0.5, 0.9, dot(mirrorNormal, N)) * mirrorFactor * tex2D(tex_reflection, screenSpace + 0.5 * w_h_height.xy).rgb;
}

// Compute refractions from screen space probe
float3 compute_refraction(const float3 pos, const float2 screenSpace, const float3 N, const float3 V)
{
   // Compute refracted visible position then project from world view position to probe UV
   // const float4x4 matProj = mul(inverse4x4(matWorldView), matWorldViewProj); // this has been moved to the matrix uniform stack for performance reasons
   const float3 R = refract(V, N, 1.0 / 1.5); // n1 = 1.0 (air), n2 = 1.5 (plastic), eta = n1 / n2
   const float3 refracted_pos = pos + refractionThickness * R; // Shift ray by the thickness of the material
   const float4 proj = mul(float4(refracted_pos, 1.0), matProj);
   float2 uv = float2(0.5, 0.5) + float2(proj.x, -proj.y) * (0.5 / proj.w);

   // Check if the sample position is behind the object pos. If it is, don't perform refraction as it would lead to refract things above us (like a reflection instead of a refraction)
   const float d = tex2D(tex_probe_depth, uv).x;
   const float4 proj_base = mul(float4(pos, 1.0), matProj); // Sadly DX9 does not give access to transformed fragment position and we need to project it again here...
   if (d < proj_base.z / proj_base.w)
      uv = screenSpace;

   // The following code gives a smoother transition but depends too much on the POV since it uses homogeneous depth to lerp instead of fragment's world depth
   //const float3 unbiased = float3(1.0, 0.0, 0.0);
   //const float3 biased = float3(0.0, 1.0, 0.0);
   //const float3 unbiased = tex2D(tex_refraction, screenSpace).rgb;
   //const float3 biased = tex2D(tex_refraction, uv).rgb;
   //return lerp(unbiased, biased, saturate(100.0 * (d - proj_base.z / proj_base.w)));

   /* // Debug output
   if (length(N) < 0.5) // invalid normal, shown as red for debugging
      return float3(1.0, 0.0, 0.0);
   if (dot(N, V) < 0.0) // Wrong orientation? (looking from the back, shown as blue for debugging)
      return float3(0.0, 0.0, 1.0);
   if (length(R) < 0.5) // invalid refraction state (no refraction, shown as green for debugging)
      return float3(0.0, 1.0, 0.0);*/

   return refractionTint.rgb * tex2D(tex_refraction, uv).rgb;
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
   // Needs to use a 'normal' matrix, and to normalize since we allow non uniform stretching, therefore matWorldView is not orthonormal
   const float3 N = normalize(mul(vNormal, matWorldViewInverseTranspose).xyz);

   VS_OUTPUT Out;
   Out.pos = mul(vPosition, matWorldViewProj);
   Out.tablePos = mul(vPosition, matWorld).xyz;
   Out.worldPos = P;
   Out.normal = N;
   Out.tex0 = tc;
   return Out;
}

VS_NOTEX_OUTPUT vs_notex_main (const in float4 vPosition : POSITION0,
                               const in float3 vNormal   : NORMAL0,
                               const in float2 tc        : TEXCOORD0)
{
   // trafo all into worldview space (as most of the weird trafos happen in view, world is identity so far)
   const float3 P = mul(vPosition, matWorldView).xyz;
   // Needs to use a 'normal' matrix, and to normalize since we allow non uniform stretching, therefore matWorldView is not orthonormal
   const float3 N = normalize(mul(vNormal, matWorldViewInverseTranspose).xyz);

   VS_NOTEX_OUTPUT Out;
   Out.pos = mul(vPosition, matWorldViewProj);
   Out.tablePos = mul(vPosition, matWorld).xyz;
   Out.worldPos = P;
   Out.normal = N;
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

VS_UNSHADED_NOTEX_SHADOW_OUTPUT vs_main_unshaded_without_texture_shadow(const in float4 vPosition : POSITION0)
{
    VS_UNSHADED_NOTEX_SHADOW_OUTPUT Out;
    Out.pos = mul(vPosition, matWorldViewProj);
    Out.tablePos = mul(vPosition, matWorld).xyz;
    return Out;
}

VS_UNSHADED_TEX_SHADOW_OUTPUT vs_main_unshaded_with_texture_shadow(const in float4 vPosition : POSITION0, const in float2 tc : TEXCOORD0)
{
    VS_UNSHADED_TEX_SHADOW_OUTPUT Out;
    Out.pos = mul(vPosition, matWorldViewProj);
    Out.tablePos = mul(vPosition, matWorld).xyz;
    Out.tex0 = tc;
    return Out;
}

float4 ps_main(const in VS_NOTEX_OUTPUT IN, float2 screenSpace : VPOS, uniform bool is_metal, uniform bool doReflections, uniform bool doRefractions) : COLOR
{
   const float3 diffuse  = cBase_Alpha.xyz;
   const float3 glossy   = is_metal ? cBase_Alpha.xyz : cGlossy_ImageLerp.xyz*0.08;
   const float3 specular = cClearcoat_EdgeAlpha.xyz*0.08;
   const float  edge     = is_metal ? 1.0 : Roughness_WrapL_Edge_Thickness.z;

   const float3 V = normalize( /*camera=0,0,0,1*/-IN.worldPos); //!! have a "real" view vector instead that mustn't assume that viewer is directly in front of monitor? (e.g. cab setup) -> viewer is always relative to playfield and/or user definable
   const float3 N = normalize(IN.normal);

   float4 color = float4(lightLoop(IN.worldPos.xyz, N, V, diffuse, glossy, specular, edge, is_metal), cBase_Alpha.a);

   BRANCH if (color.a < 1.0)
   {
      color.a = GeometricOpacity(dot(N,V), color.a, cClearcoat_EdgeAlpha.w, Roughness_WrapL_Edge_Thickness.w);

      BRANCH if (fDisableLighting_top_below.y < 1.0)
         // add light from "below" from user-flagged bulb lights, pre-rendered/blurred in previous renderpass //!! sqrt = magic
         color.rgb += lerp(sqrt(diffuse)*texNoLod(tex_base_transmission, screenSpace * w_h_height.xy).rgb*color.a, 0., fDisableLighting_top_below.y); //!! depend on normal of light (unknown though) vs geom normal, too?
   }

   BRANCH if (lightCenter_doShadow.w != 0.)
   {
      const float3 light_dir = IN.tablePos.xyz - lightCenter_doShadow.xyz;
      const float light_dist = length(light_dir);
      const float shadow = get_light_ball_shadow(lightCenter_doShadow.xyz, light_dir, light_dist);
      color.rgb *= shadow;
   }

   BRANCH if (doReflections)
      color.rgb += compute_reflection(screenSpace * w_h_height.xy, N);

   BRANCH if (doRefractions)
   {
      // alpha channel is the transparency of the object, tinting is supported even if alpha is 0 by applying a tint color
      color.rgb = lerp(compute_refraction(IN.worldPos.xyz, screenSpace * w_h_height.xy, N, V), color.rgb, cBase_Alpha.a);
      color.a = 1.0;
   }

   return color * staticColor_Alpha;
}

float4 ps_main_texture(const in VS_OUTPUT IN, float2 screenSpace : VPOS, uniform bool has_alphatest, uniform bool is_metal, uniform bool doNormalMapping, uniform bool doReflections, uniform bool doRefractions) : COLOR
{
   float4 pixel = tex2D(tex_base_color, IN.tex0);

   BRANCH if (has_alphatest)
      clip(pixel.a <= alphaTestValue ? - 1 : 1); // stop the pixel shader if alpha test should reject pixel

   pixel.a *= cBase_Alpha.a;
   if (fDisableLighting_top_below.x < 1.0) // if there is lighting applied, make sure to clamp the values (as it could be coming from a HDR tex)
      pixel.xyz = saturate(pixel.xyz);
   const float3 diffuse  = pixel.xyz * cBase_Alpha.xyz;
   const float3 glossy   = is_metal ? diffuse : (pixel.xyz * cGlossy_ImageLerp.w + (1.0-cGlossy_ImageLerp.w)) * cGlossy_ImageLerp.xyz * 0.08; //!! use AO for glossy? specular?
   const float3 specular = cClearcoat_EdgeAlpha.xyz * 0.08;
   const float  edge     = is_metal ? 1.0 : Roughness_WrapL_Edge_Thickness.z;

   const float3 V = normalize(/*camera=0,0,0,1*/-IN.worldPos);
   float3 N = normalize(IN.normal);
   BRANCH if (doNormalMapping)
      N = normal_map(N, V, IN.tex0);

   float4 color = float4(lightLoop(IN.worldPos, N, V, diffuse, glossy, specular, edge, is_metal), pixel.a);

   BRANCH if (color.a < 1.0) // We may not opacify if we already are opaque
   {
      color.a = GeometricOpacity(dot(N,V), color.a, cClearcoat_EdgeAlpha.w, Roughness_WrapL_Edge_Thickness.w);

      BRANCH if(fDisableLighting_top_below.y < 1.0)
         // add light from "below" from user-flagged bulb lights, pre-rendered/blurred in previous renderpass //!! sqrt = magic
         color.rgb += lerp(sqrt(diffuse) * texNoLod(tex_base_transmission, screenSpace * w_h_height.xy).rgb * color.a, 0., fDisableLighting_top_below.y); //!! depend on normal of light (unknown though) vs geom normal, too?
   }

   BRANCH if (lightCenter_doShadow.w != 0.)
   {
      const float3 light_dir = IN.tablePos.xyz - lightCenter_doShadow.xyz;
      const float light_dist = length(light_dir);
      const float shadow = get_light_ball_shadow(lightCenter_doShadow.xyz, light_dir, light_dist);
      color.rgb *= shadow;
   }

   BRANCH if (doReflections)
      color.rgb += compute_reflection(screenSpace * w_h_height.xy, N);

   BRANCH if (doRefractions)
   {
      // alpha channel is the transparency of the object, tinting is supported even if alpha is 0 by applying a tint color (not from main texture since this is different information (reflected/refracted color))
      color.rgb = lerp(compute_refraction(IN.worldPos, screenSpace * w_h_height.xy, N, V), color.rgb, color.a);
      color.a = 1.0;
   }

   // visualize normals for debugging
   // color.rgb = color.rgb * 0.0001 + 0.5 + 0.5 * N.rgb;

   return color * staticColor_Alpha;
}

float4 ps_main_reflection_only(const in VS_NOTEX_OUTPUT IN, float2 screenSpace : VPOS) : COLOR
{
   float4 color;
   const float3 N = normalize(IN.normal);
   color.rgb = compute_reflection(screenSpace * w_h_height.xy, N);
   color.a = 1.0;
   return color * staticColor_Alpha;
}

float4 ps_main_unshaded_without_texture(const in VS_DEPTH_ONLY_NOTEX_OUTPUT IN) : COLOR
{
   return staticColor_Alpha;
}

float4 ps_main_unshaded_with_texture(const in VS_DEPTH_ONLY_TEX_OUTPUT IN) : COLOR
{
   return staticColor_Alpha * tex2D(tex_base_color, IN.tex0);
}

float4 ps_main_unshaded_without_texture_shadow(const in VS_UNSHADED_NOTEX_SHADOW_OUTPUT IN) : COLOR
{
   const float3 light_dir = IN.tablePos.xyz - lightCenter_doShadow.xyz;
   const float light_dist = length(light_dir);
   const float shadow = get_light_ball_shadow(lightCenter_doShadow.xyz, light_dir, light_dist);
   return shadow * staticColor_Alpha;
}

float4 ps_main_unshaded_with_texture_shadow(const in VS_UNSHADED_TEX_SHADOW_OUTPUT IN) : COLOR
{
   const float3 light_dir = IN.tablePos.xyz - lightCenter_doShadow.xyz;
   const float light_dist = length(light_dir);
   const float shadow = get_light_ball_shadow(lightCenter_doShadow.xyz, light_dir, light_dist);
   return shadow * staticColor_Alpha * tex2D(tex_base_color, IN.tex0);
}

//------------------------------------------
// BG-Decal

float4 ps_main_bg_decal(const in VS_NOTEX_OUTPUT IN) : COLOR
{
   return cBase_Alpha;
}

float4 ps_main_bg_decal_texture(const in VS_OUTPUT IN) : COLOR
{
   float4 pixel = tex2D(tex_base_color, IN.tex0);
   clip(pixel.a <= alphaTestValue ? - 1 : 1); // stop the pixel shader if alpha test should reject pixel
   return pixel * cBase_Alpha;
}

//------------------------------------------
// Kicker boolean vertex shader

VS_NOTEX_OUTPUT vs_kicker (const in float4 vPosition : POSITION0,
                           const in float3 vNormal   : NORMAL0,
                           const in float2 tc        : TEXCOORD0)
{
   const float3 P = mul(vPosition, matWorldView).xyz;
   const float3 N = normalize(mul(vNormal, matWorldViewInverseTranspose).xyz);

   VS_NOTEX_OUTPUT Out;
   Out.pos.xyw = mul(vPosition, matWorldViewProj).xyw;
   Out.worldPos = P;
   Out.tablePos = mul(vPosition, matWorld).xyz;
   Out.normal = N;
   // Offset projected position to always render kicker depth "above" playfield (legacy behavior that breaks VR use of the table, was needed since playfield transparency and under playfield objects used to be unsupported)
   float4 P2 = vPosition;
   P2.z -= 30.0;
   Out.pos.z = mul(P2, matWorldViewProj).z;
   return Out;
}

//------------------------------------
// Techniques
//

technique basic_without_texture                   { pass P0 { VertexShader = compile vs_3_0 vs_notex_main(); PixelShader = compile ps_3_0 ps_main(0, 0, 0); } }
technique basic_without_texture_isMetal           { pass P0 { VertexShader = compile vs_3_0 vs_notex_main(); PixelShader  = compile ps_3_0 ps_main(1, 0, 0); } }
technique basic_without_texture_refl              { pass P0 { VertexShader = compile vs_3_0 vs_notex_main(); PixelShader = compile ps_3_0 ps_main(0, 1, 0); } }
technique basic_without_texture_refl_isMetal      { pass P0 { VertexShader = compile vs_3_0 vs_notex_main(); PixelShader = compile ps_3_0 ps_main(1, 1, 0); } }
technique basic_without_texture_refr              { pass P0 { VertexShader = compile vs_3_0 vs_notex_main(); PixelShader = compile ps_3_0 ps_main(0, 0, 1); } }
technique basic_without_texture_refr_isMetal      { pass P0 { VertexShader = compile vs_3_0 vs_notex_main(); PixelShader = compile ps_3_0 ps_main(1, 0, 1); } }
technique basic_without_texture_refr_refl         { pass P0 { VertexShader = compile vs_3_0 vs_notex_main(); PixelShader = compile ps_3_0 ps_main(0, 1, 1); } }
technique basic_without_texture_refr_refl_isMetal { pass P0 { VertexShader = compile vs_3_0 vs_notex_main(); PixelShader = compile ps_3_0 ps_main(1, 1, 1); } }

technique basic_with_texture                          { pass P0 { VertexShader = compile vs_3_0 vs_main(); PixelShader = compile ps_3_0 ps_main_texture(0, 0, 0, 0, 0); } }
technique basic_with_texture_isMetal                  { pass P0 { VertexShader = compile vs_3_0 vs_main(); PixelShader = compile ps_3_0 ps_main_texture(0, 1, 0, 0, 0); } }
technique basic_with_texture_normal                   { pass P0 { VertexShader = compile vs_3_0 vs_main(); PixelShader = compile ps_3_0 ps_main_texture(0, 0, 1, 0, 0); } }
technique basic_with_texture_normal_isMetal           { pass P0 { VertexShader = compile vs_3_0 vs_main(); PixelShader = compile ps_3_0 ps_main_texture(0, 1, 1, 0, 0); } }
technique basic_with_texture_refl                     { pass P0 { VertexShader = compile vs_3_0 vs_main(); PixelShader = compile ps_3_0 ps_main_texture(0, 0, 0, 1, 0); } }
technique basic_with_texture_refl_isMetal             { pass P0 { VertexShader = compile vs_3_0 vs_main(); PixelShader = compile ps_3_0 ps_main_texture(0, 1, 0, 1, 0); } }
technique basic_with_texture_refl_normal              { pass P0 { VertexShader = compile vs_3_0 vs_main(); PixelShader = compile ps_3_0 ps_main_texture(0, 0, 1, 1, 0); } }
technique basic_with_texture_refl_normal_isMetal      { pass P0 { VertexShader = compile vs_3_0 vs_main(); PixelShader = compile ps_3_0 ps_main_texture(0, 1, 1, 1, 0); } }
technique basic_with_texture_refr                     { pass P0 { VertexShader = compile vs_3_0 vs_main(); PixelShader = compile ps_3_0 ps_main_texture(0, 0, 0, 0, 1); } }
technique basic_with_texture_refr_isMetal             { pass P0 { VertexShader = compile vs_3_0 vs_main(); PixelShader = compile ps_3_0 ps_main_texture(0, 1, 0, 0, 1); } }
technique basic_with_texture_refr_normal              { pass P0 { VertexShader = compile vs_3_0 vs_main(); PixelShader = compile ps_3_0 ps_main_texture(0, 0, 1, 0, 1); } }
technique basic_with_texture_refr_normal_isMetal      { pass P0 { VertexShader = compile vs_3_0 vs_main(); PixelShader = compile ps_3_0 ps_main_texture(0, 1, 1, 0, 1); } }
technique basic_with_texture_refr_refl                { pass P0 { VertexShader = compile vs_3_0 vs_main(); PixelShader = compile ps_3_0 ps_main_texture(0, 0, 0, 1, 1); } }
technique basic_with_texture_refr_refl_isMetal        { pass P0 { VertexShader = compile vs_3_0 vs_main(); PixelShader = compile ps_3_0 ps_main_texture(0, 1, 0, 1, 1); } }
technique basic_with_texture_refr_refl_normal         { pass P0 { VertexShader = compile vs_3_0 vs_main(); PixelShader = compile ps_3_0 ps_main_texture(0, 0, 1, 1, 1); } }
technique basic_with_texture_refr_refl_normal_isMetal { pass P0 { VertexShader = compile vs_3_0 vs_main(); PixelShader = compile ps_3_0 ps_main_texture(0, 1, 1, 1, 1); } }

technique basic_with_texture_at                          { pass P0 { VertexShader = compile vs_3_0 vs_main(); PixelShader = compile ps_3_0 ps_main_texture(1, 0, 0, 0, 0); } }
technique basic_with_texture_at_isMetal                  { pass P0 { VertexShader = compile vs_3_0 vs_main(); PixelShader = compile ps_3_0 ps_main_texture(1, 1, 0, 0, 0); } }
technique basic_with_texture_at_normal                   { pass P0 { VertexShader = compile vs_3_0 vs_main(); PixelShader = compile ps_3_0 ps_main_texture(1, 0, 1, 0, 0); } }
technique basic_with_texture_at_normal_isMetal           { pass P0 { VertexShader = compile vs_3_0 vs_main(); PixelShader = compile ps_3_0 ps_main_texture(1, 1, 1, 0, 0); } }
technique basic_with_texture_at_refl                     { pass P0 { VertexShader = compile vs_3_0 vs_main(); PixelShader = compile ps_3_0 ps_main_texture(1, 0, 0, 1, 0); } }
technique basic_with_texture_at_refl_isMetal             { pass P0 { VertexShader = compile vs_3_0 vs_main(); PixelShader = compile ps_3_0 ps_main_texture(1, 1, 0, 1, 0); } }
technique basic_with_texture_at_refl_normal              { pass P0 { VertexShader = compile vs_3_0 vs_main(); PixelShader = compile ps_3_0 ps_main_texture(1, 0, 1, 1, 0); } }
technique basic_with_texture_at_refl_normal_isMetal      { pass P0 { VertexShader = compile vs_3_0 vs_main(); PixelShader = compile ps_3_0 ps_main_texture(1, 1, 1, 1, 0); } }
technique basic_with_texture_at_refr                     { pass P0 { VertexShader = compile vs_3_0 vs_main(); PixelShader = compile ps_3_0 ps_main_texture(1, 0, 0, 0, 1); } }
technique basic_with_texture_at_refr_isMetal             { pass P0 { VertexShader = compile vs_3_0 vs_main(); PixelShader = compile ps_3_0 ps_main_texture(1, 1, 0, 0, 1); } }
technique basic_with_texture_at_refr_normal              { pass P0 { VertexShader = compile vs_3_0 vs_main(); PixelShader = compile ps_3_0 ps_main_texture(1, 0, 1, 0, 1); } }
technique basic_with_texture_at_refr_normal_isMetal      { pass P0 { VertexShader = compile vs_3_0 vs_main(); PixelShader = compile ps_3_0 ps_main_texture(1, 1, 1, 0, 1); } }
technique basic_with_texture_at_refr_refl                { pass P0 { VertexShader = compile vs_3_0 vs_main(); PixelShader = compile ps_3_0 ps_main_texture(1, 0, 0, 1, 1); } }
technique basic_with_texture_at_refr_refl_isMetal        { pass P0 { VertexShader = compile vs_3_0 vs_main(); PixelShader = compile ps_3_0 ps_main_texture(1, 1, 0, 1, 1); } }
technique basic_with_texture_at_refr_refl_normal         { pass P0 { VertexShader = compile vs_3_0 vs_main(); PixelShader = compile ps_3_0 ps_main_texture(1, 0, 1, 1, 1); } }
technique basic_with_texture_at_refr_refl_normal_isMetal { pass P0 { VertexShader = compile vs_3_0 vs_main(); PixelShader = compile ps_3_0 ps_main_texture(1, 1, 1, 1, 1); } }

technique basic_reflection_only { pass P0 { VertexShader = compile vs_3_0 vs_notex_main(); PixelShader = compile ps_3_0 ps_main_reflection_only(); } }

technique unshaded_without_texture { pass P0 { VertexShader = compile vs_3_0 vs_depth_only_main_without_texture(); PixelShader  = compile ps_3_0 ps_main_unshaded_without_texture(); } }
technique unshaded_with_texture { pass P0 { VertexShader = compile vs_3_0 vs_depth_only_main_with_texture(); PixelShader  = compile ps_3_0 ps_main_unshaded_with_texture(); } }
technique unshaded_without_texture_shadow { pass P0 { VertexShader = compile vs_3_0 vs_main_unshaded_without_texture_shadow(); PixelShader  = compile ps_3_0 ps_main_unshaded_without_texture_shadow(); } }
technique unshaded_with_texture_shadow { pass P0 { VertexShader = compile vs_3_0 vs_main_unshaded_with_texture_shadow(); PixelShader  = compile ps_3_0 ps_main_unshaded_with_texture_shadow(); } }

technique bg_decal_without_texture { pass P0 { VertexShader = compile vs_3_0 vs_notex_main(); PixelShader  = compile ps_3_0 ps_main_bg_decal(); } }
technique bg_decal_with_texture { pass P0 { VertexShader = compile vs_3_0 vs_main(); PixelShader  = compile ps_3_0 ps_main_bg_decal_texture(); } }

technique kickerBoolean_isMetal { pass P0 { VertexShader = compile vs_3_0 vs_kicker(); PixelShader = compile ps_3_0 ps_main(1, 0, 0); } }
technique kickerBoolean { pass P0 { VertexShader = compile vs_3_0 vs_kicker(); PixelShader = compile ps_3_0 ps_main(0, 0, 0); } }

#include "ClassicLightShader.hlsl"
