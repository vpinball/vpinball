// Shared shader file used by bulb lights and basic shaders (OpenGL and DX9)

#define NUM_BALLS 8

#ifdef GLSL
uniform vec4 balls[NUM_BALLS];
#else
const float4 balls[NUM_BALLS];
#endif

// Raytraced ball shadows
float get_light_ball_shadow(const float3 light_pos, const float3 light_dir, const float light_dist)
{
	float result = 1.0;
	for (int i = 0; i < NUM_BALLS; i++)
	{
		const float ball_r = balls[i].w;
		BRANCH if (ball_r == 0.0) // early out as soon as first 'invalid' ball is detected
			return result;
		const float3 ball_pos = balls[i].xyz;
		const float3 light_ray = light_dir / light_dist;
		const float3 light_ball_ray = ball_pos - light_pos;
		const float dot_lbr_lr = dot(light_ball_ray, light_ray);
		// Don't cast shadow behind the light or before occluder
		BRANCH if (dot_lbr_lr > 0.0 && dot_lbr_lr < light_dist)
		{
			const float3 dist = light_ball_ray - dot_lbr_lr * light_ray;
			const float d2 = length(dist);
			const float light_r = 10.0; // light radius in VPX units
			const float smoothness = light_r * (1.0 - dot_lbr_lr / light_dist);
			result *= 0.1 + 0.9 * smoothstep(ball_r-smoothness, ball_r+smoothness, d2);
		}
	}
	return result;
}
