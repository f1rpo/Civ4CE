///------------------------------------------------------------------------------------------------
//  $Header: $
//------------------------------------------------------------------------------------------------
//  *****************   FIRAXIS GAME ENGINE   ********************
//
//  FILE:    Terrain Tile Splat Shader
//
//  AUTHOR:  Tom Whittaker - 02/09
//
//  PURPOSE: Splat Tile Terrain Shader - Base Texute + LM FOW
//			 todotw: right now supports one base texture. Similar to aggregate
//
//  Listing: fxc /Tvs_1_1 /ETerrainVS /FcTerrainVS.lst Terrain.fx
//------------------------------------------------------------------------------------------------
//  Copyright (c) 2003 Firaxis Games, Inc. All rights reserved.
//------------------------------------------------------------------------------------------------

#include "Civ4ShadowMap.fx.hlsl"

// Transformations
float4x4	mtxWorldViewProj	: WORLDVIEWPROJECTION;
float4x4	mtxInvView : INVVIEWTRANSPOSE;
float4x4    mtxWorld   : WORLD;
float4x4	mtxFOW     : GLOBAL;
float2		f2DetailTexScaling : GLOBAL;
float		fFrameTime : GLOBAL;
int			iShaderIndex : GLOBAL;

//------------------------------------------------------------------------------------------------
// VERTEX OUTPUT FORMATS
//------------------------------------------------------------------------------------------------
struct VS_INPUT
{
	float3 f3Position   : POSITION;
	float3 f3Normal		: NORMAL;
	float2 f2BaseTex    : TEXCOORD0;
};

struct VS_OUTPUT
{
	float4	f4Position		: POSITION;
	float4	f4BaseDetailTex	: TEXCOORD0;
    float2	f2FOWTex		: TEXCOORD1;
	float4	f4ShadowTex		: TEXCOORD2;
	float3	f3Normal		: TEXCOORD3;
	float3  f3EyeVector		: TEXCOORD4;
	float3  f3ReflectEye	: TEXCOORD5;
};

struct VS_OUTPUT_SIMPLE
{
	float4	f4Position		: POSITION;
	float4	f4Lighting		: COLOR1;
	float2	f2BaseTex		: TEXCOORD0;
    float2	f2DetailTex		: TEXCOORD1;
    float2	f2FOWTex		: TEXCOORD2;
	float4	f4ShadowTex		: TEXCOORD3;
};

//------------------------------------------------------------------------------------------------
//                          VERTEX SHADER
//------------------------------------------------------------------------------------------------

VS_OUTPUT VSTerrain_Tile( VS_INPUT vIn )
{
	VS_OUTPUT vOut = (VS_OUTPUT)0;

	//Transform position
	vOut.f4Position  = mul(float4(vIn.f3Position, 1), mtxWorldViewProj);
    float4 f4WorldPosition = mul(float4(vIn.f3Position, 1), mtxWorld);

    //eye vector
	float4 f4CameraPosition = mul(mtxInvView, float4(0, 0, 0, 1));
	float3 f3EyeVector = normalize(f4CameraPosition.xyz - f4WorldPosition.xyz);
	vOut.f3EyeVector = f3EyeVector;

	//reflected eye
	float3 f3ReflectEye = reflect(-f3EyeVector, float3(0, 0, 1));
	f3ReflectEye.y *= -1; //texture v is opposite world y
	f3ReflectEye = 0.5 * f3ReflectEye + 0.5;
	vOut.f3ReflectEye = f3ReflectEye;

	// Copy over the texture coordinates
	vOut.f4BaseDetailTex.xy = vIn.f2BaseTex;
	vOut.f4BaseDetailTex.zw = f4WorldPosition.xy * f2DetailTexScaling;

	vOut.f2FOWTex      = mul(f4WorldPosition, mtxFOW);
	vOut.f4ShadowTex   = getShadowTexCoord(f4WorldPosition);
	vOut.f3Normal = normalize(vIn.f3Normal);

	return vOut;
}

