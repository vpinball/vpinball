vec2  v_texcoord0     : TEXCOORD0 = vec2(0.0, 0.0);
vec3  v_tablePos      : TEXCOORD1 = vec3(0.0, 0.0, 0.0);
vec3  v_worldPos      : TEXCOORD2 = vec3(0.0, 0.0, 0.0);
float v_clipDistance  : TEXCOORD3 = float(0.0);
float v_eye           : TEXCOORD4 = float(0.0);
vec2  v_texcoord1     : TEXCOORD1 = vec2(0.0, 0.0);
vec4  v_texcoord2     : TEXCOORD2 = vec4(0.0, 0.0, 0.0, 0.0);
vec4  v_texcoord3     : TEXCOORD4 = vec4(0.0, 0.0, 0.0, 0.0);
vec4  v_texcoord4     : TEXCOORD5 = vec4(0.0, 0.0, 0.0, 0.0);
vec3  v_normal        : NORMAL    = vec3(0.0, 0.0, 1.0);
vec4  v_color0        : COLOR0    = vec4(1.0, 0.0, 0.0, 1.0);

vec3 a_position  : POSITION;
vec3 a_normal    : NORMAL;
vec2 a_texcoord0 : TEXCOORD0;
vec4 a_color0    : COLOR0;

// license:GPLv3+
