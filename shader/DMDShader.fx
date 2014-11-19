float fResX = 128.f;
float fResY = 32.f;
float3 vColor = float3(1.f,1.f,1.f);

texture Texture0;

sampler2D texSampler0 : TEXUNIT0 = sampler_state
{
	Texture	  = (Texture0);
    MIPFILTER = POINT;
    MAGFILTER = POINT;
    MINFILTER = POINT;
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

   Out.pos = vPosition;
   Out.tex0 = tc;
   
   return Out; 
}

float4 ps_main( in VS_OUTPUT IN) : COLOR
{
   float l = tex2D(texSampler0, IN.tex0).z*(255.9f/100.f);
   float3 color = float3(l,l,l);

   float2 xy = IN.tex0 * float2(fResX,fResY);
   float2 dist = (xy-floor(xy))*2.2f-1.1f;
   float d = dist.x*dist.x+dist.y*dist.y;

   color *= smoothstep(0,1,1.0f-d*d);

   float3 color2 = float3(0,0,0);
   /*for(int j = -1; j <= 1; ++j)
     for(int i = -1; i <= 1; ++i)
	 {
	 //collect glow from neighbors
	 }*/

   return float4(color+color2,1);
}

technique basic
{ 
   pass P0 
   { 
      VertexShader = compile vs_3_0 vs_main(); 
	  PixelShader = compile ps_3_0 ps_main();
   } 
}
