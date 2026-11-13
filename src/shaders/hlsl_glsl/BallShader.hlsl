// license:GPLv3+

//!! always adapt to changes from BasicShader.fx
//!! add playfield color, glossy, etc for more precise reflections of playfield

#define NUM_BALL_LIGHTS 8

#include "Helpers.fxh"

// transformation matrices
const float4x4 matWorldViewProj : WORLDVIEWPROJ;
const float4x4 matWorldView     : WORLDVIEW;
const float4x3 matWorldViewInverse;
//const float4x4 matWorldViewInverseTranspose; // matWorldViewInverse used instead and multiplied from other side
const float4x3 matView;
const float4x4 matPrevViewProj; // view-projection the previous frame was rendered with (to reproject it as a reflection probe)
// Raytraced impostor (see Ball::Render)
const float4x4 matProj;       // (flipped) projection
const float4x4 matProjInv;    // inverse (flipped) projection, mapping rendered clip space back to view space
const float4x4 ballViewToObj; // view space to unit sphere space
//const float4x4 matViewInverseInverseTranspose; // matView used instead and multiplied from other side

texture  Texture0; // base texture
texture  Texture3; // decal
texture  Texture4; // playfield

sampler2D tex_ball_color : TEXUNIT0 = sampler_state // base texture
{
    Texture   = (Texture0);
    //MIPFILTER = LINEAR; //!! HACK: not set here as user can choose to override trilinear by anisotropic
    //MAGFILTER = LINEAR;
    //MINFILTER = LINEAR;
    //ADDRESSU  = Wrap; //!! ?
    //ADDRESSV  = Wrap;
    SRGBTexture = true;
};

sampler2D tex_ball_decal : TEXUNIT3 = sampler_state // ball decal
{
    Texture   = (Texture3);
    MIPFILTER = LINEAR;
    MAGFILTER = LINEAR;
    MINFILTER = LINEAR;
    ADDRESSU  = Wrap;
    ADDRESSV  = Wrap;
    SRGBTexture = true;
};

sampler2D tex_ball_playfield : TEXUNIT4 = sampler_state // playfield
{
    Texture   = (Texture4);
    MIPFILTER = LINEAR; //!! ?
    MAGFILTER = LINEAR;
    MINFILTER = LINEAR;
    ADDRESSU  = Clamp; // screen space probe (previous frame)
    ADDRESSV  = Clamp;
    SRGBTexture = true;
};

#include "Material.fxh"

const float4x3 orientation;
const float4   clip_plane;     // clip plane in table coordinate system, tested against the analytic hit point
const float4   invTableRes_reflection;
const float4   w_h_disableLighting;
#define disableLighting (w_h_disableLighting.z != 0.)

//------------------------------------

//application to vertex structure
struct vin
{ 
    float4 position   : POSITION0;
    float3 normal     : NORMAL;
    float2 tex0       : TEXCOORD0;
};

//vertex to pixel shader structure
struct vout
{
    float4 position    : POSITION0;
    float4 normal_t0x  : TEXCOORD0; // tex0 is stored in w of float4s
    float4 worldPos_t0y: TEXCOORD1;
};

//vertex to pixel shader structure of the raytraced impostor quad
struct voutQuad
{
    float4 position : POSITION0;
    // camera ray in view space and in unit sphere space (same ray parameter)
    float3 rayOriginView : TEXCOORD0;
    float3 rayDirView    : TEXCOORD1;
    float3 rayOriginObj  : TEXCOORD2;
    float3 rayDirObj     : TEXCOORD3;
};

//vertex to pixel shader structure
struct voutTrail
{
    float4 position    : POSITION0;
    float3 tex0_alpha  : TEXCOORD0;
};

//------------------------------------
// VERTEX SHADER

