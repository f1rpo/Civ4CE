//------------------------------------------------------------------------------------------------
//  $Header: $
//------------------------------------------------------------------------------------------------
//  *****************   FIRAXIS GAME ENGINE   ********************
//
//  FILE:    FlagDecal.fx
//
//  AUTHOR:  Bart Muzzin
//			 Tom Whittaker
//
//  PURPOSE: Draw the flags with primary color and modulated decal texture
//
//------------------------------------------------------------------------------------------------
//  Copyright (c) 2005 Firaxis Games, Inc. All rights reserved.
//------------------------------------------------------------------------------------------------

float4x4 mtxWorldViewProj: WORLDVIEWPROJECTION;
float4x4 mtxViewProj: VIEWPROJECTION;
float4x4 mtxWorldView: WORLDVIEW;
float4x4 mtxWorld : WORLD;

//Unit Light
float3      f3UnitLightDir: GLOBAL		= {-0.6428f, 0.24603, -0.7254f};
float3      f3UnitLightDiffuse: GLOBAL	= {1.0f, 1.0f, 1.0f};
float3      f3UnitAmbientColor: GLOBAL	= { 0.2471f, 0.2353f,  0.3020f};

static const int MAX_BONES = 20;
float4x3 mtxWorldBones[MAX_BONES] : SKINBONEMATRIX3; //world space bone matrix

//------------------------------------------------------------------------------------------------
// TEXTURES
//------------------------------------------------------------------------------------------------
texture BaseTexture <string NTM = "base";>;
texture DecalTexture < string NTM = "detail";>;
texture BaseColor <string NTM = "decal"; int NTMIndex = 0;>;
texture DecalColor <string NTM = "decal"; int NTMIndex = 1;>;
texture GlossMap <string NTM = "gloss"; int NTMIndex = 0;>;
texture EnvironMap <string NTM= "glow"; int NTMIndex = 0;>;

//------------------------------------------------------------------------------------------------
//                          SAMPLERS
//------------------------------------------------------------------------------------------------
sampler BaseSampler = sampler_state { Texture = (BaseTexture); AddressU  = WRAP; AddressV  = WRAP; MagFilter = LINEAR; MinFilter = LINEAR; MipFilter = LINEAR; srgbtexture = false; };
sampler BaseColorSampler = sampler_state { Texture = (BaseColor); AddressU  = WRAP; AddressV  = WRAP; MagFilter = LINEAR; MinFilter = LINEAR; MipFilter = LINEAR; srgbtexture = false; };
sampler DecalSampler = sampler_state { Texture = (DecalTexture); AddressU = Clamp; AddressV = Clamp; MagFilter = Linear; MipFilter = Linear; MinFilter = Linear; srgbtexture = false; };
sampler DecalColorSampler = sampler_state { Texture = (DecalColor);	AddressU = Clamp; AddressV = Clamp; MagFilter = Linear;	MipFilter = Linear; MinFilter = Linear; srgbtexture = false;};
sampler GlossSampler = sampler_state { Texture = (GlossMap); ADDRESSU = wrap; ADDRESSV = wrap; MAGFILTER = linear; MINFILTER = linear; MIPFILTER = linear; srgbtexture = false; };
sampler EnvironmentMapSampler = sampler_state { Texture = (EnvironMap); ADDRESSU = wrap; ADDRESSV = wrap; MAGFILTER = linear; MINFILTER = linear; MIPFILTER = linear; srgbtexture = false; };

struct VS_INPUT
{
    float4 f4Position	: POSITION;
    float3 f3Normal		: NORMAL;
    float2 f2TexCoords    : TEXCOORD0;
    float4 f4BlendWeights : BLENDWEIGHT;
    float4 f4BlendIndices : BLENDINDICES;
};

struct VS_OUTPUT
{
	float4 f4Position : POSITION;
	float2 f2TexCoord0 : TEXCOORD0;
	float2 f2TexCoord1 : TEXCOORD1;
	float2 f2TexCoord2 : TEXCOORD2;
	float2 f2TexCoord3 : TEXCOORD3;
};

