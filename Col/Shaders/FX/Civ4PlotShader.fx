//------------------------------------------------------------------------------------------------
//  $Header: $
//------------------------------------------------------------------------------------------------
//  FILE:   Plot Shader
//
//  PURPOSE: Shader to display plot entities with shadows.
//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
//                          VARIABLES
//------------------------------------------------------------------------------------------------

#include "Civ4ShadowMap.fx.hlsl"

// Transformations
float4x4 mtxViewProj : VIEWPROJTRANSPOSE;
float4x4	mtxInvView : INVVIEWTRANSPOSE;
float4x4    mtxWorld   : WORLD;
float4x4	mtxFOW     : GLOBAL;
int			iShaderIndex : GLOBAL;
float4 f4MaterialDiffuse : MATERIALDIFFUSE = (1.0.xxxx);

static const int MAX_BONES = 20;
float4x3 mtxWorldBones[MAX_BONES] : SKINBONEMATRIX3; //world space bone matrix

//------------------------------------------------------------------------------------------------
//                          VERTEX INPUT & OUTPUT FORMATS
//------------------------------------------------------------------------------------------------
struct VS_INPUT
{
	float4 f4Position : POSITION;
	float3 f3Normal : NORMAL;
	float4 f4Color : COLOR0;
	float2 f2TexCoord : TEXCOORD0;
};

struct VS_INPUT_SKIN
{
	float4 f4Position : POSITION;
	float3 f3Normal : NORMAL;
	float4 f4Color : COLOR0;
	float2 f2TexCoord : TEXCOORD0;
	float4 f4BlendWeights : BLENDWEIGHT;
    float4 f4BlendIndices : BLENDINDICES;
};

