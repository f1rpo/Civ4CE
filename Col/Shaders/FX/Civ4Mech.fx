
//------------------------------------------------------------------------------------------------
//
//  ***************** CIV4 GAME ENGINE   ********************
//
//! \file		Civ4Mech.fx
//! \author		tomw -- 09.20.05
//! \brief		Mech shader with Decal damage states, team color and gloss map
//
//------------------------------------------------------------------------------------------------
//  Copyright (c) 2005 Firaxis Games, Inc. All rights reserved.
//------------------------------------------------------------------------------------------------

float4x4 mtxViewProj : VIEWPROJ;
float4x4 mtxInvView : INVVIEW;
float4x4 mtxDecalTexture : TEXTRANSFORMDECAL;

static const int MAX_BONES = 20;
float4x3 mtxWorldBones[MAX_BONES] : SKINBONEMATRIX3; //world space bone matrix

float3 f3TeamColor: GLOBAL = {0.0f, 1.0f, 0.0f};
float4 fUnitFade: MATERIALDIFFUSE = (1.0.xxxx);
int			iShaderIndex : GLOBAL;

//Mech Light
float3      f3MechLightDir: GLOBAL    = {-0.6428f, 0.24603, -0.7254f};
float3      f3MechLightDiffuse: GLOBAL= {1.0f, 1.0f, 1.0f};
float3      f3MechAmbientColor: GLOBAL= { 0.2471f, 0.2353f,  0.3020f};

struct VS_INPUT
{
    float4 f4Pos			: POSITION;
    float3 f3Normal		: NORMAL;
    float2 f2TexCoord0   : TEXCOORD0;
    float4 f4BlendWeights : BLENDWEIGHT;
    float4 f4BlendIndices : BLENDINDICES;
};

struct VS_OUTPUT
{
    float4 f4Pos	 : POSITION;
    float3 f3Diffuse : COLOR0;
    float2 f2TexBase   : TEXCOORD0;
    float2 f2TexDecal  : TEXCOORD1;
};

