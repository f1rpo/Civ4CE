//  $Header: $
//------------------------------------------------------------------------------------------------
//
//  ***************** CIV4 GAME ENGINE   ********************
//
//! \file		Civ4Bloom.fx
//! \author		Bart Muzzin -- 04-28-2005
//! \brief		Blooming postprocess effect
//
//------------------------------------------------------------------------------------------------
//  Copyright (c) 2005 Firaxis Games, Inc. All rights reserved.
//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
// VARIABLES
//------------------------------------------------------------------------------------------------

static const float fTexelSize = (2.0/256.0);
float3 g_f3BloomFactors : GLOBAL = { 0.0, 0.7, 3.16 };		// 1st component = don't do anything anymore
																// 2nd component = The luminosity scaling range minimum value
																// 3rd component = The inverse of the luminosity remapping range
float3 g_f3BloomColorChannelMix : GLOBAL = { 1.8, 1.8, 2.0 };	// The amount of each component of the bloom included in the final image

//------------------------------------------------------------------------------------------------

float4x4	mtxWorldViewProj	: WORLDVIEWPROJECTION;

//------------------------------------------------------------------------------------------------
// Kernel size = 13

static const float g_afBlurFactorKS13[13] =
{
	0.017996989,
	0.033159046,
	0.054670025,
	0.080656908,
	0.106482669,
	0.125794409,
	0.13298076,
	0.125794409,
	0.106482669,
	0.080656908,
	0.054670025,
	0.033159046,
	0.017996989
};

static const float2 g_afOffsetXKS13[13] =
{
	{ -6.0 * fTexelSize, 0 },
	{ -5.0 * fTexelSize, 0 },
	{ -4.0 * fTexelSize, 0 },
	{ -3.0 * fTexelSize, 0 },
	{ -2.0 * fTexelSize, 0 },
	{ -1.0 * fTexelSize, 0 },
	{  0.0 * fTexelSize, 0 },
	{  1.0 * fTexelSize, 0 },
	{  2.0 * fTexelSize, 0 },
	{  3.0 * fTexelSize, 0 },
	{  4.0 * fTexelSize, 0 },
	{  5.0 * fTexelSize, 0 },
	{  6.0 * fTexelSize, 0 }
};

static const float2 g_afOffsetYKS13[13] =
{
	{ 0, -6.0 * fTexelSize },
	{ 0, -5.0 * fTexelSize },
	{ 0, -4.0 * fTexelSize },
	{ 0, -3.0 * fTexelSize },
	{ 0, -2.0 * fTexelSize },
	{ 0, -1.0 * fTexelSize },
	{ 0,  0.0 * fTexelSize },
	{ 0,  1.0 * fTexelSize },
	{ 0,  2.0 * fTexelSize },
	{ 0,  3.0 * fTexelSize },
	{ 0,  4.0 * fTexelSize },
	{ 0,  5.0 * fTexelSize },
	{ 0,  6.0 * fTexelSize }
};


//------------------------------------------------------------------------------------------------
// TEXTURES
//------------------------------------------------------------------------------------------------
texture BaseTexture< string NTM = "base";>;
texture BlurHighPassTexture< string NTM = "detail";>;

//------------------------------------------------------------------------------------------------
// SAMPLERS
//------------------------------------------------------------------------------------------------
sampler BaseSampler = sampler_state { Texture = (BaseTexture); AddressU  = CLAMP; AddressV  = CLAMP; MagFilter = LINEAR; MinFilter = LINEAR; MipFilter = LINEAR; };
sampler HighPassTexture = sampler_state { Texture = (BlurHighPassTexture); AddressU  = CLAMP; AddressV  = CLAMP; MagFilter = LINEAR; MinFilter = LINEAR; MipFilter = LINEAR; };

//------------------------------------------------------------------------------------------------
// STRUCTURES
//------------------------------------------------------------------------------------------------
struct BLOOMOUTPUT
{
	float4 f4Position : POSITION;
	float2 f2TexCoords : TEXCOORD0;
};

//------------------------------------------------------------------------------------------------
// SHADERS
//------------------------------------------------------------------------------------------------

BLOOMOUTPUT VSBloom( BLOOMOUTPUT vIn	)
{
	BLOOMOUTPUT vOut = vIn;
	vOut.f4Position = mul(vOut.f4Position, mtxWorldViewProj);
    return vOut;
}

float4 PSHighPass( BLOOMOUTPUT kOutput ) : COLOR0
{
	float3 f3Color = tex2D( BaseSampler, kOutput.f2TexCoords );
	f3Color = f3Color * ( f3Color - g_f3BloomFactors.yyy ) * g_f3BloomFactors.z;

	return float4( saturate(f3Color), 1.0 );
}

float4 PSBlurH_KS13( BLOOMOUTPUT kOutput ) : COLOR0
{
	float3 f3Color = 0;
	for ( int i = 0; i < 13; i++ )
	{
		f3Color += tex2D( BaseSampler, kOutput.f2TexCoords + g_afOffsetXKS13[i] ) * g_afBlurFactorKS13[i];
	}
	return float4( f3Color, 1.0 );
}

float4 PSBlurVRecombine_KS13( BLOOMOUTPUT kOutput ) : COLOR0
{
	float3 f3Color = 0;
	for ( int i = 0; i < 13; i++ )
	{
		f3Color += tex2D( HighPassTexture, kOutput.f2TexCoords + g_afOffsetYKS13[i] ) * g_afBlurFactorKS13[i];
	}
	
	float3 f3BaseColor = tex2D( BaseSampler, kOutput.f2TexCoords );
	float fLuminosity = (f3Color.r + f3Color.g + f3Color.b) /3.0;
	return float4( f3BaseColor + fLuminosity * (f3Color * g_f3BloomColorChannelMix), 1.0 );
}

//------------------------------------------------------------------------------------------------
// TECHNIQUES
//------------------------------------------------------------------------------------------------

technique TBloom_T1
{
	pass HighPass
	{
        // Disable depth writing (just writing a quad)
        ZEnable        = FALSE;
        ZWriteEnable   = FALSE;

        // Disable alpha blending & testing - everything is opaque
        AlphaBlendEnable = FALSE;
        AlphaTestEnable	 = FALSE;

		VertexShader = compile vs_2_0 VSBloom( );
        PixelShader = compile ps_2_0 PSHighPass( );
	}
}

technique TBloom_T2
{
	pass BlurH
	{
        // Disable depth writing (just writing a quad)
        ZEnable        = FALSE;
        ZWriteEnable   = FALSE;

        // Disable alpha blending & testing - everything is opaque
        AlphaBlendEnable = FALSE;
        AlphaTestEnable	 = FALSE;

		VertexShader = compile vs_2_0 VSBloom( );
       	PixelShader = compile ps_2_0 PSBlurH_KS13( );
	}
}

technique TBloom_T3
{
	pass BlurH
	{
        // Disable depth writing (just writing a quad)
        ZEnable        = FALSE;
        ZWriteEnable   = FALSE;

        // Disable alpha blending & testing - everything is opaque
        AlphaBlendEnable = FALSE;
        AlphaTestEnable	 = FALSE;

		VertexShader = compile vs_2_0 VSBloom( );
        PixelShader = compile ps_2_0 PSBlurVRecombine_KS13( );
	}
}