struct VS_OUTPUT_NOLIGHTING
{
	float4 f4Position : POSITION;
	float2 f2TexCoords : TEXCOORD0;
};

struct VS_OUTPUTGLOSS
{
    float4 f4Position : POSITION;
    float2 f2TexCoords : TEXCOORD0;
    float2 f2GlossCoords	 : TEXCOORD1;
    float4 f4Diff	 : COLOR0;
};

//------------------------------------------------------------------------------------------------
float4x3 ComputeWorldBoneTransform( float4 f4BlendIndices, float4 f4BlendWeights )
{
	// Compensate for lack of UBYTE4 on Geforce3
    int4 indices = D3DCOLORtoUBYTE4(f4BlendIndices);

    // Calculate normalized fourth bone weight
    float weight4 = 1.0f - f4BlendWeights[0] - f4BlendWeights[1] - f4BlendWeights[2];
    float4 weights = float4(f4BlendWeights[0], f4BlendWeights[1], f4BlendWeights[2], weight4);

    // Calculate bone transform
    float4x3 BoneTransform;
	BoneTransform = weights[0] * mtxWorldBones[indices[0]];
	BoneTransform += weights[1] * mtxWorldBones[indices[1]];
	BoneTransform += weights[2] * mtxWorldBones[indices[2]];
	BoneTransform += weights[3] * mtxWorldBones[indices[3]];
	return BoneTransform;
}

VS_OUTPUT FlagVS(VS_INPUT vIn)
{
	VS_OUTPUT Out = (VS_OUTPUT)0;

	float4x3 mtxBoneTransform = ComputeWorldBoneTransform( vIn.f4BlendIndices, vIn.f4BlendWeights );
	float4 f4WorldPosition = float4(mul(vIn.f4Position, mtxBoneTransform), 1);

	Out.f4Position = mul(f4WorldPosition, mtxViewProj);
	Out.f2TexCoord0 = vIn.f2TexCoords;
	Out.f2TexCoord1 = vIn.f2TexCoords;
	Out.f2TexCoord2 = vIn.f2TexCoords;
	Out.f2TexCoord3 = vIn.f2TexCoords;

	return Out;
}

VS_OUTPUT_NOLIGHTING FlagNoLightingVS(VS_INPUT vIn)
{
	VS_OUTPUT_NOLIGHTING Out = (VS_OUTPUT_NOLIGHTING)0;

	float4x3 mtxBoneTransform = ComputeWorldBoneTransform( vIn.f4BlendIndices, vIn.f4BlendWeights );
	float4 f4WorldPosition = float4(mul(vIn.f4Position, mtxBoneTransform), 1);

	Out.f4Position = mul(f4WorldPosition, mtxViewProj);
	Out.f2TexCoords = vIn.f2TexCoords;

	return Out;
}

float4 FlagPS(VS_OUTPUT vIn) : COLOR
{
	//icon color
	float4 f4IconColor = tex2D(DecalSampler, vIn.f2TexCoord0);
	float4 f4FrontColor = tex2D(BaseColorSampler, vIn.f2TexCoord1);
	f4IconColor.rgb *= f4FrontColor.rgb;
	
	//background color
	float4 f4BackColor = tex2D(DecalColorSampler, vIn.f2TexCoord2);

	//combine back and front based on icon alpha
	float4 f4FinalColor = lerp(f4IconColor, f4BackColor, f4IconColor.a);

	//base texture
	float4 f4BaseTexture = tex2D(BaseSampler, vIn.f2TexCoord3);
	f4FinalColor *= f4BaseTexture;
	f4FinalColor.a = f4BaseTexture.a;

	//finish
	return f4FinalColor;
}

