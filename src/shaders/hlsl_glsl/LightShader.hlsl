// license:GPLv3+

#include "Helpers.fxh"

// transformation matrices
const float4x4 matWorldViewProj : WORLDVIEWPROJ;

const float4   lightColor_intensity;
const float4   lightColor2_falloff_power;
const float4   lightCenter_maxRange;
const float    blend_modulate_vs_add;

struct VS_LIGHTBULB_OUTPUT
{
   float4 pos      : POSITION;
   float3 tablePos : TEXCOORD0; // rely for backglass mode that z is initialized to 0!
};

// vertex shader is skipped for backglass elements, due to D3DDECLUSAGE_POSITIONT
VS_LIGHTBULB_OUTPUT vs_lightbulb_main (const in float4 vPosition : POSITION0/*,
                                       const in float3 vNormal   : NORMAL0,
                                       const in float2 tc        : TEXCOORD0*/)
{
   VS_LIGHTBULB_OUTPUT Out;

   Out.pos = mul(vPosition, matWorldViewProj);
   //Disabled since this would move backdrop lights behind other backdrop parts (rendered at z = 0), this could be readded with a max at 0, but I don't get why we would need this type of hack
   //Out.pos.z = max(Out.pos.z, 0.00001); // clamp lights to near clip plane to avoid them being partially clipped
   Out.tablePos = vPosition.xyz;

   return Out; 
}

float4 PS_BulbLight(const in VS_LIGHTBULB_OUTPUT IN) : COLOR
{
   const float3 light_dir = IN.tablePos - lightCenter_maxRange.xyz;
   const float light_dist = length(light_dir);
   const float len = light_dist * lightCenter_maxRange.w;
   const float atten = pow(max(1.0 - len, 0.0001), lightColor2_falloff_power.w);
   const float3 lcolor = lerp(lightColor2_falloff_power.xyz, lightColor_intensity.xyz, sqrt(len));
   const float3 color = lcolor * (-blend_modulate_vs_add * atten * lightColor_intensity.w); // negative as it will be blended with '1.0-thisvalue' (the 1.0 is needed to modulate the underlying elements correctly, but not wanted for the term below)
   return float4(color, 1.0 / blend_modulate_vs_add - 1.0); //saturate(atten*lightColor_intensity.w));
}

#include "BallShadows.fxh"

float4 PS_BulbLight_with_ball_shadows(const in VS_LIGHTBULB_OUTPUT IN) : COLOR
{
   const float3 light_dir = IN.tablePos - lightCenter_maxRange.xyz;
   const float light_dist = length(light_dir);
   const float len = light_dist * lightCenter_maxRange.w;
   const float atten = pow(max(1.0 - len, 0.0001), lightColor2_falloff_power.w);
   const float3 lcolor = lerp(lightColor2_falloff_power.xyz, lightColor_intensity.xyz, sqrt(len));
   const float shadow = get_light_ball_shadow(lightCenter_maxRange.xyz, light_dir, light_dist);
   const float3 color = lcolor * (-blend_modulate_vs_add * atten * lightColor_intensity.w * shadow); // negative as it will be blended with '1.0-thisvalue' (the 1.0 is needed to modulate the underlying elements correctly, but not wanted for the term below)
   return float4(color, 1.0 / blend_modulate_vs_add - 1.0); //saturate(atten*lightColor_intensity.w));
}

technique bulb_light
{
   pass P0
   {
      vertexshader = compile vs_3_0 vs_lightbulb_main();
      pixelshader  = compile ps_3_0 PS_BulbLight();
   }
}

technique bulb_light_with_ball_shadows
{
   pass P0
   {
      vertexshader = compile vs_3_0 vs_lightbulb_main();
      pixelshader = compile ps_3_0 PS_BulbLight_with_ball_shadows();
   }
}
