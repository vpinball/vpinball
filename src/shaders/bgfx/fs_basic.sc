$input v_worldPos, v_tablePos, v_normal, v_texcoord0

#include "common.sh"


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
#define doMetal             (u_basic_shade_mode.x != 0.0)
#define doNormalMapping     (u_basic_shade_mode.y != 0.0)
#define doRefractions       (u_basic_shade_mode.z != 0.0)

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
uniform vec4 alphaTestValue; // FIXME Actually float but extended to vec4 for BGFX (and we should use the builtin)

uniform vec4 w_h_height;

uniform float4 mirrorNormal_factor;
#define mirrorNormal (mirrorNormal_factor.xyz)
#define mirrorFactor (mirrorNormal_factor.w)
#define doReflections (mirrorNormal_factor.w > 0.0)

uniform vec4 lightCenter_doShadow;

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
   const vec3 tn = texture2D(tex_base_normalmap, uv).xyz * (255./127.) - (128./127.);
   BRANCH if (objectSpaceNormalMap.x != 0.0)
   { // Object space: this matches the object space, +X +Y +Z, export/baking in Blender with our trafo setup
      return normalize(mul(matWorldViewInverseTranspose, float3(tn.x, tn.y, -tn.z)).xyz);
   }
   else
   { // Tangent space
      return normalize(mul(TBN_trafo(N, V, uv), tn).xyz);
   }
}

// Compute reflections from reflection probe (screen space coordinates)
// This is a simplified reflection model where reflected light is added instead of being mixed according to the fresnel coefficient (stronger reflection hiding the object's color at grazing angles)
vec3 compute_reflection(const vec2 screenCoord, const vec3 N)
{
   // Only apply to faces pointing in the direction of the probe (normal = [0,0,-1])
   // the smoothstep values are *magic* values taken from visual tests
   // dot(mirrorNormal, N) does not really needs to be done per pixel and could be moved to the vertx shader
   // Offset by half a texel to use GPU filtering for some blur
   return smoothstep(0.5, 0.9, dot(mirrorNormal.xyz, N)) * mirrorFactor * texStereo(tex_reflection, (screenCoord.xy + vec2_splat(0.5)) * w_h_height.xy).rgb;
}

// Compute refractions from screen space probe
vec3 compute_refraction(const vec3 pos, const vec3 screenCoord, const vec3 N, const vec3 V)
{
   // Compute refracted visible position then project from world view position to probe UV
   // const vec4x4 matProj = mul(inverse4x4(matWorldView), matWorldViewProj[int(eye)]); // this has been moved to the matrix uniform stack for performance reasons
   const vec3 R = refract(V, N, 1.0 / 1.5); // n1 = 1.0 (air), n2 = 1.5 (plastic), eta = n1 / n2
   const vec3 refracted_pos = pos + refractionThickness.x * R; // Shift ray by the thickness of the material
   const vec4 proj = mul(matProj, vec4(refracted_pos, 1.0));
   
   vec2 uv = vec2(0.5, 0.5) + proj.xy * (0.5 / proj.w);

   // Check if the sample position is behind the object pos. If not take don't perform refraction as it would lead to refract things above us (so reflect)
   const float d = texStereo(tex_probe_depth, uv).x;
   if (d < screenCoord.z)
      uv = screenCoord.xy * w_h_height.xy;
      
   // The following code gives a smoother transition but depends too much on the POV since it uses homogeneous depth to lerp instead of fragment's world depth
   //const vec3 unbiased = vec3(1.0, 0.0, 0.0);
   //const vec3 biased = vec3(0.0, 1.0, 0.0);
   //const vec3 unbiased = texture2D(tex_refraction, screenCoord.xy).rgb;
   //const vec3 biased = texture2D(tex_refraction, uv).rgb;
   //return mix(unbiased, biased, saturate((d - fragCoord.z) / fragCoord.w));

   /* // Debug output
   if (length(N) < 0.5) // invalid normal, shown as red for debugging
      return vec3(1.0, 0.0, 0.0);
   if (dot(N, V) < 0.0) // Wrong orientation? (looking from the back, shown as blue for debugging)
      return vec3(0.0, 0.0, 1.0);
   if (length(R) < 0.5) // invalid refraction state (no refraction, shown as green for debugging)
      return vec3(0.0, 1.0, 0.0);*/

   return refractionTint.rgb * texStereo(tex_refraction, uv).rgb;
}

