/**
 * Copyright (C) 2013 Jorge Jimenez (jorge@iryoku.com)
 * Copyright (C) 2013 Jose I. Echevarria (joseignacioechevarria@gmail.com)
 * Copyright (C) 2013 Belen Masia (bmasia@unizar.es)
 * Copyright (C) 2013 Fernando Navarro (fernandn@microsoft.com)
 * Copyright (C) 2013 Diego Gutierrez (diegog@unizar.es)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to
 * do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software. As clarification, there
 * is no requirement that the copyright notice and permission be included in
 * binary distributions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
//Modified for Visual pinball
 
#define SMAA_PRESET_HIGH
//#define SMAA_USE_DEPTH
//#define SMAA_USE_STENCIL
#define SMAA_USE_COLOR // otherwise luma

#ifdef SMAA_PRESET_CUSTOM
/**
 * This can be ignored; its purpose is to support interactive custom parameter
 * tweaking.
 */
float threshld;
float maxSearchSteps;
float maxSearchStepsDiag;
float cornerRounding;
#endif

// Use a real macro here for maximum performance!
#ifndef SMAA_RT_METRICS // This is just for compilation-time syntax checking.
#define SMAA_RT_METRICS float4(w_h_height.x, w_h_height.y, w_h_height.z, 1.0/w_h_height.y)
#endif

// Set the HLSL version:
#define SMAA_HLSL_3

#ifdef SMAA_PRESET_CUSTOM
#define SMAA_THRESHOLD threshld
#define SMAA_MAX_SEARCH_STEPS maxSearchSteps
#define SMAA_MAX_SEARCH_STEPS_DIAG maxSearchStepsDiag
#define SMAA_CORNER_ROUNDING cornerRounding
#endif

// And include our header (identical for HLSL/GLSL)!
#include "SMAA.fxh"


/**
 * Input vars and textures.
 */

//texture2D colorTex2D;
#ifdef SMAA_USE_DEPTH
texture2D depthTex2D;
#endif

/**
 * DX9 samplers.
 */
/*sampler2D colorTex : TEXUNIT0 {
    Texture = <colorTex2D>;
    AddressU  = Clamp; AddressV = Clamp;
    MipFilter = Point; MinFilter = Linear; MagFilter = Linear;
    SRGBTexture = true;
};*/

#define colorTex tex_fb_filtered //!! misses SRGB, also see SMAA_NeighborhoodBlending()

/*sampler2D colorGammaTex : TEXUNIT1 {
    Texture = <colorTex2D>;
    AddressU  = Clamp; AddressV = Clamp;
    MipFilter = Linear; MinFilter = Linear; MagFilter = Linear;
    SRGBTexture = false;
};*/

#define colorGammaTex tex_fb_filtered //!! misses MipFilter

#ifdef SMAA_USE_DEPTH
sampler2D depthTex {
    Texture = <depthTex2D>;
    AddressU  = Clamp; AddressV = Clamp;
    MipFilter = Linear; MinFilter = Linear; MagFilter = Linear;
    SRGBTexture = false;
};
#endif

texture2D Texture7;  // edgesTex
texture2D Texture8;  // blendTex
texture2D Texture9;  // areaTex
texture2D Texture10; // searchTex

sampler2D edgesTex : TEXUNIT7 = sampler_state {
    Texture       = (Texture7);
    AddressU      = Clamp;
	AddressV      = Clamp;
    MipFilter     = Linear;
	MinFilter     = Linear;
	MagFilter     = Linear; //!! ??
    //SRGBTexture = false;
};

sampler2D blendTex : TEXUNIT8 = sampler_state {
    Texture       = (Texture8);
    AddressU      = Clamp;
	AddressV      = Clamp;
    MipFilter     = Linear;
	MinFilter     = Linear;
	MagFilter     = Linear; //!! ??
    //SRGBTexture = false;
};

sampler2D areaTex : TEXUNIT9 = sampler_state {
    Texture       = (Texture9);
    AddressU      = Clamp;
	AddressV      = Clamp;
	AddressW      = Clamp;
    MipFilter     = Linear;
	MinFilter     = Linear;
	MagFilter     = Linear; //!! ??
    //SRGBTexture = false;
};

sampler2D searchTex : TEXUNIT10 = sampler_state {
    Texture       = (Texture10);
    AddressU      = Clamp;
	AddressV      = Clamp;
	AddressW      = Clamp;
    MipFilter     = Point;
	MinFilter     = Point;
	MagFilter     = Point;
    //SRGBTexture = false;
};


/**
 * Function wrappers
 */
void DX9_SMAAEdgeDetectionVS(inout float4 position : POSITION,
                             inout float2 texcoord : TEXCOORD0,
                             out float4 offset[3] : TEXCOORD1) {
    // DirectX has 0,0 at the top left corner of the first texel. Pixel perfect sampling needs coordinates to be offseted by half a texel.
    texcoord += w_h_height.xy*0.5;
    SMAAEdgeDetectionVS(texcoord, offset);
}

void DX9_SMAABlendingWeightCalculationVS(inout float4 position : POSITION,
                                         inout float2 texcoord : TEXCOORD0,
                                         out float2 pixcoord : TEXCOORD1,
                                         out float4 offset[3] : TEXCOORD2) {
    // DirectX has 0,0 at the top left corner of the first texel. Pixel perfect sampling needs coordinates to be offseted by half a texel.
    texcoord += w_h_height.xy*0.5;
    SMAABlendingWeightCalculationVS(texcoord, pixcoord, offset);
}