struct VS_OUTPUTGLOSS
{
    float4 f4Pos		 : POSITION;
    float3 f3Diffuse : COLOR0;
    float2 f2TexBase   : TEXCOORD0;
    float2 f2TexDecal  : TEXCOORD1;
    float2 f2TexGloss  : TEXCOORD2;
    float3 f3Normal	 : TEXCOORD3;
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
VS_OUTPUT SkinningDecalVS(VS_INPUT vIn)
{
	VS_OUTPUT Out = (VS_OUTPUT)0;

	float4x3 mtxBoneTransform = ComputeWorldBoneTransform( vIn.f4BlendIndices, vIn.f4BlendWeights );
	float3 worldPosition = mul(vIn.f4Pos, mtxBoneTransform);

	Out.f4Pos = mul(float4(worldPosition, 1.0), mtxViewProj);
	Out.f2TexBase = vIn.f2TexCoord0;
	Out.f2TexDecal= mul(float4(vIn.f2TexCoord0,1,1), mtxDecalTexture);

	float3 wsNormal = mul(vIn.f3Normal, (float3x3)mtxBoneTransform );
	float NDotL = dot(normalize(wsNormal), -f3MechLightDir);
   	Out.f3Diffuse = max(NDotL, 0) * f3MechLightDiffuse;

	return Out;
}
//------------------------------------------------------------------------------------------------
VS_OUTPUTGLOSS SkinningDecalGlossVS(VS_INPUT vIn)
{
	VS_OUTPUTGLOSS Out = (VS_OUTPUTGLOSS)0;

	float4x3 mtxBoneTransform = ComputeWorldBoneTransform( vIn.f4BlendIndices, vIn.f4BlendWeights );
	float3 worldPosition = mul(vIn.f4Pos, mtxBoneTransform);

	Out.f4Pos = mul(float4(worldPosition, 1.0), mtxViewProj);
	Out.f2TexBase = vIn.f2TexCoord0;
	Out.f2TexDecal= mul(float4(vIn.f2TexCoord0,1,1), mtxDecalTexture);
	Out.f2TexGloss = vIn.f2TexCoord0;

	float3 wsNormal = mul(vIn.f3Normal, (float3x3)mtxBoneTransform );
	float NDotL = dot(normalize(wsNormal), -f3MechLightDir);
   	Out.f3Diffuse = max(NDotL, 0) * f3MechLightDiffuse;

   	// Environment map coordiantes
   	float3 cameraPosition = mul(float4(0, 0, 0, 1), mtxInvView);
   	float3 cameraVector = worldPosition - cameraPosition;
   	cameraVector = normalize(cameraVector);
   	cameraVector = reflect(cameraVector, wsNormal);
   	Out.f3Normal.x = 0.5 * cameraVector.x + 0.5;
	Out.f3Normal.y = 0.5 * -cameraVector.z + 0.5; //v texcoord flipped

	return Out;
}
//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------
//Base- 	Tank (Base Material UV 1 non animated)
//Decal	-	Tank Damage (Base Material UV 1 animated)
//Gloss	-	Tank Gloss
texture BaseMap <string NTM = "detail";>;
texture DecalMap <string NTM = "decal"; int NTMIndex = 0;>;
texture GlossMap <string NTM = "gloss"; int NTMIndex = 0;>;
texture EnvironMap <string NTM= "glow"; int NTMIndex = 0;>;

//samplers
sampler BaseSampler = sampler_state { Texture=(BaseMap); ADDRESSU=wrap; ADDRESSV=wrap; MAGFILTER=linear; MINFILTER=linear; MIPFILTER=linear; srgbtexture = false; };
sampler DecalSampler = sampler_state { Texture=(DecalMap); ADDRESSU=wrap; ADDRESSV=wrap; MAGFILTER=linear; MINFILTER=linear; MIPFILTER=linear; srgbtexture = false; };
sampler GlossSampler = sampler_state { Texture=(GlossMap); ADDRESSU=wrap; ADDRESSV=wrap; MAGFILTER=linear; MINFILTER=linear; MIPFILTER=linear; srgbtexture = false; };
sampler EnvironmentMapSampler = sampler_state { Texture=(EnvironMap); ADDRESSU=wrap; ADDRESSV=wrap; MAGFILTER=linear; MINFILTER=linear; MIPFILTER=linear; srgbtexture = false; };

//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------
float4 MechTeamColorDecalPS(VS_OUTPUT vIn) : COLOR
{
	float4	f4FinalColor = tex2D(BaseSampler, vIn.f2TexBase);
	float4  f4Decal = tex2D(DecalSampler,vIn.f2TexDecal);

	f4FinalColor.rgb = lerp(f3TeamColor,f4FinalColor.rgb, f4FinalColor.a);
	f4FinalColor.rgb = lerp(f4FinalColor.rgb,f4Decal.rgb, f4Decal.a);

	f4FinalColor.rgb *= vIn.f3Diffuse + f3MechAmbientColor;
	f4FinalColor.a = fUnitFade.a;

   return f4FinalColor;
}

float4 MechTeamColorDecalGlossPS(VS_OUTPUTGLOSS vIn) : COLOR
{
	float4	f4FinalColor = tex2D(BaseSampler, vIn.f2TexBase);
	float4  f4Decal = tex2D(DecalSampler, vIn.f2TexDecal);
	float3 f3EnvironmentMap = tex2D( EnvironmentMapSampler, vIn.f3Normal.xy );
	float3 f3GlossMask = tex2D( GlossSampler, vIn.f2TexGloss );

	f4FinalColor.rgb = lerp(f3TeamColor, f4FinalColor.rgb, f4FinalColor.a);
	f4FinalColor.rgb = lerp(f4FinalColor.rgb, f4Decal.rgb, f4Decal.a);

	float3 f3EnvMap = f3EnvironmentMap * f3GlossMask;
	f4FinalColor.rgb *= vIn.f3Diffuse + f3MechAmbientColor;
	f4FinalColor.rgb += f3EnvMap;
	f4FinalColor.a = fUnitFade.a;
	return f4FinalColor;
}

float4 MechPS_Shadow( VS_OUTPUT vIn ) : COLOR
{
	return float4(fUnitFade.a, fUnitFade.a, 0, fUnitFade.a);
}

float4 MechGlossPS_Shadow( VS_OUTPUTGLOSS vIn ) : COLOR
{
	return float4(fUnitFade.a, fUnitFade.a, 0, fUnitFade.a);
}

//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------

Pixelshader PSArrayCiv4MechShader11[4] =
{
	compile ps_1_1 MechTeamColorDecalPS(),
	compile ps_1_1 MechTeamColorDecalPS(),
	compile ps_1_1 MechTeamColorDecalPS(),
	compile ps_1_1 MechPS_Shadow()
};

technique TCiv4MechShader < string Description = "Civ4 Damage Decal FX-based skinning shader w/TeamColor(20 bones)"; int BonesPerPartition = MAX_BONES; bool UsesNiRenderState = true; >
{
  	pass P0
	{
		VertexShader = compile vs_1_1 SkinningDecalVS();
		PixelShader = (PSArrayCiv4MechShader11[iShaderIndex]);
	}
}

//-------------------------------------------------------------------------------------------
Pixelshader PSArrayCiv4MechShaderGloss11[4] =
{
	compile ps_1_1 MechTeamColorDecalGlossPS(),
	compile ps_1_1 MechTeamColorDecalGlossPS(),
	compile ps_1_1 MechTeamColorDecalGlossPS(),
	compile ps_1_1 MechGlossPS_Shadow()
};

technique TCiv4MechShaderGloss < string shadername = "TCiv4MechShaderGloss"; string Description = "Civ4 Damage Gloss Decal FX-based skinning shader w/TeamColor(20 bones)"; int BonesPerPartition = MAX_BONES; bool UsesNiRenderState = true; int implementation=0; >
{
  	pass P0
	{
		AlphaTestEnable  = false;
   		AlphaRef         = 0;

		VertexShader = compile vs_1_1 SkinningDecalGlossVS();
		PixelShader = (PSArrayCiv4MechShaderGloss11[iShaderIndex]);
	}
}

//-------------------------------------------------------------------------------------------
technique TCiv4MechNonShader < string Description = "Civ4 Damage Decal FX-based skinning shader w/TeamColor(20 bones)"; int BonesPerPartition = MAX_BONES; bool UsesNiRenderState = true; >
{
  	pass P0
	{
		VertexShader = compile vs_1_1 SkinningDecalVS();
		PixelShader = (PSArrayCiv4MechShader11[iShaderIndex]);
	}
}
