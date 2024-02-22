$input v_worldPos, v_tablePos, v_normal, v_texcoord0

#include "bgfx_shader.sh"


#define NUM_BALL_LIGHTS 0 // just to avoid having too much constant mem allocated

#ifdef TEX
SAMPLER2D(tex_base_color, 0); // base color
#endif
SAMPLER2D(tex_env, 1); // envmap
SAMPLER2D(tex_diffuse_env, 2); // envmap radiance
SAMPLER2D(tex_base_transmission, 3); // bulb light buffer
SAMPLER2D(tex_base_normalmap, 4); // normal map
SAMPLER2D(tex_reflection, 5); // reflections
SAMPLER2D(tex_refraction, 6); // refractions
SAMPLER2D(tex_probe_depth, 7); // depth probe

uniform mat4 matWorldViewProj;
uniform mat4 matWorldView;
uniform mat4 matWorldViewInverseTranspose;
uniform mat4 matWorld;
uniform mat4 matView;
uniform mat4 matProj;

uniform vec4 objectSpaceNormalMap; // float extended to vec4 for BGFX FIXME float uniforms are not supported: group or declare as vec4

uniform vec4 u_basic_shade_mode;
#define doMetal       	    (u_basic_shade_mode.x)
#define doNormalMapping     (u_basic_shade_mode.y)
#define doRefractions       (u_basic_shade_mode.w)

uniform float4 refractionTint_thickness;
#define refractionTint (refractionTint_thickness.rgb)
#define refractionThickness (refractionTint_thickness.w)

uniform vec4 cClearcoat_EdgeAlpha;
uniform vec4 cGlossy_ImageLerp;
//!! No value is under 0.02
//!! Non-metals value are un-intuitively low: 0.02-0.08
//!! Gemstones are 0.05-0.17
//!! Metals have high specular reflectance: 0.5-1.0

uniform vec4 staticColor_Alpha;
uniform vec4 alphaTestValue; // Actually float but extended to vec4 for BGFX (and we should use the builtin)

uniform vec4 w_h_height;

uniform float4 mirrorNormal_factor;
#define mirrorNormal (mirrorNormal_factor.xyz)
#define mirrorFactor (mirrorNormal_factor.w)
#define doReflections (mirrorNormal_factor.w > 0.0)

uniform vec4 lightCenter_doShadow;

#include "common.sh"
#include "material.sh"
#include "ball_shadows.sh"

mat3 TBN_trafo(const vec3 N, const vec3 V, const vec2 uv)
{
   // derivatives: edge vectors for tri-pos and tri-uv
   const vec3 dpx = dFdx(V);
   const vec3 dpy = dFdy(V);
   const vec2 duvx = dFdx(uv);
   const vec2 duvy = dFdy(uv);

   // solve linear system
   const vec3 dp2perp = cross(N, dpy);
   const vec3 dp1perp = cross(dpx, N);
   const vec3 T = dp2perp * duvx.x + dp1perp * duvy.x;
   const vec3 B = dp2perp * duvx.y + dp1perp * duvy.y;

   // construct scale-invariant transformation
   return mat3(T, B, N * sqrt( max(dot(T,T), dot(B,B)) )); // inverse scale, as will be normalized anyhow later-on (to save some mul's)
}

vec3 normal_map(const vec3 N, const vec3 V, const vec2 uv)
{
   vec3 tn = texture2D(tex_base_normalmap, uv).xyz * (255./127.) - (128./127.); // Note that Blender apparently does export tangent space normalmaps for Z (Blue) at full range, so 0..255 -> 0..1, which misses an option for here!

   BRANCH if (objectSpaceNormalMap.x != 0.0)
   {
      tn.z = -tn.z; // this matches the object space, +X +Y +Z, export/baking in Blender with our trafo setup
      return normalize( mul(tn, matWorldViewInverseTranspose).xyz );
      // return normalize(mul(inverse(matView), vec4(tn, 0.0)).xyz);
   } else // tangent space
      return normalize(( mul(tn, TBN_trafo(N, V, uv)) ).xyz);  // !! The order of tn, and TBN_trainfo matters here?  Ship on FishTales gets blown out if you reverse them 
}