void DX9_SMAANeighborhoodBlendingVS(inout float4 position : POSITION,
                                    inout float2 texcoord : TEXCOORD0,
                                    out float4 offset : TEXCOORD1) {
    // DirectX has 0,0 at the top left corner of the first texel. Pixel perfect sampling needs coordinates to be offseted by half a texel.
    texcoord += w_h_height.xy*0.5;
    SMAANeighborhoodBlendingVS(texcoord, offset);
}

#ifndef SMAA_USE_COLOR
float4 DX9_SMAALumaEdgeDetectionPS(float4 position : SV_POSITION,
                                   float2 texcoord : TEXCOORD0,
                                   float4 offset[3] : TEXCOORD1) : COLOR {
    return float4(SMAALumaEdgeDetectionPS(texcoord, offset, colorGammaTex), 0.0, 0.0);
}
#else
float4 DX9_SMAAColorEdgeDetectionPS(float4 position : SV_POSITION,
                                    float2 texcoord : TEXCOORD0,
                                    float4 offset[3] : TEXCOORD1) : COLOR {
    return float4(SMAAColorEdgeDetectionPS(texcoord, offset, colorGammaTex), 0.0, 0.0);
}
#endif

#ifdef SMAA_USE_DEPTH
float4 DX9_SMAADepthEdgeDetectionPS(float4 position : SV_POSITION,
                                    float2 texcoord : TEXCOORD0,
                                    float4 offset[3] : TEXCOORD1) : COLOR {
    return float4(SMAADepthEdgeDetectionPS(texcoord, offset, depthTex), 0.0, 0.0);
}
#endif

float4 DX9_SMAABlendingWeightCalculationPS(float4 position : SV_POSITION,
                                           float2 texcoord : TEXCOORD0,
                                           float2 pixcoord : TEXCOORD1,
                                           float4 offset[3] : TEXCOORD2) : COLOR {
    return SMAABlendingWeightCalculationPS(texcoord, pixcoord, offset, edgesTex, areaTex, searchTex, 0.0);
}

float4 DX9_SMAANeighborhoodBlendingPS(float4 position : SV_POSITION,
                                      float2 texcoord : TEXCOORD0,
                                      float4 offset : TEXCOORD1) : COLOR {
    return SMAANeighborhoodBlendingPS(texcoord, offset, colorTex, blendTex);
}


/**
 * Time for some techniques!
 */
#ifndef SMAA_USE_COLOR
technique SMAA_LumaEdgeDetection {
    pass SMAA_LumaEdgeDetection {
        VertexShader = compile vs_3_0 DX9_SMAAEdgeDetectionVS();
        PixelShader = compile ps_3_0 DX9_SMAALumaEdgeDetectionPS();
        //ZEnable = false;        
        //SRGBWriteEnable = false;
        //AlphaBlendEnable = false;
        //AlphaTestEnable = false;

#ifdef SMAA_USE_STENCIL
        // We will be creating the stencil buffer for later usage.
        StencilEnable = true;
        StencilPass = REPLACE;
        StencilRef = 1;
#endif
    }
}
#else
technique SMAA_ColorEdgeDetection {
    pass SMAA_ColorEdgeDetection {
        VertexShader = compile vs_3_0 DX9_SMAAEdgeDetectionVS();
        PixelShader = compile ps_3_0 DX9_SMAAColorEdgeDetectionPS();
        //ZEnable = false;        
        //SRGBWriteEnable = false;
        //AlphaBlendEnable = false;
        //AlphaTestEnable = false;

#ifdef SMAA_USE_STENCIL
        // We will be creating the stencil buffer for later usage.
        StencilEnable = true;
        StencilPass = REPLACE;
        StencilRef = 1;
#endif
    }
}
#endif

#ifdef SMAA_USE_DEPTH
technique SMAA_DepthEdgeDetection {
    pass SMAA_DepthEdgeDetection {
        VertexShader = compile vs_3_0 DX9_SMAAEdgeDetectionVS();
        PixelShader = compile ps_3_0 DX9_SMAADepthEdgeDetectionPS();
        //ZEnable = false;        
        //SRGBWriteEnable = false;
        //AlphaBlendEnable = false;
        //AlphaTestEnable = false;

#ifdef SMAA_USE_STENCIL
        // We will be creating the stencil buffer for later usage.
        StencilEnable = true;
        StencilPass = REPLACE;
        StencilRef = 1;
#endif
    }
}
#endif

technique SMAA_BlendWeightCalculation {
    pass SMAA_BlendWeightCalculation {
        VertexShader = compile vs_3_0 DX9_SMAABlendingWeightCalculationVS();
        PixelShader = compile ps_3_0 DX9_SMAABlendingWeightCalculationPS();
        //ZEnable = false;
        //SRGBWriteEnable = false;
        //AlphaBlendEnable = false;
        //AlphaTestEnable = false;

#ifdef SMAA_USE_STENCIL
        // Here we want to process only marked pixels.
        StencilEnable = true;
        StencilPass = KEEP;
        StencilFunc = EQUAL;
        StencilRef = 1;
#endif
    }
}

technique SMAA_NeighborhoodBlending {
    pass SMAA_NeighborhoodBlending {
        VertexShader = compile vs_3_0 DX9_SMAANeighborhoodBlendingVS();
        PixelShader = compile ps_3_0 DX9_SMAANeighborhoodBlendingPS();
        //ZEnable = false;
        //!! SRGBWriteEnable = true;
        //AlphaBlendEnable = false;
        //AlphaTestEnable = false;

#ifdef SMAA_USE_STENCIL
        // Here we want to process all the pixels.
        StencilEnable = false;
#endif
    }
}
