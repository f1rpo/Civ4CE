//------------------------------------------------------------------------------------------------
//  $Header: $
//------------------------------------------------------------------------------------------------
//  FILE:   Ground Shadow Shader
//
//  PURPOSE: Shader to display ground with shadows.
//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
//                          VARIABLES
//------------------------------------------------------------------------------------------------

#include "Civ4ShadowMap.fx.hlsl"

// Transformations
float4x4 mtxWorldViewProj: WORLDVIEWPROJECTIONTRANSPOSE;
float4x4    mtxWorld   : WORLD;
float4x4	mtxFOW     : GLOBAL;

//------------------------------------------------------------------------------------------------
//                          VERTEX INPUT & OUTPUT FORMATS
//------------------------------------------------------------------------------------------------
struct VS_INPUT
{
	float4 f4Position : POSITION;
	float4 f4Color : COLOR;
	float3 f3Normal : NORMAL;
	float2 f2TexCoord : TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 f4Position : POSITION;
	float4 f4Color : COLOR0;
	float3 f3Diffuse : COLOR1;
	float2 f2TexCoord : TEXCOORD0;
	float4 f4ShadowTex : TEXCOORD1;
	float2 f2FOWTex : TEXCOORD2;
};

//------------------------------------------------------------------------------------------------
//                          VERTEX SHADER
//------------------------------------------------------------------------------------------------
VS_OUTPUT GroundShadowVS( VS_INPUT vIn	)
{
    VS_OUTPUT vOut = (VS_OUTPUT) 0;

    //Transform point
   	vOut.f4Position  = mul(mtxWorldViewProj, vIn.f4Position);

   	float4 f4WorldPosition = mul(vIn.f4Position, mtxWorld);
   	float3 f3WorldNormal = mul(vIn.f3Normal, mtxWorld);
   	vOut.f4ShadowTex = getShadowTexCoord(f4WorldPosition);
   	vOut.f2FOWTex = mul(f4WorldPosition, mtxFOW);

   	vOut.f4Color = vIn.f4Color;
   	vOut.f2TexCoord = vIn.f2TexCoord;

   	float NDotL = dot(normalize(f3WorldNormal), -f3SunLightDir);
	vOut.f3Diffuse = max(NDotL, 0) * f3SunLightDiffuse;

	return vOut;
}

//------------------------------------ TEXTURES ------------------------------------------------------------
texture BaseTexture <string NTM = "base";>;
texture ShadowMapTexture <string NTM = "shader";  int NTMIndex = 0;>;
texture TerrainFOWarTexture <string NTM = "shader";  int NTMIndex = 1;>;

//------------------------------------ SAMPLERS ------------------------------------------------------------
sampler BaseSampler = sampler_state { Texture = (BaseTexture); AddressU  = WRAP; AddressV  = WRAP; MagFilter = LINEAR; MinFilter = LINEAR; MipFilter = LINEAR; srgbtexture = false;};
sampler ShadowMap = sampler_state { Texture = (ShadowMapTexture);  AddressU = CLAMP; AddressV = CLAMP; MagFilter = LINEAR; MipFilter = NONE; MinFilter = LINEAR; srgbtexture = false;};
sampler TerrainFOWar = sampler_state  { Texture = (TerrainFOWarTexture);  AddressU = Wrap; AddressV = Wrap; MagFilter = Linear; MipFilter = Linear; MinFilter = Linear; srgbtexture = false;};

//------------------------------------------------------------------------------------------------
float4 GroundShadowPS( VS_OUTPUT vIn ) : COLOR
{
	// Get Base * Vertex Color * FOW * lighting
	float4 f4FinalColor = tex2D( BaseSampler, vIn.f2TexCoord );
	float3 f3FOWTex = tex2D( TerrainFOWar, vIn.f2FOWTex );

	//lighting
	float fShadowFactor = getShadowFactor( ShadowMap, vIn.f4ShadowTex, float4(1, 0, 0, 0) );
	float3 f3Lighting = vIn.f3Diffuse * fShadowFactor + f3SunAmbientColor;

	f4FinalColor *= vIn.f4Color;
	f4FinalColor.rgb *= f3Lighting;
	f4FinalColor.rgb *= f3FOWTex;

	return f4FinalColor;
}

//------------------------------------------------------------------------------------------------
//                          TECHNIQUES
//------------------------------------------------------------------------------------------------
technique GroundShadowShader< string shadername= "GroundShadowShader"; int implementation=0; bool UsesNiRenderState = true;>
{
    pass P0
    {
        VertexShader = compile vs_1_1 GroundShadowVS();
        PixelShader  = compile ps_1_1 GroundShadowPS();
    }
}
