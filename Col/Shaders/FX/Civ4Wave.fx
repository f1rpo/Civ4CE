//------------------------------------------------------------------------------------------------
//  $Header: $
//------------------------------------------------------------------------------------------------
//  *****************   FIRAXIS GAME ENGINE   ********************
//
//  FILE:    Waves
//
//  AUTHOR:  Tom Whittaker - 05
//
//  PURPOSE: Draw waves with fog/decal
//
//  Listing: fxc /Tvs_1_1 /EWaveVS /FcWave.lst Water.fx
//------------------------------------------------------------------------------------------------
//  Copyright (c) 2003 Firaxis Games, Inc. All rights reserved.
//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
//                          VARIABLES
//------------------------------------------------------------------------------------------------

// Transformations
float4x4 mtxWorldViewProj: WORLDVIEWPROJECTION;
float4x4 mtxWorld: WORLD;
float4x4 mtxFOW  : GLOBAL;
float4x4 mtxBaseTexture : TEXTRANSFORMBASE;
float4x4 mtxDecalTexture: TEXTRANSFORMDECAL;


//------------------------------------------------------------------------------------------------
//                          VERTEX INPUT & OUTPUT FORMATS
//------------------------------------------------------------------------------------------------
struct VS_INPUT
{
	float4 f4Pos     : POSITION;
	float4 f4Color   : COLOR;
	float2 f2BaseTex : TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 f4Pos     : POSITION;
	float4 f4Color   : COLOR;
	float2 f2BaseTex : TEXCOORD0;
	float2 f2DecalTex: TEXCOORD1;
	float2 f2FowTex  : TEXCOORD2;
};

//------------------------------------------------------------------------------------------------
//                          VERTEX SHADER
//------------------------------------------------------------------------------------------------
VS_OUTPUT WaveVS( VS_INPUT vIn )
{
    VS_OUTPUT vOut = (VS_OUTPUT)0;

	//Transform point
  	vOut.f4Pos  = mul(float4(vIn.f4Pos), mtxWorldViewProj);

    // Set texture coordinates
    vOut.f2BaseTex  = mul(float4(vIn.f2BaseTex,1,1),  mtxBaseTexture);
    vOut.f2DecalTex = mul(float4(vIn.f2BaseTex,1,1), mtxDecalTexture);

	//transform worldpos to FOW
    float3 P = mul(float4(vIn.f4Pos), (float4x3)mtxWorld);
    vOut.f2FowTex   = mul(float4(P,1),mtxFOW);

	vOut.f4Color = vIn.f4Color; //vertex alpha
	return vOut;
}
//------------------------------------------------------------------------------------------------
//                          PIXEL SHADER
//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------
// TEXTURES
//------------------------------------------------------------------------------------------------
texture BaseTexture		 <string NTM = "base";  int NTMIndex = 0;>;
texture DecalTexture	 <string NTM = "decal";  int NTMIndex = 0;>;
texture FOGTexture		 <string NTM = "shader";  int NTMIndex = 1;>;
//------------------------------------------------------------------------------------------------
//                          SAMPLERS
//------------------------------------------------------------------------------------------------
sampler WaveBase = sampler_state  { Texture = (BaseTexture);	   AddressU = Wrap; AddressV = Wrap; MagFilter = Linear; MipFilter = Linear; MinFilter = Linear; srgbtexture = false; };
sampler DecalBase = sampler_state  { Texture = (DecalTexture);	   AddressU = Wrap; AddressV = Wrap; MagFilter = Linear; MipFilter = Linear; MinFilter = Linear; srgbtexture = false; };
sampler Fog    	  = sampler_state  { Texture = (FOGTexture);	   AddressU = Clamp;  AddressV = Clamp;  MagFilter = Linear; MipFilter = Linear; MinFilter = Linear; srgbtexture = false; };
//------------------------------------------------------------------------------------------------
float4 WavePS( VS_OUTPUT vIn ) : COLOR
{
	float4	f4FinalColor = tex2D( WaveBase,  vIn.f2BaseTex );
	float4 f4Decal       = tex2D( DecalBase, vIn.f2DecalTex);
	f4FinalColor= lerp(f4FinalColor,f4Decal, f4Decal.a);

	f4FinalColor.a *= vIn.f4Color.a;

	// apply fog of war
	float3 f4FOWTex   = tex2D( Fog, vIn.f2FowTex ).rgb;
	f4FinalColor.rgb *= f4FOWTex;

	return f4FinalColor;
}
//------------------------------------------------------------------------------------------------
//                          TECHNIQUES
//------------------------------------------------------------------------------------------------

technique Wave_Shader < string shadername = "Wave_Shader"; int implementation=0; string Description = "Civ4 Wave shader"; >
{
    pass P0
    {
        // Enable depth writing
        ZEnable        = TRUE;
        ZWriteEnable   = false;
        ZFunc          = LESSEQUAL;

        // Enable alpha blending & testing
        AlphaBlendEnable = TRUE;
        AlphaTestEnable	 = true;
        SrcBlend         = SRCALPHA;
        DestBlend        = INVSRCALPHA;
        AlphaRef		 = 0;
        AlphaFunc		 = GREATER;

        // Set up textures and texture stage states
        VertexShader = compile vs_1_1 WaveVS();
        PixelShader  = compile ps_1_1 WavePS();
    }
}

