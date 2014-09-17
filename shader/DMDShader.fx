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
   float l = tex2D(texSampler0, IN.tex0).z*(255.f/100.f);

   return float4(l,l,l,1);
}

technique basic
{ 
   pass P0 
   { 
      //SPECULARENABLE = (bSpecular);
      VertexShader = compile vs_3_0 vs_main(); 
	  PixelShader = compile ps_3_0 ps_main();
   } 
}
