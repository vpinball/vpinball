// license:GPLv3+

#ifdef STEREO
$input v_texcoord0, v_eye
#else
$input v_texcoord0
#endif

#include "common.sh"


// w_h_height.xy contains inverse size of source texture (1/w, 1/h), i.e. one texel shift to the upper (DX)/lower (OpenGL) left texel. Since OpenGL has upside down textures it leads to a different texel if not sampled on both sides
// . for bloom, w_h_height.z keeps strength
// . for mirror, w_h_height.z keeps inverse strength
// . for AO, w_h_height.zw keeps per-frame offset for temporal variation of the pattern
// . for AA techniques, w_h_height.z keeps source texture width, w_h_height.w is a boolean set to 1 when depth is available
// . for parallax stereo, w_h_height.z keeps source texture height, w_h_height.w keeps the 3D offset
uniform vec4 w_h_height;

SAMPLER2DSTEREO(tex_fb_filtered,  0); // Framebuffer (filtered)


// For reference, see https://catlikecoding.com/unity/tutorials/advanced-rendering/bloom/
vec3 bloom_cutoff(const vec3 c)
{
    const float Threshold = 2.5; //!! magic
    const float SoftThreshold = 1.0;
    const float Knee = Threshold * SoftThreshold;

    const float brightness = max(c.r, max(c.g, c.b));
    float soft = brightness - (Threshold - Knee);
    soft = clamp(soft, 0., 2. * Knee);
    soft *= soft * (1.0 / (4. * Knee + 0.00001));
    const float contribution = max(soft, brightness - Threshold) / max(brightness, 0.00001);
    return c * contribution;
}

void main()
{
    // collect clipped contribution of the 4x4 texels (via box blur, NOT gaussian, as this is wrong) from original FB
    const vec3 result = (texStereoNoLod(tex_fb_filtered, v_texcoord0 - w_h_height.xy).rgb
                      +  texStereoNoLod(tex_fb_filtered, v_texcoord0 + w_h_height.xy).rgb
                      +  texStereoNoLod(tex_fb_filtered, v_texcoord0 + vec2(w_h_height.x,-w_h_height.y)).rgb
                      +  texStereoNoLod(tex_fb_filtered, v_texcoord0 + vec2(-w_h_height.x,w_h_height.y)).rgb)*0.25; //!! offset for useAA?
    gl_FragColor = vec4(max(bloom_cutoff(FBToneMap(result)), vec3_splat(0.)) * w_h_height.z, 1.0);
}