#ifdef AT
void main() {
#else
EARLY_DEPTH_STENCIL void main() {
#endif
   vec4 color;

   #ifdef REFL
      // Reflection only pass variant of the basic material shading
      vec3 N = normalize(v_normal);
      color.rgb = compute_reflection(gl_FragCoord.xy, N);
      color.a = 1.0;

   #else
      // Full basic material shading
      #ifdef TEX
         vec4 pixel = texture2D(tex_base_color, v_texcoord0);
         #ifdef AT
            if (pixel.a <= alphaTestValue.x)
               discard; //stop the pixel shader if alpha test should reject pixel
         #endif
         
         pixel.a *= cBase_Alpha.a;
         if (fDisableLighting_top_below.x < 1.0) // if there is lighting applied, make sure to clamp the values (as it could be coming from a HDR tex)
            pixel.rgb = clamp(pixel.rgb, vec3_splat(0.0), vec3_splat(1.0));
         
         const vec3 diffuse  = pixel.rgb * cBase_Alpha.rgb;
         const vec3 glossy   = doMetal ? diffuse : (pixel.rgb * cGlossy_ImageLerp.w + (1.0-cGlossy_ImageLerp.w)) * cGlossy_ImageLerp.rgb * 0.08; //!! use AO for glossy? specular?
      #else
         const vec4 pixel    = cBase_Alpha;
         const vec3 diffuse  = cBase_Alpha.rgb;
         const vec3 glossy   = doMetal ? cBase_Alpha.rgb : cGlossy_ImageLerp.rgb * 0.08;
      #endif
      const vec3 specular = cClearcoat_EdgeAlpha.rgb * 0.08;
      const float  edge   = doMetal ? 1.0 : Roughness_WrapL_Edge_Thickness.z;
      
      const vec3 V = normalize(-v_worldPos);
      #ifdef TEX
         vec3 N = normalize(v_normal);
         BRANCH if (doNormalMapping)
             N = normal_map(N, normalize(v_worldPos), v_texcoord0);
      #else
         const vec3 N = normalize(v_normal);
      #endif

      //color = vec4((N+1.0)*0.5,1.0); return; // visualize normals

      color = float4(lightLoop(v_worldPos, N, V, diffuse, glossy, specular, edge, doMetal), pixel.a);

      BRANCH if (color.a < 1.0) // We may not opacify if we already are opaque
      {
         color.a = GeometricOpacity(dot(N,V), color.a, cClearcoat_EdgeAlpha.w, Roughness_WrapL_Edge_Thickness.w);

         if (fDisableLighting_top_below.y < 1.0)
            // add light from "below" from user-flagged bulb lights, pre-rendered/blurred in previous renderpass //!! sqrt = magic
            color.rgb += mix(sqrt(diffuse)*texture2DLod(tex_base_transmission, gl_FragCoord.xy * w_h_height.xy, 0.0).rgb*color.a, vec3_splat(0.), fDisableLighting_top_below.y); //!! depend on normal of light (unknown though) vs geom normal, too?
      }

      BRANCH if (lightCenter_doShadow.w != 0.)
      {
         const vec3 light_dir = v_tablePos.xyz - lightCenter_doShadow.xyz;
         const float light_dist = length(light_dir);
         const float shadow = get_light_ball_shadow(lightCenter_doShadow.xyz, light_dir, light_dist);
         color.rgb *= shadow;
      }

      BRANCH if (doReflections)
         color.rgb += compute_reflection(gl_FragCoord.xy, N);

      BRANCH if (doRefractions)
      {
         // alpha channel is the transparency of the object, tinting is supported even if alpha is 0 by applying a tint color
         color.rgb = mix(compute_refraction(v_worldPos.xyz, gl_FragCoord.xyz, N, V), color.rgb, cBase_Alpha.a);
         color.a = 1.0;
      }
   #endif

   gl_FragColor = color * staticColor_Alpha;
}
