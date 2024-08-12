// license:GPLv3+

#ifdef STEREO
$input v_texcoord0, v_eye
#else
$input v_texcoord0
#endif

#include "common.sh"

SAMPLER2D(tex_env,  0); // envmap

// Compute Van der Corput radical inverse
// See: http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
float radicalInverse_VdC(uint bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits >> 8) * 0.000000059604644775390625;
	//return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

void main()
{
   // Compute sample direction
   const float phi   = PI + v_texcoord0.x * (2.0 * PI);
   const float theta =      v_texcoord0.y        * PI;
   const vec3 N = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));

   // Monte Carlo integration of hemispherical radiance -> irradiance
   const int NumSamples = 32768; // Limit to 32768 or 65536, will fail on macOS Metal otherwise
   const float InvNumSamples = 1.0 / float(NumSamples);
   const float g = 15936.0 / float(NumSamples); // 25962 = matching rank-1 generator constant for 65536 samples, 15936 for 32768 samples
   vec3 irradiance = vec3_splat(0.);
   for (int i=0; i<NumSamples; ++i) {
      //const vec2 u = vec2(float(i) * InvNumSamples, radicalInverse_VdC(i));
      const vec2 u = vec2(float(i) * InvNumSamples, fract(float(i) * g));
      const vec3 Li = rotate_to_vector_upper(cos_hemisphere_sample(u), N);
      const vec2 uv = ray_to_equirectangular_uv(Li);
      irradiance += texNoLod(tex_env, uv).rgb * InvNumSamples;
   }
   gl_FragColor = vec4(irradiance, 1.0);
}