// Compute reflections from reflection probe (screen space coordinates)
// This is a simplified reflection model where reflected light is added instead of being mixed according to the fresnel coefficient (stronger reflection hiding the object's color at grazing angles)
vec3 compute_reflection(const vec2 screenSpace, const vec3 N)
{
   // Only apply to faces pointing in the direction of the probe (normal = [0,0,-1])
   // the smoothstep values are *magic* values taken from visual tests
   // dot(mirrorNormal, N) does not really needs to be done per pixel and could be moved to the vertx shader
   // Offset by half a texel to use GPU filtering for some blur
   return smoothstep(0.5, 0.9, dot(mirrorNormal.xyz, N)) * mirrorFactor.x * texture2D(tex_reflection, screenSpace + 0.5 * w_h_height.xy).rgb;
}

// Compute refractions from screen space probe
#ifdef DX9
vec3 compute_refraction(const vec3 pos, const vec2 screenSpace, const vec3 N, const vec3 V)
#else
vec3 compute_refraction(const vec3 pos, const vec2 screenSpace, const vec3 N, const vec3 V, const vec4 fragCoord)
#endif
{
   // Compute refracted visible position then project from world view position to probe UV
   // const vec4x4 matProj = mul(inverse4x4(matWorldView), matWorldViewProj[0]); // this has been moved to the matrix uniform stack for performance reasons
   const vec3 R = refract(V, N, 1.0 / 1.5); // n1 = 1.0 (air), n2 = 1.5 (plastic), eta = n1 / n2
   const vec3 refracted_pos = pos + refractionThickness.x * R; // Shift ray by the thickness of the material
   const vec4 proj = mul(matProj, vec4(refracted_pos, 1.0));
   vec2 uv = vec2(0.5, 0.5) + proj.xy * (0.5 / proj.w);

   // Check if the sample position is behind the object pos. If not take don't perform refraction as it would lead to refract things above us (so reflect)
   const float d = texture2D(tex_probe_depth, uv).x;

#ifdef DX9
   // Sadly DX9 does not give access to transformed fragment position and we need to project it again here...
   const vec4 proj_base = mul(vec4(pos, 1.0), matProj); 
   if (d < proj_base.z / proj_base.w)
      uv = screenSpace;
#else
   if (d < fragCoord.z)
      uv = screenSpace;
#endif
   // The following code gives a smoother transition but depends too much on the POV since it uses homogeneous depth to lerp instead of fragment's world depth
   //const vec3 unbiased = vec3(1.0, 0.0, 0.0);
   //const vec3 biased = vec3(0.0, 1.0, 0.0);
   //const vec3 unbiased = texture2D(tex_refraction, screenSpace * w_h_height.xy).rgb;
   //const vec3 biased = texture2D(tex_refraction, uv).rgb;
   //return mix(unbiased, biased, saturate((d - fragCoord.z) / fragCoord.w));

   /* // Debug output
   if (length(N) < 0.5) // invalid normal, shown as red for debugging
      return vec3(1.0, 0.0, 0.0);
   if (dot(N, V) < 0.0) // Wrong orientation? (looking from the back, shown as blue for debugging)
      return vec3(0.0, 0.0, 1.0);
   if (length(R) < 0.5) // invalid refraction state (no refraction, shown as green for debugging)
      return vec3(0.0, 1.0, 0.0);*/

   return refractionTint.rgb * texture2D(tex_refraction, uv).rgb;
}

