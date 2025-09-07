$input v_color0, v_texcoord0, v_texcoord1

#include "common.sh"

SAMPLER2D      (tex_base_color, 0); // base color

uniform vec4 clip_plane; // Not a plane for UI, but reuse the existing uniform

void main()
{
    if (any(lessThan(v_texcoord1.xy, clip_plane.xy)) || any(greaterThan(v_texcoord1.xy, clip_plane.zw)))
        discard;
    gl_FragColor = texture2D(tex_base_color, v_texcoord0) * v_color0;
}
