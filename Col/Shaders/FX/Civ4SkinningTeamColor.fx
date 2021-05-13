//------------------------------------------------------------------------------------------------
//
//  ***************** CIV4 GAME ENGINE   ********************
//
//! \file		Civ4SkinShader.fx
//! \author		tomw -- 06.15.05
//! \brief		Skin shader w/ team color
//
//------------------------------------------------------------------------------------------------
//  Copyright (c) 2005 Firaxis Games, Inc. All rights reserved.
//------------------------------------------------------------------------------------------------

float4x4 mtxViewProj : VIEWPROJ;
float4x4 mtxInvView : INVVIEW;
float4x4 mtxDecalTexture : TEXTRANSFORMDECAL;
float4x4 mtxFOW  : GLOBAL;
int iShaderIndex : GLOBAL;

static const int MAX_BONES = 20;
float4x3 mtxWorldBones[MAX_BONES] : SKINBONEMATRIX3; //world space bone matrix

float3 f3TeamColor: GLOBAL = {0.0f, 1.0f, 0.0f};
float4 fUnitFade: MATERIALDIFFUSE = (1.0.xxxx);

//Unit Light
float3      f3UnitLightDir: GLOBAL		= {-0.6428f, 0.24603, -0.7254f};
float3      f3UnitLightDiffuse: GLOBAL	= {1.0f, 1.0f, 1.0f};
float3      f3UnitAmbientColor: GLOBAL	= { 0.2471f, 0.2353f,  0.3020f};

struct VS_INPUT
{
    float4 Pos			: POSITION;
    float3 f3Normal		: NORMAL;
    float2 TexCoords    : TEXCOORD0;
    float4 BlendWeights : BLENDWEIGHT;
    float4 BlendIndices : BLENDINDICES;
};

struct VS_OUTPUT
{
    float4 Pos		 : POSITION;
    float3 f3Diffuse : COLOR0;
    float2 TexCoords : TEXCOORD0;
    float2 f2FowTex  : TEXCOORD2;
};