VS_OUTPUT_SIMPLE VSTerrain_TileSimple( VS_INPUT vIn )
{
	VS_OUTPUT_SIMPLE vOut = (VS_OUTPUT_SIMPLE)0;

	//Transform position
	vOut.f4Position  = mul(float4(vIn.f3Position, 1), mtxWorldViewProj);
    float4 f4WorldPosition = mul(float4(vIn.f3Position, 1), mtxWorld);

	// Copy over the texture coordinates
	vOut.f2BaseTex = vIn.f2BaseTex;
	vOut.f2DetailTex = f4WorldPosition.xy * f2DetailTexScaling;

	vOut.f2FOWTex      = mul(f4WorldPosition, mtxFOW);
	vOut.f4ShadowTex   = getShadowTexCoord(f4WorldPosition);

	float3 f3Normal = normalize(vIn.f3Normal);
	float fDiffuseDot = max(dot(f3Normal, -f3SunLightDir), 0);
	float3 f3Diffuse = fDiffuseDot * f3SunLightDiffuse;
	vOut.f4Lighting = float4(f3Diffuse, 1);

	return vOut;
}

//------------------------------------------------------------------------------------------------
//                          PIXEL SHADER
//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------
// TEXTURES
//------------------------------------------------------------------------------------------------
texture TerrainBaseTexture	   <string NTM = "shader";  int NTMIndex = 0;>;
texture TerrainDetailTexture   <string NTM = "shader";  int NTMIndex = 1;>;
texture TerrainFOWarTexture    <string NTM = "shader";  int NTMIndex = 2;>;
texture ShadowMapTexture	   <string NTM = "shader";  int NTMIndex = 3;>;
texture EnvironmentMapTexture <string NTM = "shader";  int NTMIndex = 4;>;

//------------------------------------------------------------------------------------------------
// SAMPLERS
//------------------------------------------------------------------------------------------------											//NONE!
sampler TerrainBase : register(s0) = sampler_state  { Texture = (TerrainBaseTexture);  AddressU = Clamp; AddressV = Clamp; MagFilter = Linear; MipFilter = Linear; MinFilter = Linear; srgbtexture = false;};
sampler TerrainBaseNoMips : register(s0) = sampler_state  { Texture = (TerrainBaseTexture);  AddressU = Clamp; AddressV = Clamp; MagFilter = POINT; MipFilter = None; MinFilter = POINT; srgbtexture = false;};
sampler TerrainDetail : register(s1) = sampler_state { Texture = (TerrainDetailTexture  );  AddressU = Wrap; AddressV = Wrap; MagFilter = Linear; MipFilter = Linear; MinFilter = Linear; srgbtexture = false;};
sampler TerrainDetailNoMips : register(s1) = sampler_state { Texture = (TerrainDetailTexture  );  AddressU = Wrap; AddressV = Wrap; MagFilter = POINT; MipFilter = NONE; MinFilter = POINT; srgbtexture = false;};
sampler TerrainFOWar = sampler_state  { Texture = (TerrainFOWarTexture);  AddressU = Wrap; AddressV = Wrap; MagFilter = Linear; MipFilter = Linear; MinFilter = Linear; srgbtexture = false;};
sampler ShadowMap = sampler_state { Texture = (ShadowMapTexture);  AddressU = CLAMP; AddressV = CLAMP; MagFilter = LINEAR; MipFilter = NONE; MinFilter = LINEAR; srgbtexture = false;};
sampler EnvironmentMap = sampler_state { Texture = (EnvironmentMapTexture);  AddressU = CLAMP; AddressV = CLAMP; AddressW = CLAMP; MagFilter = LINEAR; MipFilter = LINEAR; MinFilter = LINEAR; srgbtexture = false;};

float4 PSTerrain ( VS_OUTPUT Input, uniform sampler TerrainBaseSampler, uniform sampler TerrainDetailSampler ) : COLOR
{
	// Read all our base textures, the grid and FOW texture
	float4 f4BaseTex = tex2D( TerrainBaseSampler,  Input.f4BaseDetailTex.xy );
	float4 f4DetailTex = tex2D( TerrainDetailSampler, Input.f4BaseDetailTex.zw );
	float3 f3FOWTex   = tex2D( TerrainFOWar, Input.f2FOWTex );

	//environment
	float4 f4Environment = tex2D( EnvironmentMap, Input.f3ReflectEye.xy );

	//shadow
	float fShadowFactor = getShadowFactor( ShadowMap, Input.f4ShadowTex, float4(1, 0, 0, 0) );

	//diffuse
	float3 f3Normal = normalize(Input.f3Normal);
	float fDiffuseDot = max(dot(f3Normal, -f3SunLightDir), 0);
	float3 f3Diffuse = fDiffuseDot * f3SunLightDiffuse * fShadowFactor + f3SunAmbientColor;

	//specular
	float3 f3HalfVector = normalize(Input.f3EyeVector - f3SunLightDir);
	float fSpecularDot = max(dot(f3Normal, f3HalfVector), 0);
	float3 f3Specular = pow(fSpecularDot, 4) * f3SunLightDiffuse * fShadowFactor;

	//combine textures
	float4 f4FinalColor = f4BaseTex;
	f4FinalColor.rgb += f4DetailTex.rgb - 0.5f;
	f4FinalColor.rgb *= f3Diffuse;
	f4FinalColor.rgb += 0.1 * f3Specular;
	f4FinalColor.rgb += f4DetailTex.a * f4Environment;
	f4FinalColor.rgb *= f3FOWTex;				//FOW textures

	// Return the result
	return f4FinalColor;
}