struct VS_OUTPUT
{
	float4 f4Position : POSITION;
	float4 f4Color : COLOR0;
	float3 f3Diffuse : COLOR1;
	float2 f2TexCoord : TEXCOORD0;
	float4 f4ShadowTex : TEXCOORD1;
	float2 f2FOWTex : TEXCOORD2;
	float3 f3ReflectEye : TEXCOORD3;
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
//                          VERTEX SHADER
//------------------------------------------------------------------------------------------------
VS_OUTPUT PlotVSHelper(float4 f4WorldPosition, float3 f3WorldNormal, float4 f4Color, float2 f2TexCoord)
{
	VS_OUTPUT vOut = (VS_OUTPUT) 0;
	
	//Transform point
    vOut.f4Position = mul(mtxViewProj, f4WorldPosition);

   	//lighting
   	float NDotL = dot(normalize(f3WorldNormal), -f3SunLightDir);
	vOut.f3Diffuse = max(NDotL, 0) * f3SunLightDiffuse;
	vOut.f4Color = f4Color * f4MaterialDiffuse;

   	vOut.f4ShadowTex = getShadowTexCoord(f4WorldPosition);
   	vOut.f2FOWTex = mul(f4WorldPosition, mtxFOW);

   	vOut.f2TexCoord = f2TexCoord;
   	
   	//reflect eye
	float4 f4CameraPosition = mul(mtxInvView, float4(0, 0, 0, 1));
	float3 f3EyeVector = normalize(f4CameraPosition.xyz - f4WorldPosition.xyz);
	float3 f3ReflectEye = reflect(-f3EyeVector, f3WorldNormal);
	f3ReflectEye.y *= -1; //texture v is opposite world y
	f3ReflectEye = 0.5 * f3ReflectEye + 0.5;
	vOut.f3ReflectEye = f3ReflectEye;

	return vOut;
}

VS_OUTPUT PlotVS( VS_INPUT vIn	)
{
    float4 f4WorldPosition = mul(vIn.f4Position, mtxWorld);
    float3 f3WorldNormal = mul(vIn.f3Normal, mtxWorld);
	return PlotVSHelper(f4WorldPosition, f3WorldNormal, vIn.f4Color, vIn.f2TexCoord);
}

VS_OUTPUT PlotSkinVS( VS_INPUT_SKIN vIn	)
{
    float4x3 mtxBoneTransform = ComputeWorldBoneTransform( vIn.f4BlendIndices, vIn.f4BlendWeights );
	float4 f4WorldPosition = float4(mul(vIn.f4Position, mtxBoneTransform), 1);
	float3 f3WorldNormal = mul(vIn.f3Normal, (float3x3)mtxBoneTransform );
	return PlotVSHelper(f4WorldPosition, f3WorldNormal, vIn.f4Color, vIn.f2TexCoord);
}

//------------------------------------ TEXTURES ------------------------------------------------------------
texture BaseTexture <string NTM = "base";>;
texture GlossMap <string NTM = "gloss"; int NTMIndex = 0;>;
texture EnvironMap <string NTM= "glow"; int NTMIndex = 0;>;
texture ShadowMapTexture <string NTM = "shader";  int NTMIndex = 0;>;
texture TerrainFOWarTexture <string NTM = "shader";  int NTMIndex = 1;>;

//------------------------------------ SAMPLERS ------------------------------------------------------------
sampler BaseSampler = sampler_state { Texture = (BaseTexture); AddressU  = WRAP; AddressV  = WRAP; MagFilter = LINEAR; MinFilter = LINEAR; MipFilter = LINEAR; srgbtexture = false;};
sampler GlossSampler = sampler_state { Texture=(GlossMap); ADDRESSU=wrap; ADDRESSV=wrap; MAGFILTER=linear; MINFILTER=linear; MIPFILTER=linear; srgbtexture = false; };
sampler EnvironmentMapSampler = sampler_state { Texture=(EnvironMap); ADDRESSU=wrap; ADDRESSV=wrap; MAGFILTER=linear; MINFILTER=linear; MIPFILTER=linear; srgbtexture = false; };
sampler ShadowMap = sampler_state { Texture = (ShadowMapTexture);  AddressU = CLAMP; AddressV = CLAMP; MagFilter = LINEAR; MipFilter = NONE; MinFilter = LINEAR; srgbtexture = false;};
sampler TerrainFOWar = sampler_state  { Texture = (TerrainFOWarTexture);  AddressU = Wrap; AddressV = Wrap; MagFilter = Linear; MipFilter = Linear; MinFilter = Linear; srgbtexture = false;};

//------------------------------------------------------------------------------------------------
float4 PlotPS( VS_OUTPUT vIn, uniform bool bShadow, uniform bool bFOW, uniform bool bGloss ) : COLOR
{
	// Get Base * Vertex Color * Shadow * FOW
	float4 f4FinalColor = tex2D( BaseSampler, vIn.f2TexCoord );
	f4FinalColor *= vIn.f4Color;

	//shadow
	float fShadowFactor = 1;
	if(bShadow)
		fShadowFactor = getShadowFactor( ShadowMap, vIn.f4ShadowTex, float4(0, 1, 0, 0) );

	//lighting
	float3 f3Lighting = vIn.f3Diffuse * fShadowFactor + f3SunAmbientColor;
	f4FinalColor.rgb *= f3Lighting;
	
	if(bGloss)
	{
		float3 f3EnvironmentMap = tex2D( EnvironmentMapSampler, vIn.f3ReflectEye.xy );
		float3 f3GlossMask = tex2D( GlossSampler, vIn.f2TexCoord );
		f4FinalColor.rgb += f3GlossMask * f3EnvironmentMap;
	}

	//fow
	if(bFOW)
	{
		float3 f3FOWTex = tex2D( TerrainFOWar, vIn.f2FOWTex );
		f4FinalColor.rgb *= f3FOWTex;
	}

	return f4FinalColor;
}

float4 PlotPS_Shadow( VS_OUTPUT vIn ) : COLOR
{
	float4 f4Base = tex2D( BaseSampler, vIn.f2TexCoord );
	f4Base *= vIn.f4Color;
	return float4(f4Base.a, 0, f4Base.a, f4Base.a);
}

//------------------------------------------------------------------------------------------------
//                          PLOT TECHNIQUES
//------------------------------------------------------------------------------------------------
Pixelshader PSArrayPlotShader11[4] =
{
	compile ps_1_1 PlotPS(false, false, false),
	compile ps_1_1 PlotPS(true, true, false),
	compile ps_1_1 PlotPS(true, true, false),
	compile ps_1_1 PlotPS_Shadow()
};

technique TCiv4PlotShader< string shadername= "TCiv4PlotShader"; int implementation=0; bool UsesNiRenderState = true;>
{
    pass P0
    {
        VertexShader = compile vs_1_1 PlotVS();
        PixelShader	 = (PSArrayPlotShader11[iShaderIndex]);
    }
}

technique TCiv4PlotSkinShader< string shadername= "TCiv4PlotSkinShader"; int BonesPerPartition = MAX_BONES; int implementation=0; bool UsesNiRenderState = true;>
{
    pass P0
    {
        VertexShader = compile vs_1_1 PlotSkinVS();
        PixelShader	 = (PSArrayPlotShader11[iShaderIndex]);
    }
}

//------------------------------------------------------------------------------------------------
technique TCiv4PlotNoShadowShader< string shadername= "TCiv4PlotNoShadowShader"; int implementation=0; bool UsesNiRenderState = true;>
{
    pass P0
    {
        VertexShader = compile vs_1_1 PlotVS();
        PixelShader	 = compile ps_1_1 PlotPS(false, true, false);
    }
}

//------------------------------------------------------------------------------------------------
//                          BUILDING TECHNIQUES
//------------------------------------------------------------------------------------------------
technique TCiv4BuildingShader< string shadername= "TCiv4BuildingShader"; int implementation=0; bool UsesNiRenderState = true;>
{
    pass P0
    {
        VertexShader = compile vs_1_1 PlotVS();
        PixelShader	 = (PSArrayPlotShader11[iShaderIndex]);
    }
}

//------------------------------------------------------------------------------------------------
Pixelshader PSArrayBuildingGlossShader20[4] =
{
	compile ps_2_0 PlotPS(false, false, true),
	compile ps_2_0 PlotPS(true, true, true),
	compile ps_2_0 PlotPS(true, true, true),
	compile ps_2_0 PlotPS_Shadow()
};

technique TCiv4BuildingGlossShader< string shadername= "TCiv4BuildingGlossShader"; int implementation=0; bool UsesNiRenderState = true;>
{
    pass P0
    {
        VertexShader = compile vs_2_0 PlotVS();
        PixelShader	 = (PSArrayBuildingGlossShader20[iShaderIndex]);
    }
}

//------------------------------------------------------------------------------------------------
technique TCiv4BuildingGlossShader_Simple< string shadername= "TCiv4BuildingGlossShader"; int implementation=1; bool UsesNiRenderState = true;>
{
    pass P0
    {
        VertexShader = compile vs_1_1 PlotVS();
        PixelShader	 = (PSArrayPlotShader11[iShaderIndex]);
    }
}