float4 FlagNoLightingPS(VS_OUTPUT_NOLIGHTING vIn) : COLOR
{
	//icon color
	float4 f4BaseColor = tex2D(BaseSampler, vIn.f2TexCoords);
	return f4BaseColor;
}

float3 ComputeCiv4UnitLighting( float3 f3Normal )
{
	return saturate(dot(f3Normal.xyz, -f3UnitLightDir )) * f3UnitLightDiffuse + f3UnitAmbientColor;
}

VS_OUTPUTGLOSS FlagGlossVS(VS_INPUT vIn)
{
	VS_OUTPUTGLOSS Out = (VS_OUTPUTGLOSS)0;

	Out.f4Position = mul(vIn.f4Position, mtxWorldViewProj);
	Out.f2TexCoords = vIn.f2TexCoords;

	float3 wsNormal = mul(vIn.f3Normal, mtxWorld );
    wsNormal  = normalize(wsNormal );

   	Out.f4Diff.rgb = ComputeCiv4UnitLighting( wsNormal );	// L.N
   	Out.f4Diff.a = 1.0f;

   	// Environment map coordiantes, normal in view space
   	float3 temp = mul(float4(vIn.f3Normal,0.0), mtxWorldView);
	Out.f2GlossCoords.x = 0.5 * temp.x + 0.5;
	Out.f2GlossCoords.y = -0.5 * temp.y + 0.5;

	return Out;
}

float4 FlagGlossPS(VS_OUTPUTGLOSS vIn) : COLOR
{
	//gloss
	float4 f4EnvironmentMap = tex2D( EnvironmentMapSampler, vIn.f2GlossCoords );
	float4 f4GlossMask = tex2D( GlossSampler, vIn.f2TexCoords );
	f4EnvironmentMap *= f4GlossMask;

	//icon color
	float4 f4IconColor = tex2D(DecalSampler, vIn.f2TexCoords);
	float4 f4FrontColor = tex2D(BaseColorSampler, vIn.f2TexCoords);
	f4IconColor.rgb *= f4FrontColor.rgb;

	//background color
	float4 f4BackColor = tex2D(DecalColorSampler, vIn.f2TexCoords);

	//combine back and front based on icon alpha
	float4 f4FinalColor = lerp(f4IconColor, f4BackColor, f4IconColor.a);

	//base texture
	float4 f4BaseTexture = tex2D(BaseSampler, vIn.f2TexCoords);
	f4FinalColor *= f4BaseTexture;
	f4FinalColor.a = f4BaseTexture.a;

	//add lighting
	f4FinalColor *= vIn.f4Diff + f4EnvironmentMap;

	//finish
	return f4FinalColor;
}

technique TFlagGloss < string shadername = "TFlagGloss"; bool UsesNiRenderState = true; int implementation=0; >
{
  	pass P0
	{
		VertexShader = compile vs_1_1 FlagGlossVS();
		PixelShader = compile ps_1_4 FlagGlossPS();
	}
}

technique TFlagGloss_Simple < string shadername = "TFlagGloss"; bool UsesNiRenderState = true; int implementation=1; >
{
  	pass P0
	{
		VertexShader = compile vs_1_1 FlagVS();
		PixelShader = compile ps_1_1 FlagPS();
	}
}

technique TCiv4FlagSkinShader < string shadername = "TCiv4FlagSkinShader"; bool UsesNiRenderState = true; int BonesPerPartition = MAX_BONES; int implementation=0; >
{
  	pass P0
	{
		VertexShader = compile vs_1_1 FlagVS();
		PixelShader = compile ps_1_1 FlagPS();
	}
}

technique TCiv4SkinShaderNoLighting < string shadername = "TCiv4SkinShaderNoLighting"; bool UsesNiRenderState = true; int BonesPerPartition = MAX_BONES; int implementation=0; >
{
  	pass P0
	{
		VertexShader = compile vs_1_1 FlagNoLightingVS();
		PixelShader = compile ps_1_1 FlagNoLightingPS();
	}
}