vout vsBall( const in vin IN )
{
    // apply spinning and move the ball to it's actual position
    float4 pos = IN.position;
    pos.xyz = mul_w1(pos.xyz, orientation);

    // apply spinning to the normals too to get the sphere mapping effect
    const float3 nspin = mul_w0(IN.normal, orientation);
    // Needs to use a 'normal' matrix, and to normalize since we allow non uniform stretching, therefore matWorldView is not orthonormal
    const float3 normal = normalize(mul(matWorldViewInverse, nspin).xyz); // actually: mul(float4(nspin,0.), matWorldViewInverseTranspose), but optimized to save one matrix

    const float3 p = mul_w1(pos.xyz, matWorldView);

    vout OUT;
    OUT.position = mul(pos, matWorldViewProj);
    OUT.normal_t0x = float4(normal,IN.tex0.x);
    OUT.worldPos_t0y = float4(p,IN.tex0.y);
    return OUT;
}

voutQuad vsBallQuad( const in vin IN )
{
    // IN.position is a clip space corner of the impostor quad (see Ball::Render), at the nearest projected ball depth
    voutQuad OUT;
    OUT.position = float4(IN.position.xyz, 1.0);
    // Ray through the corner, between its unprojected points at clip depth 0 (the origin) and 1. The projection center is
    // not assumed to be the view space origin, which it is not with scene scale, layback, stereo eye shift or orthographic
    // views. The unprojected w only depends on the clip depth, so the points are linear across the quad and interpolate
    // exactly. The ray is also expressed in unit sphere space (affine, so with the same ray parameter)
    const float4 p0 = mul(float4(IN.position.xy, 0.0, 1.0), matProjInv);
    const float4 p1 = mul(float4(IN.position.xy, 1.0, 1.0), matProjInv);
    OUT.rayOriginView = p0.xyz / p0.w;
    OUT.rayDirView = p1.xyz / p1.w - OUT.rayOriginView;
    OUT.rayOriginObj = mul(float4(OUT.rayOriginView, 1.0), ballViewToObj).xyz;
    OUT.rayDirObj = mul(float4(OUT.rayDirView, 0.0), ballViewToObj).xyz;
    return OUT;
}

voutTrail vsBallTrail( const in vin IN )
{
    voutTrail OUT;
    OUT.position = mul(IN.position, matWorldViewProj);
    OUT.tex0_alpha = float3(IN.tex0, IN.normal.x); //!! abuses normal for now

    return OUT;
}

//------------------------------------

float3 ballLightLoop(const float3 pos, const float3 N, const float3 V, float3 diffuse, float3 glossy, const float3 specular, const float edge, const bool is_metal)
{
   // N and V must be already normalized by the caller
   // N = normalize(N);
   // V = normalize(V);

   // normalize BRDF layer inputs //!! use diffuse = (1-glossy)*diffuse instead?
   const float diffuseMax = max(diffuse.x,max(diffuse.y,diffuse.z));
   const float glossyMax = max(glossy.x,max(glossy.y,glossy.z));
   const float specularMax = max(specular.x,max(specular.y,specular.z)); //!! not needed as 2nd layer only so far
   const float sum = diffuseMax + glossyMax; //+ specularMax
   if(sum > 1.0)
   {
      const float invsum = 1.0/sum;
      diffuse  *= invsum;
      glossy   *= invsum;
      //specular *= invsum;
   }

   float3 color = float3(0.0, 0.0, 0.0);

   BRANCH if((!is_metal && (diffuseMax > 0.0)) || (glossyMax > 0.0))
      for(int i = 0; i < iLightPointBallsNum; i++)  
         color += DoPointLight(pos, N, V, diffuse, glossy, edge, Roughness_WrapL_Edge_Thickness.x, i, is_metal); // no clearcoat needed as only pointlights so far

   BRANCH if(!is_metal && (diffuseMax > 0.0))
      color += DoEnvmapDiffuse(normalize(mul(matView, N).xyz), diffuse); // trafo back to world for lookup into world space envmap // actually: mul(float4(N, 0.0), matViewInverseInverseTranspose)

   if(specularMax > 0.0)
      color += specular; //!! blend? //!! Fresnel with 1st layer?

   return color;
}