struct VS_OUTPUTGLOSS
{
    float4 Pos		 : POSITION;
    float3 f3Diffuse : COLOR0;
    float2 f2TexCoords1 : TEXCOORD0;
    float2 f2TexCoords2 : TEXCOORD1;
    float2 f2Environment : TEXCOORD2;
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

//------------------------------------------------------------------------------------------------
VS_OUTPUT SkinningVS(VS_INPUT vIn)
{
	VS_OUTPUT Out = (VS_OUTPUT)0;

	float4x3 mtxBoneTransform = ComputeWorldBoneTransform( vIn.BlendIndices, vIn.BlendWeights );
	float4 f4WorldPosition = float4(mul(vIn.Pos, mtxBoneTransform), 1);

	Out.Pos = mul(f4WorldPosition, mtxViewProj);
	Out.TexCoords = vIn.TexCoords;

   	Out.f2FowTex = mul(f4WorldPosition, mtxFOW);

	float3 wsNormal = mul(vIn.f3Normal, (float3x3)mtxBoneTransform );

	float NDotL = dot(normalize(wsNormal), -f3UnitLightDir);
   	Out.f3Diffuse = max(NDotL, 0) * f3UnitLightDiffuse;

	return Out;
}
//------------------------------------------------------------------------------------------------
VS_OUTPUTGLOSS SkinningGlossVS(VS_INPUT vIn)
{
	VS_OUTPUTGLOSS Out = (VS_OUTPUTGLOSS)0;

	float4x3 mtxBoneTransform = ComputeWorldBoneTransform( vIn.BlendIndices, vIn.BlendWeights );
	float3 worldPosition = mul(vIn.Pos, mtxBoneTransform);

	Out.Pos = mul(float4(worldPosition, 1.0), mtxViewProj);
	Out.f2TexCoords1 = vIn.TexCoords;
	Out.f2TexCoords2 = vIn.TexCoords;

	float3 wsNormal = mul(vIn.f3Normal, (float3x3)mtxBoneTransform );
    wsNormal = normalize(wsNormal);

   	float NDotL = dot(normalize(wsNormal), -f3UnitLightDir);
   	Out.f3Diffuse = max(NDotL, 0) * f3UnitLightDiffuse;

   	// Environment map coordiantes
   	float3 cameraPosition = mul(float4(0, 0, 0, 1), mtxInvView);
   	float3 cameraVector = worldPosition - cameraPosition;
   	cameraVector = normalize(cameraVector);
   	cameraVector = reflect(cameraVector, wsNormal);
   	Out.f2Environment.x = 0.5 * cameraVector.x + 0.5;
	Out.f2Environment.y = -0.5 * cameraVector.z + 0.5;

	return Out;
}

//------------------------------------------------------------------------------------------------
//                          PIXEL SHADER
//------------------------------------------------------------------------------------------------
texture BaseMap <string NTM = "base";>;
texture GlossMap <string NTM = "gloss"; int NTMIndex = 0;>;
texture EnvironMap <string NTM= "glow"; int NTMIndex = 0;>;
texture FOGTexture<string NTM = "shader";  int NTMIndex = 1;>;

sampler BaseSampler = sampler_state { Texture=(BaseMap); ADDRESSU=wrap; ADDRESSV=wrap; MAGFILTER=linear; MINFILTER=linear; MIPFILTER=linear; srgbtexture = false; };
sampler GlossSampler = sampler_state { Texture=(GlossMap); ADDRESSU=wrap; ADDRESSV=wrap; MAGFILTER=linear; MINFILTER=linear; MIPFILTER=linear; srgbtexture = false; };
sampler EnvironmentMapSampler = sampler_state { Texture=(EnvironMap); ADDRESSU=wrap; ADDRESSV=wrap; MAGFILTER=linear; MINFILTER=linear; MIPFILTER=linear; srgbtexture = false; };
sampler Fog = sampler_state  { Texture = (FOGTexture);	   AddressU = Clamp;  AddressV = Clamp;  MagFilter = Linear; MipFilter = Linear; MinFilter = Linear; srgbtexture = false; };

float4 SkinningPS(VS_OUTPUT vIn, uniform bool bTeamColor, uniform bool bFOW ) : COLOR
{
	//base
	float4	f4FinalColor = tex2D(BaseSampler, vIn.TexCoords);

	//team color alpha
	if(bTeamColor)
	{
		f4FinalColor.rgb = lerp(f3TeamColor, f4FinalColor.rgb, f4FinalColor.a);
		f4FinalColor.a = fUnitFade.a;
	}
	else
	{
		f4FinalColor.a *= fUnitFade.a;
	}

	//lighting
	f4FinalColor.rgb *= vIn.f3Diffuse + f3UnitAmbientColor;

	//FOW
	if(bFOW)
	{
		float3 f4FOWTex   = tex2D( Fog, vIn.f2FowTex ).rgb;
		f4FinalColor.rgb *= f4FOWTex;
	}

	return f4FinalColor;
}

float4 SkinningShadowPS( VS_OUTPUT vIn, uniform bool bTeamColor ) : COLOR
{
	float4 f4Base = tex2D( BaseSampler, vIn.TexCoords );

	if(bTeamColor)
		f4Base.a = fUnitFade.a;
	else
		f4Base.a *= fUnitFade.a;

	return float4(f4Base.a, f4Base.a, 0, f4Base.a);
}

float4 SkinningTeamColorGlossPS(VS_OUTPUTGLOSS vIn) : COLOR
{
	float4 f4FinalColor = tex2D(BaseSampler, vIn.f2TexCoords1);
	float3 f3EnvironmentMap = tex2D( EnvironmentMapSampler, vIn.f2Environment );
	float3 f3GlossMask = tex2D( GlossSampler, vIn.f2TexCoords2 );
	float3 f3EnvMap = f3EnvironmentMap * f3GlossMask;

	f4FinalColor.rgb = lerp(f3TeamColor,f4FinalColor.rgb, f4FinalColor.a);
	f4FinalColor.rgb *= vIn.f3Diffuse + f3UnitAmbientColor;
	f4FinalColor.rgb += f3EnvMap;
	f4FinalColor.a = fUnitFade.a;

	return f4FinalColor;
}

float4 SkinningGlossShadowPS(VS_OUTPUTGLOSS vIn) : COLOR
{
	float fAlpha = fUnitFade.a;
	return float4(fAlpha, fAlpha, 0, fAlpha);
}

//------------------------------------------------------------------------------------------------
//                          TECHNIQUES
//------------------------------------------------------------------------------------------------

Pixelshader PSCiv4Skinning11[4] =
{
	compile ps_1_1 SkinningPS(true, false),
	compile ps_1_1 SkinningPS(true, false),
	compile ps_1_1 SkinningPS(true, false),
	compile ps_1_1 SkinningShadowPS(true)
};

technique TCiv4Skinning < string Description = "Civ4 TeamColor FX-based skinning shader(20 bones)"; int BonesPerPartition = MAX_BONES; bool UsesNiRenderState = true; >
{
  	pass P0
	{
		VertexShader = compile vs_1_1 SkinningVS();
		PixelShader = (PSCiv4Skinning11[iShaderIndex]);
	}
}

//------------------------------------------------------------------------------------------------
Pixelshader PSCiv4SkinningNoTeamColor11[4] =
{
	compile ps_1_1 SkinningPS(false, false),
	compile ps_1_1 SkinningPS(false, false),
	compile ps_1_1 SkinningPS(false, false),
	compile ps_1_1 SkinningShadowPS(false)
};

technique TCiv4SkinningNoTeamColor < string Description = "Civ4 FX-based skinning shader(20 bones)"; int BonesPerPartition = MAX_BONES; bool UsesNiRenderState = true; >
{
  	pass P0
	{
		VertexShader = compile vs_1_1 SkinningVS();
		PixelShader = (PSCiv4SkinningNoTeamColor11[iShaderIndex]);
	}
}

//------------------------------------------------------------------------------------------------
Pixelshader PSCiv4SkinningNoTColorFOW11[4] =
{
	compile ps_1_1 SkinningPS(false, true),
	compile ps_1_1 SkinningPS(false, true),
	compile ps_1_1 SkinningPS(false, true),
	compile ps_1_1 SkinningShadowPS(false)
};

technique TCiv4SkinningNoTColorFOW < string Description = "Civ4 FOW FX-based skinning shader(20 bones)"; int BonesPerPartition = MAX_BONES; bool UsesNiRenderState = true; >
{
  	pass P0
	{
		VertexShader = compile vs_1_1 SkinningVS();
		PixelShader = (PSCiv4SkinningNoTColorFOW11[iShaderIndex]);
	}
}

//-------------------------------------------------------------------------------------------
// Environment Map Units---------------------------------------------------------------------
//-------------------------------------------------------------------------------------------
Pixelshader PSCiv4SkinningGloss11[4] =
{
	compile ps_1_1 SkinningTeamColorGlossPS(),
	compile ps_1_1 SkinningTeamColorGlossPS(),
	compile ps_1_1 SkinningTeamColorGlossPS(),
	compile ps_1_1 SkinningGlossShadowPS()
};

technique TCiv4SkinningGloss < string shadername = "TCiv4SkinningGloss"; string Description = "Civ4 TeamColor GlossMap FX-based skinning shader(20 bones)"; int BonesPerPartition = MAX_BONES; bool UsesNiRenderState = true; int implementation=0; >
{
  	pass P0
	{
		VertexShader = compile vs_1_1 SkinningGlossVS();
		PixelShader = (PSCiv4SkinningGloss11[iShaderIndex]);
	}
}
