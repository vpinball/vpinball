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
const float4x4 matProj;
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
    ADDRESSU  = Wrap;
    ADDRESSV  = Wrap;
    SRGBTexture = true;
};

#include "Material.fxh"

const float4x3 orientation;
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

float4 psBall( const in vout IN, uniform bool equirectangularMap, uniform bool decalMode ) : COLOR
{
    const float3 V = normalize(/*camera=0,0,0,1*/-IN.worldPos_t0y.xyz);
    const float3 N = normalize(IN.normal_t0x.xyz);
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

    const float4 decalColorT = tex2D(tex_ball_decal, float2(IN.normal_t0x.w, IN.worldPos_t0y.w));
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
       return float4(ballImageColor,cBase_Alpha.a);

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
    const float t = dot(playfield_normal, IN.worldPos_t0y.xyz - playfield_p0) / NdotR;
    const float3 playfield_hit = IN.worldPos_t0y.xyz - t * R;

    // New implementation: use previous frame as a reflection probe instead of computing a simplified render (this is faster and more accurate, support playfield mesh, lighting,... but there can be artefacts, with self reflection,...)
    // TODO use previous frame projection instead of the one of the current frame to limit reflection distortion (still this is minimal)
    const float4 proj = mul(float4(playfield_hit, 1.0), matProj);
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
    color.rgb = ballLightLoop(IN.worldPos_t0y.xyz, N, V, diffuse, glossy, specular, 1.0, false);
    color.a = cBase_Alpha.a;
    return color;
}

float4 psBallDebug(in vout IN) : COLOR
{
    return float4(1.0, 1.0, 1.0, 0.0);

}

float4 psBallTrail( in voutTrail IN ) : COLOR
{
   const float3 ballImageColor = tex2D(tex_ball_color, IN.tex0_alpha.xy).rgb;
   if (disableLighting)
      return float4(ballImageColor, IN.tex0_alpha.z);
   else
      return float4((cBase_Alpha.rgb*(0.075*0.25) + ballImageColor)*fenvEmissionScale_TexWidth.x, IN.tex0_alpha.z); //!! just add the ballcolor in, this is a whacky trail anyhow
}

//------------------------------------
// Techniques

technique RenderBall { pass p0 { vertexshader = compile vs_3_0 vsBall(); pixelshader  = compile ps_3_0 psBall(true, false);	}}
technique RenderBall_DecalMode { pass p0 { vertexshader = compile vs_3_0 vsBall(); pixelshader  = compile ps_3_0 psBall(true, true); }}
technique RenderBall_SphericalMap {	pass p0	{ vertexshader = compile vs_3_0 vsBall(); pixelshader = compile ps_3_0 psBall(false, false); }}
technique RenderBall_SphericalMap_DecalMode { pass p0 { vertexshader = compile vs_3_0 vsBall(); pixelshader = compile ps_3_0 psBall(false, true); }}
technique RenderBall_Debug { pass p0 { vertexshader = compile vs_3_0 vsBall(); pixelshader = compile ps_3_0 psBallDebug(); }}
technique RenderBallTrail {	pass p0	{ vertexshader = compile vs_3_0 vsBallTrail(); pixelshader  = compile ps_3_0 psBallTrail();	}}
