// license:GPLv3+

$input v_color0, v_texcoord0, v_texcoord1

#include "common.sh"

SAMPLER2D      (tex_base_color, 0); // base color

uniform vec4 clip_plane; // Not a plane for UI, but reuse the existing uniform

uniform vec4 staticColor_Alpha;
#define isSRGBTexture (staticColor_Alpha.y != 0.0) // Texture is decoded from sRGB when sampled (table images)
#define isLinearFB    (staticColor_Alpha.z != 0.0) // Target expects linear values (in-scene VR UI, drawn on the linear XR projection surface)
#define forceOpaque   (staticColor_Alpha.x != 0.0) // Keep the composited surface opaque (in-scene VR UI)
#define sdrWhitePQ staticColor_Alpha.a // SDR white level, normalized so 1.0 is 10000 nits, or 0 for a sRGB backbuffer

void main()
{
    if (any(lessThan(v_texcoord1.xy, clip_plane.xy)) || any(greaterThan(v_texcoord1.xy, clip_plane.zw)))
        discard;
    vec4 color = texture2D(tex_base_color, v_texcoord0);
    // ImGui works in sRGB: re-encode textures that the sampler decoded to linear
    BRANCH if (isSRGBTexture)
        color.rgb = FBGamma(color.rgb);
    color *= v_color0;
    // UI is drawn after tonemapping, straight into the backbuffer, and ImGui works in sRGB. On a
    // HDR10/BT.2100 backbuffer convert rather than just scale. Note: Blending stays in PQ, closer to the gamma space blending ImGui assumes
    BRANCH if (sdrWhitePQ > 0.0)
        color.rgb = LinearSRGBToPQBT2020(InvGamma(color.rgb) * sdrWhitePQ);
    // The XR projection surface holds linear values (see isLinearFrameBuffer in the tonemap pass): decode ImGui's sRGB
    BRANCH if (isLinearFB)
        color.rgb = InvGamma(color.rgb);
    // The in-scene VR UI is a solid world-space surface: transparent texels are black-backed, not see-through
    BRANCH if (forceOpaque)
        color.a = 1.0;
    gl_FragColor = color;
}
