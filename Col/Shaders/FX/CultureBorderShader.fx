//------------------------------------------------------------------------------------------------
//  $Header: $
//------------------------------------------------------------------------------------------------
//  *****************   FIRAXIS GAME ENGINE   ********************
//
//  FILE:    CultureBorder
//
//  AUTHOR:  Jason Winokur - 3/29/2005
//
//  PURPOSE: Draw culture with fog/decal(borders)
//
//  Listing: fxc /Tvs_1_1 /EWaterVS /FcWater.lst Water.fx
//------------------------------------------------------------------------------------------------
//  Copyright (c) 2003 Firaxis Games, Inc. All rights reserved.
//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
//                          VARIABLES
//------------------------------------------------------------------------------------------------

// Transformations
float4x4 mtxWorldViewProj: WORLDVIEWPROJECTION;
float4x4 mtxWorld : WORLD;
float4x4 mtxFOW	: GLOBAL;

float4x4 mtxBorderTextureMat1 : GLOBAL;
float4x4 mtxBorderTextureMat2 : GLOBAL;
float4x4 mtxBorderTextureMat3 : GLOBAL;
float4x4 mtxBorderTextureMat4 : GLOBAL;
float4 f4BorderColor1 : GLOBAL;
float4 f4BorderColor2 : GLOBAL;
float4 f4BorderColor3 : GLOBAL;
float4 f4BorderColor4 : GLOBAL;

//------------------------------------------------------------------------------------------------
//                          VERTEX INPUT & OUTPUT FORMATS
//------------------------------------------------------------------------------------------------
struct VS_INPUT
{
   float3 f3Pos     : POSITION;
   float2 f2BaseTex : TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 f4Pos      : POSITION;
	float2 f2BaseTex1 : TEXCOORD0;
	float2 f2BaseTex2 : TEXCOORD1;
	float2 f2FOWTex	  : TEXCOORD2;
};

VS_OUTPUT CultureBorderVS( VS_INPUT vIn, uniform bool bSecondPass )
{
    VS_OUTPUT vOut = (VS_OUTPUT) 0;

	//Transform point
   	vOut.f4Pos  = mul(float4(vIn.f3Pos, 1), mtxWorldViewProj);
   	float4 f4WorldPos = mul(float4(vIn.f3Pos, 1), mtxWorld);
   	vOut.f2FOWTex = mul(f4WorldPos, mtxFOW);

    // Set texture coordinates
    if(!bSecondPass)
    {
		vOut.f2BaseTex1 = mul(float3(vIn.f2BaseTex,1), mtxBorderTextureMat1);
		vOut.f2BaseTex2 = mul(float3(vIn.f2BaseTex,1), mtxBorderTextureMat2);
	}
	else
	{
		vOut.f2BaseTex1 = mul(float3(vIn.f2BaseTex,1), mtxBorderTextureMat3);
		vOut.f2BaseTex2 = mul(float3(vIn.f2BaseTex,1), mtxBorderTextureMat4);
	}

	return vOut;
}

//------------------------------------------------------------------------------------------------
//                          PIXEL SHADER
//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
// TEXTURES
//------------------------------------------------------------------------------------------------
texture BaseTexture <string NTM = "base";>;
texture TerrainFOWarTexture <string NTM = "shader";  int NTMIndex = 1;>;

//------------------------------------------------------------------------------------------------
//                          SAMPLERS
//------------------------------------------------------------------------------------------------
sampler CultureBorderBase = sampler_state { Texture = (BaseTexture); AddressU  = CLAMP; AddressV  = CLAMP; MagFilter = LINEAR; MinFilter = LINEAR; MipFilter = NONE; srgbtexture = false; };
sampler TerrainFOWar = sampler_state { Texture = (TerrainFOWarTexture); AddressU = Wrap; AddressV = Wrap; MagFilter = Linear; MipFilter = Linear; MinFilter = Linear; srgbtexture = false; };

//------------------------------------------------------------------------------------------------
float4 CultureBorderPS( VS_OUTPUT vIn, uniform bool bSecondPass ) : COLOR
{
	//mixes 2 textures
	float4	f4FinalColor = 0.0f;

	// Get Base textures
	float4 f4Base1 = tex2D( CultureBorderBase, vIn.f2BaseTex1 );
	float4 f4Base2 = tex2D( CultureBorderBase, vIn.f2BaseTex2 );

	float4 f4Color1, f4Color2;
	if(!bSecondPass)
	{
		f4Color1 = f4Base1 * f4BorderColor1;
		f4Color2 = f4Base2 * f4BorderColor2;
	}
	else
	{
		f4Color1 = f4Base1 * f4BorderColor3;
		f4Color2 = f4Base2 * f4BorderColor4;
	}

	f4Color1.a *= f4Color1.a;
	f4Color2.a *= f4Color2.a;
	f4FinalColor = f4Color1 * f4Color1.a + f4Color2 * f4Color2.a;
	f4FinalColor.a = f4Color1.a + f4Color2.a;
		
	//FOW
	float4 f4FOWTex = tex2D( TerrainFOWar, vIn.f2FOWTex );
	f4FinalColor.rgb *= f4FOWTex.rgb;

	return f4FinalColor;
}

technique TCultureBorder_Shader4T
{
    pass P0
    {
        // Enable depth writing
        ZEnable        = TRUE;
        ZWriteEnable   = FALSE;
        ZFunc          = LESSEQUAL;

        // Enable alpha blending & testing
        AlphaBlendEnable = TRUE;
        AlphaTestEnable	 = TRUE;
		AlphaRef		 = 1;
        AlphaFunc		 = GREATER;
        SrcBlend         = ONE;
        DestBlend        = INVSRCALPHA;

        // Set up textures and texture stage states
        VertexShader = compile vs_1_1 CultureBorderVS(false);
        PixelShader  = compile ps_1_1 CultureBorderPS(false);
    }

	pass P1
    {
        // Set up textures and texture stage states
        VertexShader = compile vs_1_1 CultureBorderVS(true);
        PixelShader  = compile ps_1_1 CultureBorderPS(true);
    }
}

technique TCultureBorder_Shader2T
{
    pass P0
    {
        // Enable depth writing
        ZEnable        = TRUE;
        ZWriteEnable   = FALSE;
        ZFunc          = LESSEQUAL;

        // Enable alpha blending & testing
        AlphaBlendEnable = TRUE;
        AlphaTestEnable	 = TRUE;
		AlphaRef		 = 1;
        AlphaFunc		 = GREATER;
        SrcBlend         = ONE;
        DestBlend        = INVSRCALPHA;

        // Set up textures and texture stage states
        VertexShader = compile vs_1_1 CultureBorderVS(false);
        PixelShader  = compile ps_1_1 CultureBorderPS(false);
    }
}
