//------------------------------------------------------------------------------------------------
//  $Header: $
//------------------------------------------------------------------------------------------------
//  *****************   FIRAXIS GAME ENGINE   ********************
//
//  FILE:    River
//
//  AUTHOR:  Jason Winnaker - 4/08/2005
//
//  PURPOSE: Draw river sparkles over base map.
//
//  Listing: fxc /Tvs_1_1 /ERiverVS /FcRiver.lst River.fx
//------------------------------------------------------------------------------------------------
//  Copyright (c) 2003 Firaxis Games, Inc. All rights reserved.
//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
//                          VARIABLES
//------------------------------------------------------------------------------------------------

// Transformations
float4x4 mtxWorldViewProj : WORLDVIEWPROJECTION;
float4x4 mtxWorld : WORLD;
float4x4 mtxWorldInv : WORLDINVERSE;
float4x4 mtxFOW : GLOBAL;
float3x3 mtxBaseTextureMat : GLOBAL;
float3 f3SunLightDir : GLOBAL;
float3 f3SunLightDiffuse : GLOBAL;
float3 f3SunAmbientColor : GLOBAL;

//------------------------------------------------------------------------------------------------
//                          VERTEX INPUT & OUTPUT FORMATS
//------------------------------------------------------------------------------------------------
struct VS_INPUT
{
	float3 f3Pos : POSITION;
	float2 f2BaseTex : TEXCOORD0;
	float2 f2UniformTex : TEXCOORD1;
	float3 f3Normal		: NORMAL;
	float3 f3Binormal	: BINORMAL;
	float3 f3Tangent	: TANGENT;
};

struct VS_OUTPUT
{
	float4 f4Pos : POSITION;
	float3 f3LightVec : COLOR0;
	float2 f2NormalTex : TEXCOORD0;
	float2 f2FOWTex : TEXCOORD1;
	float2 f2BaseTex : TEXCOORD2;
};

//------------------------------------------------------------------------------------------------
//                          VERTEX SHADER
//------------------------------------------------------------------------------------------------
VS_OUTPUT RiverVS(VS_INPUT vIn)
{
    VS_OUTPUT vOut = (VS_OUTPUT)0;

	//Transform point
   	vOut.f4Pos = mul(float4(vIn.f3Pos, 1), mtxWorldViewProj);
   	float4 f4WorldPos = mul(float4(vIn.f3Pos, 1), mtxWorld);

   	// compute the 3x3 tranform from tangent space to object space
	float3x3 objToTangentSpace;
	objToTangentSpace[0] = vIn.f3Tangent;
	objToTangentSpace[1] = vIn.f3Binormal; //possible sign problem
	objToTangentSpace[2] = vIn.f3Normal;

	//transform the light vector to object space
    float3 f3ObjectSpaceLight = mul(-f3SunLightDir, mtxWorldInv);
    float3 f3TangentSpaceLight = mul(objToTangentSpace, f3ObjectSpaceLight);
    vOut.f3LightVec = 0.5 * f3TangentSpaceLight + 0.5;

    // Set texture coordinates
    vOut.f2BaseTex = mul(float3(vIn.f2UniformTex, 1), mtxBaseTextureMat);
    vOut.f2NormalTex = vOut.f2BaseTex;
    vOut.f2FOWTex = mul(f4WorldPos, mtxFOW);

	return vOut;
}
//------------------------------------------------------------------------------------------------
//                          PIXEL SHADER
//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
// TEXTURES
//------------------------------------------------------------------------------------------------
texture BaseTexture <string NTM = "shader";  int NTMIndex = 0;>;
texture RiverNormalTexture <string NTM = "shader";  int NTMIndex = 1;>;
texture RiverFOWTexture <string NTM = "shader";  int NTMIndex = 2;>;

//------------------------------------------------------------------------------------------------
//                          SAMPLERS
//------------------------------------------------------------------------------------------------
sampler RiverNormalSampler = sampler_state { Texture = (RiverNormalTexture); AddressU = WRAP; AddressV = WRAP; MagFilter = Linear; MipFilter = None; MinFilter = Linear; srgbtexture = false; };
sampler RiverBase = sampler_state { Texture = (BaseTexture); AddressU  = WRAP; AddressV  = WRAP; MagFilter = LINEAR; MinFilter = LINEAR; MipFilter = LINEAR; srgbtexture = false; };
sampler RiverFOWSampler = sampler_state { Texture = (RiverFOWTexture); AddressU = Wrap; AddressV = Wrap; MagFilter = Linear; MipFilter = Linear; MinFilter = Linear; srgbtexture = false; };

//------------------------------------------------------------------------------------------------
//This applies the river base texture and normal map
float4 RiverPS( VS_OUTPUT vIn ) : COLOR
{
	float4 f4FinalColor = 0;

	// Get Base textures
	float3 f3Normal = tex2D(RiverNormalSampler, vIn.f2NormalTex).rgb;
	float4 f4FOW = tex2D(RiverFOWSampler, vIn.f2FOWTex);
	float4 f4Base = tex2D(RiverBase, vIn.f2BaseTex);

	// Uncompress normals
	f3Normal = (f3Normal - 0.5) * 2;
	float3 f3LightVec = (vIn.f3LightVec - 0.5) * 2;

	//calculate diffuse lighting
	float3 f3Diffuse = dot(f3Normal, f3LightVec) * f3SunLightDiffuse;

	//light base texture
	f4FinalColor = f4Base;
	f4FinalColor.rgb = saturate(f4FinalColor.rgb * (f3Diffuse + f3SunAmbientColor));

	// Get FOW
	f4FinalColor.rgb *= f4FOW.rgb;

	return f4FinalColor;
}
//------------------------------------------------------------------------------------------------
//                          TECHNIQUES
//------------------------------------------------------------------------------------------------
technique River_Shader< string shadername = "River_Shader"; int implementation=0;>
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
        AlphaRef		 = 0;
        AlphaFunc		 = GREATER;
        SrcBlend         = SRCALPHA;
        DestBlend        = INVSRCALPHA;

        // Set up textures and texture stage states
        VertexShader = compile vs_1_1 RiverVS();
        PixelShader  = compile ps_1_1 RiverPS();
    }
}
