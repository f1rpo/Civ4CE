//  *****************   CIV4 ********************
//
//  FILE:    PlotIndicator.fx
//
//  AUTHOR:  Nat Duca (03-24-2005)
//			 Tom Whittaker 9.15.05 - simpipled Plot icons to 2 textures, simple draw call and not texturecompostiing
//
//  PURPOSE: Draw the plot widget with specified color, masked icon and border highlights
//
//------------------------------------------------------------------------------------------------
//  Copyright (c) 2005 Firaxis Games, Inc. All rights reserved.
//------------------------------------------------------------------------------------------------

float4 f4MaterialEmissive : MATERIALEMISSIVE = (1.0.xxxx);
float4 f4MaterialDiffuse : MATERIALDIFFUSE = (1.0.xxxx);
float4x4 mtxWorldViewProj: WORLDVIEWPROJECTIONTRANSPOSE;

//structs
struct VS_INPUT
{
   float4 f4Position : POSITION;
   float2 f2TexCoord1 : TEXCOORD0;
   float2 f2TexCoord2 : TEXCOORD1;
};

struct VS_OUTPUT
{
	float4 f4Position : POSITION;
	float2 f2TexCoord1 : TEXCOORD0;
	float2 f2TexCoord2 : TEXCOORD1;
	float2 f2TexCoord3 : TEXCOORD2;
};

//------------------------------------------------------------------------------------------------
//                          VERTEX SHADER
//------------------------------------------------------------------------------------------------
VS_OUTPUT PlotIndicatorVS( VS_INPUT vIn	)
{
    VS_OUTPUT vOut = (VS_OUTPUT) 0;

    //Transform point
   	vOut.f4Position  = mul(mtxWorldViewProj, vIn.f4Position);
   	vOut.f2TexCoord1 = vIn.f2TexCoord1;
   	vOut.f2TexCoord2 = vIn.f2TexCoord2;
   	vOut.f2TexCoord3 = vIn.f2TexCoord1;

	return vOut;
}

//------------------------------------------------------------------------------------------------
// TEXTURES
//------------------------------------------------------------------------------------------------
texture IconOverlayTexture <string NTM = "base";>;
texture ButtonTexture <string NTM = "decal"; int NTMIndex = 0;>;
texture OverlayAlphaTexture<string NTM = "decal"; int NTMIndex = 1;>;

//------------------------------------------------------------------------------------------------
//                          SAMPLERS
//------------------------------------------------------------------------------------------------
sampler IconOverlay = sampler_state { Texture = (IconOverlayTexture); AddressU = Clamp; AddressV = Clamp; MagFilter = Linear; MipFilter = Linear; MinFilter = Linear; };
sampler IconButton = sampler_state { Texture = (ButtonTexture); AddressU = Clamp; AddressV = Clamp; MagFilter = Linear;	MipFilter = Linear;	MinFilter = Linear; };
sampler IconOverlayAlpha = sampler_state { Texture = (OverlayAlphaTexture); AddressU = Clamp; AddressV = Clamp;	MagFilter = Linear;	MipFilter = Linear;	MinFilter = Linear; };

//------------------------------------------------------------------------------------------------
float4 PlotIndicatorPS( VS_OUTPUT vIn ) : COLOR
{
	// Get Base textures
	float4 f4InnerAlpha = tex2D(IconOverlay, vIn.f2TexCoord1);
	float4 f4Button = tex2D(IconButton, vIn.f2TexCoord2);
	float4 f4OuterAlpha = tex2D(IconOverlayAlpha, vIn.f2TexCoord3);
	
	float4 f4FinalColor = lerp(f4MaterialEmissive, f4Button, f4InnerAlpha.a);
	f4FinalColor.a = f4OuterAlpha.a * f4MaterialDiffuse.a;
	return f4FinalColor;
}

//------------------------------------------------------------------------------------------------
//                          TECHNIQUES
//------------------------------------------------------------------------------------------------
technique PlotIndicatorShader <string shadername= "PlotIndicatorShader"; int implementation=0;>
{
    pass P0
    {
        // Enable depth writing
        ZEnable        = true;
        ZWriteEnable   = true;
        ZFunc          = lessequal;

        //Alpha Testing
        AlphaBlendEnable= true;
        AlphaTestEnable	= true;
        AlphaREf		= 0;
        AlphaFunc		= greater;
        SrcBlend        = SRCALPHA;
        DestBlend       = INVSRCALPHA;

   		VertexShader = compile vs_1_1 PlotIndicatorVS();
        PixelShader  = compile ps_1_1 PlotIndicatorPS();
	}
}
