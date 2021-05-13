//------------------------------------------------------------------------------------------------
//  $Header: $
//------------------------------------------------------------------------------------------------
//  *****************   FIRAXIS GAME ENGINE   ********************
//
//  FILE:    ContourGroupVS11
//
//  AUTHOR:  Tom Whittaker - 12/10/2003
//
//  PURPOSE: Contour Group Vertex Shader
//                - Adjusts a vertex by a worldspace offset.
//                - Used to place objects patches on the terrain.
//
//  Listing: fxc /Tvs_1_1 /EContourVS /FcContourShader.lst ContourShader.fx
//------------------------------------------------------------------------------------------------
//  Copyright (c) 2003 Firaxis Games, Inc. All rights reserved.
//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
//                          VARIABLES
//------------------------------------------------------------------------------------------------

#include "Civ4ShadowMap.fx.hlsl"

float4x4 ViewProj: VIEWPROJECTION;

float4x4 mtxFOW  : GLOBAL;
float4 f4ForestOffsets[64] : GLOBAL;
float fFrameTime : GLOBAL;
int iShaderIndex : GLOBAL;

//------------------------------------------------------------------------------------------------
//                          VERTEX INPUT & OUTPUT FORMATS
//------------------------------------------------------------------------------------------------
struct VS_INPUT
{
   float4 f4Pos     : POSITION;
   float3 f3Normal  : NORMAL;
   float4 f4InstanceIndex: COLOR;
   float2 f2Tex     : TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 f4Pos                : POSITION;
	float3 f3Normal				: COLOR0;
	float2 f2BaseTex			: TEXCOORD0;
	float4 f4FOWTex				: TEXCOORD1;
	float4 f4ShadowTex			: TEXCOORD2;
};

float4 windir :GLOBAL= { 0.02, 0, 0, 0 };
//------------------------------------------------------------------------------------------------
//                          VERTEX SHADER
//------------------------------------------------------------------------------------------------
//
// - Adjust Z height based on Heights[Color.R]
//
VS_OUTPUT ContourVS( VS_INPUT vIn, uniform bool bSimpleShader )
{
    VS_OUTPUT vOut = (VS_OUTPUT)0;

	//Transform vertex and adjust height offset
	int index = vIn.f4InstanceIndex.r * 255 + 0.5; //expand and bias for integer truncation
	float4 f4ForestOffset = f4ForestOffsets[index];
	vIn.f4Pos.xyz *= f4ForestOffset.w; //scale
	float zHeight = vIn.f4Pos.z;
	vIn.f4Pos.xyz += f4ForestOffset.xyz;				//Adjut height by Height offset
	
	if(!bSimpleShader)
		vIn.f4Pos += sin(fFrameTime * 0.7 + index) * zHeight * windir;
	
	vOut.f4Pos = mul(vIn.f4Pos, ViewProj);

    // Set texture coordinates
    vOut.f2BaseTex = vIn.f2Tex;
    vOut.f4FOWTex = mul(vIn.f4Pos, mtxFOW);
    vOut.f4ShadowTex = getShadowTexCoord(vIn.f4Pos);
    vOut.f3Normal = normalize(vIn.f3Normal);

    return vOut;
}

//------------------------------------------------------------------------------------------------
//                          PIXEL SHADER
//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------
//                          SAMPLERS
//------------------------------------------------------------------------------------------------
texture BaseTexture <string NTM = "base";>;
texture ShadowMapTexture <string NTM = "shader";  int NTMIndex = 0;>;
texture FogOfWarMap <string NTM = "shader"; int NTMIndex = 1; >;

sampler ContourBase      = sampler_state{ Texture = (BaseTexture);  AddressU = wrap; AddressV = wrap; MagFilter = linear; MinFilter = linear; MipFilter = linear; srgbtexture = false;};
sampler ShadowMap = sampler_state { Texture = (ShadowMapTexture);  AddressU = CLAMP; AddressV = CLAMP; MagFilter = LINEAR; MipFilter = NONE; MinFilter = LINEAR; srgbtexture = false;};
sampler FogOfWarSampler  = sampler_state{ Texture = (FogOfWarMap);  AddressU = wrap; AddressV = wrap; MagFilter = linear; MinFilter = linear; MipFilter = linear; srgbtexture = false;};