//------------------------------------
// PIXEL SHADER

float4 psBall( const in voutQuad IN, uniform bool equirectangularMap, uniform bool decalMode, out float depth : DEPTH ) : COLOR
{
    // Analytic raycast of the unit sphere, in its own space (so the anti stretch ellipsoid comes for free): exact silhouette, surface point and normal
    const float3 o_o = IN.rayOriginObj;
    const float3 d_o = IN.rayDirObj;
    // Numerically stable form, for balls small relative to their distance: with a normalized direction, the squared
    // distance of the ray to the center is computed directly instead of as a difference of large terms
    const float invLenD = rsqrt(dot(d_o, d_o));
    const float3 dir = d_o * invLenD;
    const float b = dot(o_o, dir);
    const float3 h = o_o - b * dir; // ray point closest to the center
    const float discr = 1.0 - dot(h, h);
    // Analytic silhouette antialiasing: fade out the coverage over ~1 pixel around the outline (discr is 0 there)
    const float coverage = saturate(discr / max(fwidth(discr), 1e-7) + 0.5);
    // Evaluated before the discards (for a ray missing the sphere, at its closest point): the decal gradients need every
    // pixel of each 2x2 quad
    const float t_dir = -b - sqrt(max(discr, 0.0)); // along the normalized direction
    const float3 n_obj = normalize(o_o + t_dir * dir); // hit point on the unit sphere = exact normal
    const float t_hit = t_dir * invLenD; // ray parameter, shared with view space

    // Decal UV, reproducing the ball mesh UVs, with explicit gradients: implicit ones jump where the longitude wraps
    // (at +-pi, and u at 1), selecting the smallest mip along a meridian. The longitude gradient is derived from the
    // normal instead, d(atan2(y, x)) = (x dy - y dx) / (x^2 + y^2), which has no wrap (clamped to pi like a wrapped
    // difference, as it grows unbounded at the poles)
    const float2 decalUV = float2(frac(0.5935 + atan2(n_obj.y, n_obj.x) * 0.1339), 0.5 + asin(clamp(n_obj.z, -1., 1.)) * 0.2678);
    const float2 nxyDx = ddx(n_obj.xy);
    const float2 nxyDy = ddy(n_obj.xy);
    const float invLen2 = 1.0 / max(dot(n_obj.xy, n_obj.xy), 1e-6);
    const float dLonX = clamp((n_obj.x * nxyDx.y - n_obj.y * nxyDx.x) * invLen2, -3.14159265, 3.14159265);
    const float dLonY = clamp((n_obj.x * nxyDy.y - n_obj.y * nxyDy.x) * invLen2, -3.14159265, 3.14159265);
    const float2 decalDx = float2(dLonX * 0.1339, ddx(decalUV.y));
    const float2 decalDy = float2(dLonY * 0.1339, ddy(decalUV.y));

    if (coverage <= 0.0)
        discard;
    if (t_hit <= 0.0) // Before the ray origin, that is in front of the near plane: clipped like the rest of the frame
        discard;
    // Test the clip plane at the analytic surface point (not at the impostor quad depth)
    clip(dot(clip_plane, float4(mul_w1(n_obj, orientation), 1.0)));
    const float3 hit_view = IN.rayOriginView + t_hit * IN.rayDirView;

    // Exact per pixel depth of the analytic hit
    const float4 hit_clip = mul(float4(hit_view, 1.0), matProj);
    depth = hit_clip.z / hit_clip.w;

    // Normals transform by the transpose of the inverse transform (normalized as it is not orthonormal with anti stretch)
    const float3 N = normalize(mul(ballViewToObj, float4(n_obj, 0.0)).xyz);
    const float3 V = normalize(/*camera=0,0,0,1*/-hit_view);
    const float3 R = reflect(V, N);

    float3 ballImageColor;
    const float edge = dot(V, R);
    // edge falloff to reduce aliasing on edges (picks smaller mipmap -> more blur)
    const float lod = (edge > 0.6) ? edge*(6.0*1.0/0.4)-(6.0*0.6/0.4) : 0.0;
    BRANCH if (equirectangularMap)
    { // Equirectangular Map Reflections
      // trafo back to world for lookup into world space envmap
      // matView is always an orthonormal matrix, so no need to normalize after transform
      const float3 rv = /*normalize*/(mul(matView, -R).xyz);
      const float2 uv = ray_to_equirectangular_uv(rv);
      ballImageColor = tex2Dlod(tex_ball_color, float4(uv, 0., lod)).rgb;
    }
    else
    { // Spherical Map Reflections
      // calculate the intermediate value for the final texture coords. found here http://www.ozone3d.net/tutorials/glsl_texturing_p04.php
      const float m = (1.0 - R.z > 0.) ? 0.3535533905932737622 * rsqrt(1.0 - R.z) : 0.; // 0.353...=0.5/sqrt(2)
      const float2 uv = float2(0.5 - m * R.x, 0.5 - m * R.y);
      ballImageColor = tex2Dlod(tex_ball_color, float4(uv, 0., lod)).rgb;
    }

    const float4 decalColorT = tex2Dgrad(tex_ball_decal, decalUV, decalDx, decalDy);
    float3 decalColor = decalColorT.rgb;

    if (!decalMode)
    {
       // decal texture is an alpha scratch texture and must be added to the ball texture
       // the strength of the scratches totally rely on the alpha values.
       decalColor *= decalColorT.a;
       ballImageColor += decalColor;
    }
    else
       ballImageColor = ScreenHDR(ballImageColor, decalColor);

    BRANCH if (disableLighting)
       return float4(ballImageColor,cBase_Alpha.a * coverage);

    if (!decalMode)
       ballImageColor *= fenvEmissionScale_TexWidth.x;
    else
       ballImageColor *= 0.5*fenvEmissionScale_TexWidth.x; //!! 0.5=magic

    // No need to use a dedicated 'normal' matrix and normalize here since the matWorldView matrix is normal (world is identity and view is always orthonormal)
    //const float3 playfield_normal = normalize(mul(matWorldViewInverse, float3(0.,0.,1.)).xyz); //!! normalize necessary? // actually: mul(float4(0.,0.,1.,0.), matWorldViewInverseTranspose), but optimized to save one matrix
    //const float3 playfield_normal = mul(float4(0.0, 0.0, 1.0, 0.0), matWorldView).xyz;
    const float3 playfield_normal = float3(matWorldView._31, matWorldView._32, matWorldView._33);
    const float NdotR = dot(playfield_normal, R);

    const float3 playfield_p0 = mul_w1(float3(/*playfield_pos=*/0.,0.,0.), matWorldView);
    const float t = dot(playfield_normal, hit_view - playfield_p0) / NdotR;
    const float3 playfield_hit = hit_view - t * R;

    // New implementation: use previous frame as a reflection probe instead of computing a simplified render (this is faster and more accurate, support playfield mesh, lighting,... but there can be artefacts, with self reflection,...)
    // Move the hit point back to world space, then reproject it with the view-projection the previous frame was rendered with
    const float3 playfield_hit_world = mul(float4(playfield_hit, 1.0), matWorldViewInverse);
    const float4 proj = mul(float4(playfield_hit_world, 1.0), matPrevViewProj);
    const float2 uvp = float2(0.5, 0.5) + float2(proj.x, -proj.y) * (0.5 / proj.w);
    const float3 playfieldColor = 0.25 * (
          tex2D(tex_ball_playfield, uvp + float2(w_h_disableLighting.x, 0.)).rgb
        + tex2D(tex_ball_playfield, uvp - float2(w_h_disableLighting.x, 0.)).rgb
        + tex2D(tex_ball_playfield, uvp + float2(0., w_h_disableLighting.y)).rgb
        + tex2D(tex_ball_playfield, uvp - float2(0., w_h_disableLighting.y)).rgb
    ); // a bit of supersampling, not strictly needed, but a bit better and not that costly

    // we don't clamp sampling outside the playfield (costly and no real visual impact)
    // const float2 uv = (matWorldViewInverse * float4(playfield_hit, 1.0)).xy * invTableRes_reflection.xy;
    // && !(uv.x < 0.1 && uv.y < 0.1 && uv.x > 0.9 && uv.y > 0.9)
    BRANCH if (!(uvp.x < 0. || uvp.x > 1. || uvp.y < 0. || uvp.y > 1.) // outside of previous render => discard (we could use sampling techniques to optimize a bit)
            && !(t <= 0.)) // t < 0.0 may happen in some situation where ball intersects the playfield and the reflected point is inside the ball (like in kicker)
    {
        // NdotR allows to fade between playfield (down) and environment (up)
        ballImageColor = lerp(ballImageColor, playfieldColor, smoothstep(0.0, 0.15, NdotR) * invTableRes_reflection.z);
    }

    // We can face infinite reflections (ball->playfield->ball->playfield->...) which would overflow, or very bright dots that would cause lighting artefacts, so we saturate to an arbitrary value
    ballImageColor = min(ballImageColor, float3(15., 15., 15.));

    float3 diffuse = cBase_Alpha.rgb*0.075;
    if(!decalMode)
       diffuse *= decalColor; // scratches make the material more rough

    const float3 glossy = max(diffuse*2.0, float3(0.1,0.1,0.1)); //!! meh

    float3 specular = ballImageColor * cBase_Alpha.rgb; //!! meh, too, as only added in ballLightLoop anyhow
    if(!decalMode)
       specular *= float3(1.,1.,1.)-decalColor; // see above

    float4 color;
    color.rgb = ballLightLoop(hit_view, N, V, diffuse, glossy, specular, 1.0, false);
    color.a = cBase_Alpha.a * coverage;
    return color;
}

