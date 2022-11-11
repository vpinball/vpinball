// transformation matrices
const float4x4 matWorldViewProj : WORLDVIEWPROJ;

#define NUM_BALLS 8
const float4 balls[NUM_BALLS];

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

float4 PS_BulbLight_with_ball_shadows(const in VS_LIGHTBULB_OUTPUT IN) : COLOR
{
	const float len = length(lightCenter_maxRange.xyz - IN.tablePos) * lightCenter_maxRange.w;
	const float atten = pow(1.0 - saturate(len), lightColor2_falloff_power.w);
	const float3 lcolor = lerp(lightColor2_falloff_power.xyz, lightColor_intensity.xyz, sqrt(len));
   float3 color = lcolor * (-blend_modulate_vs_add * atten * lightColor_intensity.w); // negative as it will be blended with '1.0-thisvalue' (the 1.0 is needed to modulate the underlying elements correctly, but not wanted for the term below)

	// Raytraced ball shadows
   for (int i = 0; i < NUM_BALLS; i++)
   {
      const float light_r = 10.0f; // light radius in VPX units
      float ball_r = balls[i].w;
      float3 ball_pos = balls[i].xyz;
      float3 light_dir = IN.tablePos - lightCenter_maxRange.xyz;
      float light_dist = length(light_dir);
      float3 light_ray = light_dir / light_dist;
      float3 light_ball_ray = ball_pos - lightCenter_maxRange.xyz;
      float dot_lbr_lr = dot(light_ball_ray, light_ray);
      // Don't cast shadow behind the light or before occluder
      if (dot_lbr_lr > 0.0 && dot_lbr_lr < light_dist)
      {
         float3 dist = light_ball_ray - dot_lbr_lr * light_ray;
         float d2 = length(dist);
         float smoothness = light_r * (1.0 - dot_lbr_lr / light_dist);
         color.rgb *= 0.1 + 0.9 * smoothstep(ball_r - smoothness, ball_r + smoothness, d2);
      }
   }

   return float4(color, 1.0 / blend_modulate_vs_add - 1.0); //saturate(atten*lightColor_intensity.w));
}

technique bulb_light
{
   pass P0
   {
		vertexshader = compile vs_3_0 vs_lightbulb_main();
		pixelshader  = compile ps_3_0 PS_BulbLight();
		// pixelshader  = compile ps_3_0 PS_BulbLight_with_ball_shadows();
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