EARLY_DEPTH_STENCIL void main() {
#ifdef REFL
   vec4 color;
   vec3 N = normalize(v_normal);
   color.rgb = compute_reflection(gl_FragCoord.xy * w_h_height.xy, N);

#ifdef TEX
   color.a = texture2D(tex_base_color, v_texcoord0).a;
   
   if (color.a <= alphaTestValue.x)
      discard;           //stop the pixel shader if alpha test should reject pixel
	  
   color.a *= cBase_Alpha.a;
   
   // Maybe overkill for just the additive reflections
   BRANCH if (cBase_Alpha.a < 1.0 && color.a < 1.0)
   {
      const vec3 V = normalize(/*camera=0,0,0,1*/ -v_worldPos);
      color.a = GeometricOpacity(dot(N, V), color.a, cClearcoat_EdgeAlpha.w, Roughness_WrapL_Edge_Thickness.w);
   }
#else
   color.a = 1.0;
#endif

#else
#ifdef TEX
   vec4 pixel = texture2D(tex_base_color, v_texcoord0);

   if (pixel.a <= alphaTestValue.x)
      discard;           //stop the pixel shader if alpha test should reject pixel

   if (fDisableLighting_top_below.x < 1.0) // if there is lighting applied, make sure to clamp the values (as it could be coming from a HDR tex)
      pixel.xyz = clamp(pixel.xyz, vec3(0.0,0.0,0.0), vec3(1.0,1.0,1.0));

   const vec3 t = pixel.xyz;

   const vec3 diffuse  = t*cBase_Alpha.xyz;
   const vec3 glossy   = doMetal != 0.0 ? diffuse : (t*cGlossy_ImageLerp.w + (1.0-cGlossy_ImageLerp.w))*cGlossy_ImageLerp.xyz*0.08; //!! use AO for glossy? specular?
   const vec3 specular = cClearcoat_EdgeAlpha.xyz*0.08;
   const float  edge     = doMetal != 0.0 ? 1.0 : Roughness_WrapL_Edge_Thickness.z;
#else
   const vec3 diffuse  = cBase_Alpha.xyz;
   const vec3 glossy   = doMetal != 0.0 ? cBase_Alpha.xyz : cGlossy_ImageLerp.xyz*0.08;
   const vec3 specular = cClearcoat_EdgeAlpha.xyz*0.08;
   const float  edge     = doMetal != 0.0 ? 1.0 : Roughness_WrapL_Edge_Thickness.z;
#endif

   const vec3 V = normalize(-v_worldPos);

#ifdef TEX
   vec3 N = normalize(v_normal);
   BRANCH if (doNormalMapping != 0.0)
      N = normal_map(N, normalize(v_worldPos), v_texcoord0);
#else
   const vec3 N = normalize(v_normal);
#endif

   //color = vec4((N+1.0)*0.5,1.0); return; // visualize normals

   vec4 color = vec4(
      lightLoop(v_worldPos, N, V, diffuse, glossy, specular, edge, 1.0, doMetal != 0.0), //!! have a "real" view vector instead that mustn't assume that viewer is directly in front of monitor? (e.g. cab setup) -> viewer is always relative to playfield and/or user definable
      cBase_Alpha.a);
#ifdef TEX
   color.a = saturate(pixel.a * cBase_Alpha.a);
#endif

   BRANCH if (cBase_Alpha.a < 1.0)
   {
      color.a = GeometricOpacity(dot(N,V), color.a,cClearcoat_EdgeAlpha.w,Roughness_WrapL_Edge_Thickness.w);

      if (fDisableLighting_top_below.y < 1.0)
         // add light from "below" from user-flagged bulb lights, pre-rendered/blurred in previous renderpass //!! sqrt = magic
         color.xyz += mix(sqrt(diffuse)*texture2DLod(tex_base_transmission, gl_FragCoord.xy * w_h_height.xy, 0.0).rgb*color.a, vec3_splat(0.), fDisableLighting_top_below.y); //!! depend on normal of light (unknown though) vs geom normal, too?
   }

   BRANCH if (lightCenter_doShadow.w != 0.)
   {
      const vec3 light_dir = v_tablePos.xyz - lightCenter_doShadow.xyz;
      const float light_dist = length(light_dir);
      const float shadow = get_light_ball_shadow(lightCenter_doShadow.xyz, light_dir, light_dist);
      color.rgb *= shadow;
   }

   BRANCH if (doReflections != 0.0)
      color.rgb += compute_reflection(gl_FragCoord.xy * w_h_height.xy, N);

   BRANCH if (doRefractions != 0.0)
   {
      // alpha channel is the transparency of the object, tinting is supported even if alpha is 0 by applying a tint color
#ifdef DX9
      color.rgb = mix(compute_refraction(v_worldPos.xyz, gl_FragCoord.xy * w_h_height.xy, N, V), color.rgb, cBase_Alpha.a);
#else
      color.rgb = mix(compute_refraction(v_worldPos.xyz, gl_FragCoord.xy * w_h_height.xy, N, V, gl_FragCoord), color.rgb, cBase_Alpha.a);
#endif
      color.a = 1.0;
   }
#endif

   gl_FragColor = color * staticColor_Alpha;
}
