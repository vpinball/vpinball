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

#define NUM_BALLS 8
const float4 balls[NUM_BALLS];

// Raytraced ball shadows
float get_light_ball_shadow(const float3 light_pos, const float3 light_dir, const float light_dist)
{
	float result = 1.0;
	for (int i = 0; i < NUM_BALLS; i++)
	{
		const float ball_r = balls[i].w;
		[branch] if (ball_r == 0.0) // early out as soon as first 'invalid' ball is detected
			return result;
		const float3 ball_pos = balls[i].xyz;
		const float3 light_ball_ray = ball_pos - light_pos;
		const float dot_lbr_lr_divld = dot(light_ball_ray, light_dir) / (light_dist * light_dist);
		// Don't cast shadow behind the light or before occluder
		[branch] if (dot_lbr_lr_divld > 0.0 && dot_lbr_lr_divld < 1.0)
		{
			const float3 dist = light_ball_ray - dot_lbr_lr_divld * light_dir;
			const float d2 = length(dist);
			const float light_r = 10.0; // light radius in VPX units
			const float smoothness = light_r - light_r * dot_lbr_lr_divld;
			const float light_inside_ball_sqr = saturate((light_ball_ray.x*light_ball_ray.x + light_ball_ray.y*light_ball_ray.y)/(ball_r*ball_r)); // fade to 1 if light inside ball
			result *= 1.0 + light_inside_ball_sqr*(-1.0 + 0.1 + 0.9 * smoothstep(ball_r-smoothness, ball_r+smoothness, d2));
		}
	}
	return result;
}

float4 PS_BulbLight(const in VS_LIGHTBULB_OUTPUT IN) : COLOR
{
	const float3 light_dir = IN.tablePos - lightCenter_maxRange.xyz;
	const float light_dist = length(light_dir);
	const float len = light_dist * lightCenter_maxRange.w;
	const float atten = pow(1.0 - saturate(len), lightColor2_falloff_power.w);
	const float3 lcolor = lerp(lightColor2_falloff_power.xyz, lightColor_intensity.xyz, sqrt(len));
	float shadow = 1.0;
	[branch] if(lightCenter_maxRange.w > 0.002) // 1/500 (=large, almost/to table filling lights)
		shadow = get_light_ball_shadow(lightCenter_maxRange.xyz, light_dir, light_dist);
	return float4(
	 lcolor*(-blend_modulate_vs_add*atten*lightColor_intensity.w*shadow), // negative as it will be blended with '1.0-thisvalue' (the 1.0 is needed to modulate the underlying elements correctly, but not wanted for the term below)
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
