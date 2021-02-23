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
   Out.pos.z = max(Out.pos.z, 0.00001); // clamp lights to near clip plane to avoid them being partially clipped
   Out.tablePos = vPosition.xyz;

   return Out; 
}

float4 PS_BulbLight(const in VS_LIGHTBULB_OUTPUT IN) : COLOR
{
	const float len = length(lightCenter_maxRange.xyz - IN.tablePos) * lightCenter_maxRange.w;
	const float atten = pow(1.0 - saturate(len), lightColor2_falloff_power.w);
	const float3 lcolor = lerp(lightColor2_falloff_power.xyz, lightColor_intensity.xyz, sqrt(len));
	return float4(
	 lcolor*(-blend_modulate_vs_add*atten*lightColor_intensity.w), // negative as it will be blended with '1.0-thisvalue' (the 1.0 is needed to modulate the underlying elements correctly, but not wanted for the term below)
	 1.0/blend_modulate_vs_add - 1.0); //saturate(atten*lightColor_intensity.w));
}

technique bulb_light
{
   pass P0
   {
		vertexshader = compile vs_3_0 vs_lightbulb_main();
		pixelshader  = compile ps_3_0 PS_BulbLight();
   }
}
