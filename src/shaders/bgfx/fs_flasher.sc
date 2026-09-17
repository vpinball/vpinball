// license:GPLv3+

$input v_tablePos, v_texcoord0
#ifdef CLIP
	$input v_clipDistance
#endif

#include "common.sh"
#include "ball_shadows.sh"

#define Filter_None     0.
#define Filter_Additive 1.
#define Filter_Overlay  2.
#define Filter_Multiply 3.
#define Filter_Screen   4.

uniform vec4 staticColor_Alpha;
uniform vec4 alphaTestValueAB_filterMode_addBlend; // last one bool
uniform vec4 amount_blend_modulate_vs_add_flasherMode; // vec3 extended to vec4 for BGFX
#define blendAmount (amount_blend_modulate_vs_add_flasherMode.x)
#define modulateVsAdd (amount_blend_modulate_vs_add_flasherMode.y)
#define flasherMode (amount_blend_modulate_vs_add_flasherMode.z)

uniform vec4 lightCenter_doShadow;

SAMPLER2D(tex_flasher_A, 0); // base texture
SAMPLER2D(tex_flasher_B, 1); // second image

// Flasher don't write to depth buffer, so they can have EARLY_DEPTH_STENCIL even when using discard for clip plane
EARLY_DEPTH_STENCIL void main()
{
   #ifdef CLIP
   if (v_clipDistance < 0.0)
      discard;
   #endif

   vec4 pixel1,pixel2;

   BRANCH if (flasherMode < 2.) // Mode 0 & 1
   {
      pixel1 = texture2D(tex_flasher_A, v_texcoord0);
      if (pixel1.a <= alphaTestValueAB_filterMode_addBlend.x)
      {
         gl_FragColor = vec4_splat(0.0);
         return;
      }
   }
   BRANCH if (flasherMode == 1.)
   {
      pixel2 = texture2D(tex_flasher_B, v_texcoord0);
      if (pixel2.a <= alphaTestValueAB_filterMode_addBlend.y)
      {
         gl_FragColor = vec4_splat(0.0);
         return;
      }
   }

   vec4 result = staticColor_Alpha; // Mode 2 wires this through

   if (flasherMode == 0.) // Mode 0 mods it by Texture
   {
      result *= pixel1;
   }

   BRANCH if (flasherMode == 1.) // Mode 1 allows blends between Tex 1 & 2, and then mods the staticColor with it
   {
      BRANCH if (alphaTestValueAB_filterMode_addBlend.z == Filter_Overlay)
         result *= OverlayHDR(pixel1,pixel2); // could be HDR
      if (alphaTestValueAB_filterMode_addBlend.z == Filter_Multiply)
         result *= Multiply(pixel1,pixel2, blendAmount);
      if (alphaTestValueAB_filterMode_addBlend.z == Filter_Additive)
         result *= Additive(pixel1,pixel2, blendAmount);
      if (alphaTestValueAB_filterMode_addBlend.z == Filter_Screen)
         result *= ScreenHDR(pixel1,pixel2); // could be HDR
   }

   BRANCH if (lightCenter_doShadow.w != 0.)
   {
      const vec3 light_dir = v_tablePos.xyz - lightCenter_doShadow.xyz;
      const float light_dist = length(light_dir);
      const float shadow = get_light_ball_shadow(lightCenter_doShadow.xyz, light_dir, light_dist);
      result.rgb *= shadow;
   }

   if (alphaTestValueAB_filterMode_addBlend.w == 0.)
      gl_FragColor = vec4(result.xyz, saturate(result.a)); // Need to clamp here or we get some saturation artifacts on some tables
   else if (modulateVsAdd > 0.) // AB_ADD
   {
      // Blend unit set to dst' = dst * (1 - src) - src * srcAlpha. Emitting a negative color (negative as it gets
      // blended with '1.0-thisvalue', the 1.0 being what modulates the underlying elements, and unwanted for the
      // term below) along a 1/m - 1 alpha gives the historical additive behavior dst' = dst + L * (1 - m * (1 - dst)),
      // for L = result.rgb * result.a the emitted light: the light is added, and what is behind the flasher is
      // amplified on top of it, the more so the brighter the flasher is
      gl_FragColor = vec4(result.xyz*(-modulateVsAdd*result.a), 1.0/modulateVsAdd - 1.0);
   }
   else // AB_ABSORB, which a negative 'modulate vs add' selects
   {
      // Blend unit set to the premultiplied alpha 'over' dst' = src + dst * (1 - srcAlpha). Emitting the
      // premultiplied light along an m * coverage alpha gives dst' = L + dst * (1 - m * result.a): the very same
      // light added, but over a background the flasher now absorbs instead of amplifying, and independently of how
      // bright it is. That is a glass like reflection, m being how reflective (hence opaque) the glass is. The
      // flasher alpha still drives the coverage, so its shape and its edges fade the absorption out as well.
      // Saturated since opacity may go above 100%: the added light keeps that boost, but absorbing more than all of
      // the background would take it negative, inverting it instead of blackening it
      gl_FragColor = vec4(result.xyz*result.a, saturate(-modulateVsAdd*result.a));
   }
}