float4 psBallDebug(in vout IN) : COLOR
{
    return float4(1.0, 1.0, 1.0, 0.0);

}

float4 psBallTrail( in voutTrail IN ) : COLOR
{
   // the texture used is the ball HDR IBL so it can lead to very high value => keep it SDR
   const float3 ballImageColor = saturate(tex2D(tex_ball_color, IN.tex0_alpha.xy).rgb);
   if (disableLighting)
      return float4(ballImageColor, IN.tex0_alpha.z);
   else
      return float4((cBase_Alpha.rgb*(0.075*0.25) + ballImageColor)*fenvEmissionScale_TexWidth.x, IN.tex0_alpha.z); //!! just add the ballcolor in, this is a whacky trail anyhow
}

//------------------------------------
// Techniques

technique RenderBall { pass p0 { vertexshader = compile vs_3_0 vsBallQuad(); pixelshader  = compile ps_3_0 psBall(true, false);	}}
technique RenderBall_DecalMode { pass p0 { vertexshader = compile vs_3_0 vsBallQuad(); pixelshader  = compile ps_3_0 psBall(true, true); }}
technique RenderBall_SphericalMap {	pass p0	{ vertexshader = compile vs_3_0 vsBallQuad(); pixelshader = compile ps_3_0 psBall(false, false); }}
technique RenderBall_SphericalMap_DecalMode { pass p0 { vertexshader = compile vs_3_0 vsBallQuad(); pixelshader = compile ps_3_0 psBall(false, true); }}
technique RenderBall_Debug { pass p0 { vertexshader = compile vs_3_0 vsBall(); pixelshader = compile ps_3_0 psBallDebug(); }}
technique RenderBallTrail {	pass p0	{ vertexshader = compile vs_3_0 vsBallTrail(); pixelshader  = compile ps_3_0 psBallTrail();	}}
