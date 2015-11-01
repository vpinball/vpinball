// transformation matrices
float4x4 matWorldViewProj : WORLDVIEWPROJ;

float4   lightColor_intensity;
float4   lightColor2_falloff_power;
float4   lightCenter_maxRange;
float    blend_modulate_vs_add;

struct VS_LIGHTBULB_OUTPUT
{ 
   float4 pos           : POSITION;
   float3 tablePos      : TEXCOORD0;
};

VS_LIGHTBULB_OUTPUT vs_lightbulb_main (float4 vPosition : POSITION0,  
                                       float3 vNormal   : NORMAL0,  
                                       float2 tc        : TEXCOORD0) 
{
   VS_LIGHTBULB_OUTPUT Out;

   Out.pos = mul(vPosition, matWorldViewProj);
   Out.pos.z = max(Out.pos.z, 0.00001); // clamp lights to near clip plane to avoid them being partially clipped
   Out.tablePos = vPosition.xyz;
   
   return Out; 
}

float4 PS_BulbLight(in VS_LIGHTBULB_OUTPUT IN) : COLOR
{
	const float len = length(lightCenter_maxRange.xyz - IN.tablePos) * lightCenter_maxRange.w;
    const float atten = pow(1.0 - saturate(len), lightColor2_falloff_power.w);
	const float3 lcolor = lerp(lightColor2_falloff_power.xyz, lightColor_intensity.xyz, sqrt(len));
	float4 result;
	result.xyz = lcolor*(-blend_modulate_vs_add*atten*lightColor_intensity.w); // negative as it will be blended with '1.0-thisvalue' (the 1.0 is needed to modulate the underlying elements correctly, but not wanted for the term below)
	result.a = 1.0/blend_modulate_vs_add - 1.0; //saturate(atten*lightColor_intensity.w);
	return result;
}

technique bulb_light
{ 
   pass P0 
   { 
		vertexshader = compile vs_3_0 vs_lightbulb_main();
		pixelshader  = compile ps_3_0 PS_BulbLight();
   } 
}