float4 ContourPS ( VS_OUTPUT vIn ) : COLOR
{
	//base * FOW
	float4 f4Base = tex2D(ContourBase, vIn.f2BaseTex);
	float4 f4FOW = tex2D(FogOfWarSampler, vIn.f4FOWTex.xy);
	float4 f4Result = f4Base * f4FOW;
	
	float NDotL = dot(vIn.f3Normal, -f3SunLightDir);
	float3 f3Diffuse = saturate(NDotL) * f3SunLightDiffuse;

	//lighting
	float fShadowFactor = getShadowFactor( ShadowMap, vIn.f4ShadowTex, float4(0, 1, 0, 0) );
	float3 f3Lighting = f3Diffuse * fShadowFactor + f3SunAmbientColor;
	f4Result.rgb *= f3Lighting;

	return f4Result;
}

float4 ContourPS_Simple ( VS_OUTPUT vIn ) : COLOR
{
	//base
	float4 f4Result = tex2D(ContourBase, vIn.f2BaseTex);

	//lighting
	float NDotL = dot(vIn.f3Normal, -f3SunLightDir);
	float3 f3Diffuse = saturate(NDotL) * f3SunLightDiffuse;
	float3 f3Lighting = f3Diffuse + f3SunAmbientColor;
	f4Result.rgb *= f3Lighting;

	return f4Result;
}

float4 ContourPS_Shadow ( VS_OUTPUT vIn ) : COLOR
{
	//base * FOW
	float4 f4Base = tex2D(ContourBase, vIn.f2BaseTex);
	return float4(f4Base.a, 0, f4Base.a, f4Base.a);
}

//------------------------------------------------------------------------------------------------
//                          TECHNIQUES //bool UsesNIRenderState = true;>
//------------------------------------------------------------------------------------------------
Pixelshader PSArray11[4] =
{
	compile ps_1_1 ContourPS_Simple(),
	compile ps_1_1 ContourPS(),
	compile ps_1_1 ContourPS(),
	compile ps_1_1 ContourPS_Shadow()
};

technique Contour_Shader< string shadername = "Contour_Shader"; int implementation=0;>
{
    pass P0
    {
		// Enable depth writing
        ZEnable        = TRUE;
        ZWriteEnable   = TRUE;
        ZFunc          = LESSEQUAL;

        // Enable alpha blending & testing
        AlphaBlendEnable = TRUE;
        AlphaTestEnable	 = TRUE;
        SrcBlend         = SRCALPHA;
        DestBlend        = INVSRCALPHA;
        AlphaRef		 = 100;
        AlphaFunc		 = GREATER;

		VertexShader = compile vs_1_1 ContourVS(false);
		PixelShader	 = (PSArray11[iShaderIndex]);
    }
}

//------------------------------------------------------------------------------------------------
//                          TECHNIQUES //bool UsesNIRenderState = true;>
//------------------------------------------------------------------------------------------------
technique ContourSimple_Shader< string shadername = "ContourSimple_Shader"; int implementation=0;>
{
    pass P0
    {
		// Enable depth writing
        ZEnable        = TRUE;
        ZWriteEnable   = TRUE;
        ZFunc          = LESSEQUAL;

        // Enable alpha blending & testing
        AlphaBlendEnable = TRUE;
        AlphaTestEnable	 = TRUE;
        SrcBlend         = SRCALPHA;
        DestBlend        = INVSRCALPHA;
        AlphaRef		 = 100;
        AlphaFunc		 = GREATER;

		VertexShader = compile vs_1_1 ContourVS(true);
		PixelShader	 = (PSArray11[iShaderIndex]);
    }
}
