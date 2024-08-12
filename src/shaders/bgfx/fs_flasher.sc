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

   BRANCH if (amount_blend_modulate_vs_add_flasherMode.z < 2.) // Mode 0 & 1
   {
      pixel1 = texture2D(tex_flasher_A, v_texcoord0);
      if (pixel1.a <= alphaTestValueAB_filterMode_addBlend.x)
	  {
		gl_FragColor = vec4_splat(0.0);
		return;
	  }
   }
   BRANCH if (amount_blend_modulate_vs_add_flasherMode.z == 1.)
   {
      pixel2 = texture2D(tex_flasher_B, v_texcoord0);
      if (pixel2.a <= alphaTestValueAB_filterMode_addBlend.y)
	  {
		gl_FragColor = vec4_splat(0.0);
		return;
	  }
   }

   vec4 result = staticColor_Alpha; // Mode 2 wires this through

   if (amount_blend_modulate_vs_add_flasherMode.z == 0.) // Mode 0 mods it by Texture
   {
      result *= pixel1;
   }

   BRANCH if (amount_blend_modulate_vs_add_flasherMode.z == 1.) // Mode 1 allows blends between Tex 1 & 2, and then mods the staticColor with it
   {
      BRANCH if (alphaTestValueAB_filterMode_addBlend.z == Filter_Overlay)
         result *= OverlayHDR(pixel1,pixel2); // could be HDR
      if (alphaTestValueAB_filterMode_addBlend.z == Filter_Multiply)
         result *= Multiply(pixel1,pixel2, amount_blend_modulate_vs_add_flasherMode.x);
      if (alphaTestValueAB_filterMode_addBlend.z == Filter_Additive)
         result *= Additive(pixel1,pixel2, amount_blend_modulate_vs_add_flasherMode.x);
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
   else
      gl_FragColor = vec4(result.xyz*(-amount_blend_modulate_vs_add_flasherMode.y*result.a), // negative as it will be blended with '1.0-thisvalue' (the 1.0 is needed to modulate the underlying elements correctly, but not wanted for the term below)
                     1.0/amount_blend_modulate_vs_add_flasherMode.y - 1.0);
}
