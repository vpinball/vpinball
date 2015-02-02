float fResX = 128.f;
float fResY = 32.f;
float3 vColor = float3(1.f,1.f,1.f);
float intensity = 1.0f;

texture Texture0;

float3 InvGamma(float3 color) //!! use hardware support? D3DSAMP_SRGBTEXTURE
{
	return /*color * (color * (color * 0.305306011f + 0.682171111f) + 0.012522878f);/*/ pow(color,2.2f); // pow does not matter anymore on current GPUs
}

float3 InvToneMap(float3 color)
{
    float burnhighlights = 0.25f;
    
	const float inv_2bh = 0.5f/burnhighlights;
    const float bh = 4.0f*burnhighlights - 2.0f;
	color.x = (color.x - 1.0f + sqrt(color.x*(color.x + bh) + 1.0f))*inv_2bh;
	color.y = (color.y - 1.0f + sqrt(color.y*(color.y + bh) + 1.0f))*inv_2bh;
	color.z = (color.z - 1.0f + sqrt(color.z*(color.z + bh) + 1.0f))*inv_2bh;

	return color;
}

sampler2D texSampler0 : TEXUNIT0 = sampler_state
{
	Texture	  = (Texture0);
    MIPFILTER = NONE;
    MAGFILTER = POINT;
    MINFILTER = POINT;
};

sampler2D texSampler1 : TEXUNIT0 = sampler_state
{
	Texture	  = (Texture0);
    MIPFILTER = LINEAR;
    MAGFILTER = LINEAR;
    MINFILTER = LINEAR;
    // Set texture to mirror, so the alpha state of the texture blends correctly to the outside
	ADDRESSU  = MIRROR;
	ADDRESSV  = MIRROR;
};


//function output structures 
struct VS_OUTPUT 
{ 
   float4 pos           : POSITION; 
   float2 tex0          : TEXCOORD0; 
}; 

VS_OUTPUT vs_main (float4 vPosition  : POSITION0,  
                   float2 tc         : TEXCOORD0) 
{ 
   VS_OUTPUT Out;

   Out.pos = float4(vPosition.xy,0.0f,1.0f);
   Out.tex0 = tc;
   
   return Out; 
}

float4 ps_main_DMD_big( in VS_OUTPUT IN) : COLOR
{
   float l = tex2Dlod(texSampler0, float4(IN.tex0, 0.f,0.f)).z*(255.9/100.);
   float3 color = l*(vColor*intensity); //!! create function that resembles LUT from VPM?

   float2 xy = IN.tex0 * float2(fResX,fResY);
   float2 dist = (xy-floor(xy))*2.2f-1.1f;
   float d = dist.x*dist.x+dist.y*dist.y;

   color *= smoothstep(0,1,1.0f-d*d);

   /*float3 color2 = float3(0,0,0);
   for(int j = -1; j <= 1; ++j)
     for(int i = -1; i <= 1; ++i)
	 {
	 //collect glow from neighbors
	 }*/

   return float4(InvToneMap(InvGamma(color/*+color2*/)),1); //!! meh, this sucks a bit performance-wise, but how to avoid this when doing fullscreen-tonemap/gamma without stencil and depth read?
}

float4 ps_main_DMD( in VS_OUTPUT IN) : COLOR
{
   float l = tex2Dlod(texSampler0, float4(IN.tex0, 0.f,0.f)).z*(255.9/100.);
   float3 color = l*(vColor*1.25f*intensity); //!! create function that resembles LUT from VPM?  //!! 1.25f meh

   float2 xy = IN.tex0 * float2(fResX,fResY);
   float2 dist = (xy-floor(xy))*2.2f-1.1f;
   float d = dist.x*dist.x+dist.y*dist.y;

   color *= saturate(1.0f-d);

   /*float3 color2 = float3(0,0,0);
   for(int j = -1; j <= 1; ++j)
     for(int i = -1; i <= 1; ++i)
	 {
	 //collect glow from neighbors
	 }*/

   return float4(InvToneMap(InvGamma(color/*+color2*/)),1); //!! meh, this sucks a bit performance-wise, but how to avoid this when doing fullscreen-tonemap/gamma without stencil and depth read?
}

float4 ps_main_noDMD( in VS_OUTPUT IN) : COLOR
{
   float4 l = tex2D(texSampler1, IN.tex0);

   return float4(InvToneMap(InvGamma(l.xyz*(vColor*intensity))),l.w); //!! meh, this sucks a bit performance-wise, but how to avoid this when doing fullscreen-tonemap/gamma without stencil and depth read?
}

technique basic_DMD
{ 
   pass P0 
   { 
      VertexShader = compile vs_3_0 vs_main(); 
	  PixelShader = compile ps_3_0 ps_main_DMD();
   } 
}

technique basic_DMD_big
{ 
   pass P0 
   { 
      VertexShader = compile vs_3_0 vs_main(); 
	  PixelShader = compile ps_3_0 ps_main_DMD_big();
   } 
}


technique basic_noDMD
{ 
   pass P0 
   { 
      VertexShader = compile vs_3_0 vs_main(); 
	  PixelShader = compile ps_3_0 ps_main_noDMD();
   } 
}