float4 PSTerrainSimple ( VS_OUTPUT_SIMPLE Input, uniform sampler TerrainBaseSampler, uniform sampler TerrainDetailSampler ) : COLOR
{
	// Read all our base textures, the grid and FOW texture
	float4 f4BaseTex = tex2D( TerrainBaseSampler,  Input.f2BaseTex );
	float4 f4DetailTex = tex2D( TerrainDetailSampler, Input.f2DetailTex );
	float3 f3FOWTex   = tex2D( TerrainFOWar, Input.f2FOWTex );

	//shadow
	float fShadowFactor = getShadowFactor( ShadowMap, Input.f4ShadowTex, float4(1, 0, 0, 0) );
	float3 f3Diffuse = Input.f4Lighting.rgb * fShadowFactor + f3SunAmbientColor;

	//combine textures
	float4 f4FinalColor = f4BaseTex;
	f4FinalColor.rgb = saturate(f4FinalColor.rgb + f4DetailTex.rgb - 0.5);
	f4FinalColor.rgb *= f3Diffuse;
	f4FinalColor.rgb *= f3FOWTex;				//FOW textures

	// Return the result
	return f4FinalColor;
}

float4 PSTerrain_Shadow ( VS_OUTPUT Input ) : COLOR
{
	return float4(1, 1, 1, 1);
}

//------------------------------------------------------------------------------------------------
//                          TECHNIQUES
//<bool UsesNIRenderState = true;>
//------------------------------------------------------------------------------------------------
Pixelshader PSArray20[4] =
{
	compile ps_2_0 PSTerrain(TerrainBase, TerrainDetail),
	compile ps_2_0 PSTerrain(TerrainBase, TerrainDetail),
	compile ps_2_0 PSTerrain(TerrainBaseNoMips, TerrainDetailNoMips),
	compile ps_2_0 PSTerrain_Shadow()
};

technique TerrainShader< string shadername= "TerrainShader"; int implementation=0;>
{
	pass P0
	{
		// Enable depth writing
		ZEnable				= TRUE;
		ZWriteEnable		= TRUE;
		ZFunc				= LESSEQUAL;

		// Disable alpha blending and testing
		AlphaBlendEnable = true;
		AlphaTestEnable	 = true;
		AlphaRef         = 0;
		AlphaFunc        = GREATER;
		SrcBlend		 = SrcAlpha;
		DestBlend		 = InvSrcAlpha;

		// Set vertex and pixel shaders
		VertexShader = compile vs_2_0 VSTerrain_Tile();
		PixelShader	 = (PSArray20[iShaderIndex]);
	}
}

Pixelshader PSArray11[4] =
{
	compile ps_1_1 PSTerrainSimple(TerrainBase, TerrainDetail),
	compile ps_1_1 PSTerrainSimple(TerrainBase, TerrainDetail),
	compile ps_1_1 PSTerrainSimple(TerrainBaseNoMips, TerrainDetailNoMips),
	compile ps_1_1 PSTerrain_Shadow()
};

technique TerrainShaderSimple< string shadername= "TerrainShaderSimple"; int implementation=0;>
{
	pass P0
	{
		// Enable depth writing
		ZEnable				= TRUE;
		ZWriteEnable		= TRUE;
		ZFunc				= LESSEQUAL;

		// Disable alpha blending and testing
		AlphaBlendEnable = true;
		AlphaTestEnable	 = true;
		AlphaRef         = 0;
		AlphaFunc        = GREATER;
		SrcBlend		 = SrcAlpha;
		DestBlend		 = InvSrcAlpha;

		// Set vertex and pixel shaders
		VertexShader = compile vs_1_1 VSTerrain_TileSimple();
		PixelShader	 = (PSArray11[iShaderIndex]);
	}
}
