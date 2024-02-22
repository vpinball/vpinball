$input v_tablePos

#include "bgfx_shader.sh"
#include "ball_shadows.sh"

uniform vec4 lightColor_intensity;
uniform vec4 lightColor2_falloff_power;
uniform vec4 lightCenter_maxRange;
uniform vec4 blend_modulate_vs_add; // single float, passed as vec4 (BGFX does not support float/int/bool uniforms)

void main()
{
   const vec3 light_dir = v_tablePos - lightCenter_maxRange.xyz;
   const float light_dist = length(light_dir);
   const float len = light_dist * lightCenter_maxRange.w;
   const float atten = saturate(pow(1.0 - saturate(len), lightColor2_falloff_power.w));
   const vec3 lcolor = mix(lightColor2_falloff_power.xyz, lightColor_intensity.xyz, sqrt(len));
   gl_FragColor.xyz = lcolor * (-blend_modulate_vs_add.x * atten * lightColor_intensity.w); // negative as it will be blended with '1.0-thisvalue' (the 1.0 is needed to modulate the underlying elements correctly, but not wanted for the term below)
   gl_FragColor.a = 1.0 / blend_modulate_vs_add.x - 1.0; //saturate(atten*lightColor_intensity.w);
#ifdef BALLSHADOW
   gl_FragColor.xyz *= get_light_ball_shadow(lightCenter_maxRange.xyz, light_dir, light_dist);
#endif
}
