//------------------------------------------------------------------------------------------------
//  $Header: $
//------------------------------------------------------------------------------------------------
//  *****************   FIRAXIS GAME ENGINE   ********************
//
//  FILE:    Water
//
//  AUTHOR:  Tom Whittaker - 4/12/2004
//
//  PURPOSE: Draw water with fog/decal(borders)
//
//  Listing: fxc /Tvs_1_1 /EWaterVS /FcWater.lst Water.fx
//------------------------------------------------------------------------------------------------
//  Copyright (c) 2003 Firaxis Games, Inc. All rights reserved.
//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
//                          VARIABLES
//------------------------------------------------------------------------------------------------

// Transformations
float4x4 mtxWorldViewProj: WORLDVIEWPROJECTION;
float4x4 mtxInvView : INVVIEWTRANSPOSE;
float4x4 mtxWorld   : WORLD;
float3x3 mtxWaterTexture1 : GLOBAL;
float3x3 mtxWaterTexture2: GLOBAL;
float3x3 mtxWaterGrid: GLOBAL;
float4x4 mtxFOW     : GLOBAL;
float4x4 mtxReflection  : GLOBAL;
float4x4 mtxRefraction  : GLOBAL;
float3 f3SunLightDiffuse : GLOBAL = {1.00, 0.92, 0.74};
float4 f4WaterConstants : GLOBAL; //rgba => (camera_zoom, reflection_scale, refraction_scale, alpha)

//------------------------------------------------------------------------------------------------
//                          VERTEX INPUT & OUTPUT FORMATS
//------------------------------------------------------------------------------------------------
struct VS_INPUT
{
   float3 f3Pos     : POSITION;
   float2 f2BaseTex : TEXCOORD0;
   float2 f2CoastTex: TEXCOORD1;
   float2 f2FowTex  : TEXCOORD2;
};

struct VS_OUTPUT_SIMPLE
{
	float4 f4Pos     : POSITION;
	float2 f2BaseTex1 : TEXCOORD0;
	float2 f2BaseTex2 : TEXCOORD1;
	float2 f2CoastTex : TEXCOORD2;
	float2 f2FowTex  : TEXCOORD3;
	float2 f2GridTex : TEXCOORD4;
};

struct VS_OUTPUT
{
	float4 f4Pos     : POSITION;
	float4 f4BaseTex : TEXCOORD0;
	float2 f2CoastTex: TEXCOORD1;
	float2 f2FowTex  : TEXCOORD2;
	float3 f3EyeVector: TEXCOORD3;
	float4 f4ReflectionTex: TEXCOORD4;
	float4 f4RefractionTex: TEXCOORD5;
	float2 f2GridTex : TEXCOORD6;
};

//------------------------------------------------------------------------------------------------
//                          VERTEX SHADER
//------------------------------------------------------------------------------------------------
VS_OUTPUT WaterVS( VS_INPUT vIn )
{
	VS_OUTPUT vOut = (VS_OUTPUT)0;

	//Transform point
	vOut.f4Pos  = mul(float4(vIn.f3Pos, 1), mtxWorldViewProj);
	float4 f4WorldPosition = mul(float4(vIn.f3Pos,1), mtxWorld);

	//eye vector
	float4 f4CameraPosition = mul(mtxInvView, float4(0, 0, 0, 1));
	vOut.f3EyeVector = normalize(f4CameraPosition.xyz - f4WorldPosition.xyz);

	// Set texture coordinates
	float3 f3WorldTexturePosition = float3(f4WorldPosition.xy, 1);
	vOut.f4BaseTex.xy = mul(mtxWaterTexture1, f3WorldTexturePosition);
	vOut.f4BaseTex.zw = mul(mtxWaterTexture2, f3WorldTexturePosition);
	vOut.f2GridTex = mul(mtxWaterGrid, f3WorldTexturePosition);
	vOut.f2CoastTex = vIn.f2CoastTex;
	vOut.f2FowTex   = vIn.f2FowTex;
	vOut.f4ReflectionTex = mul(f4WorldPosition, mtxReflection);
	vOut.f4RefractionTex = mul(f4WorldPosition, mtxRefraction);

	return vOut;
}

VS_OUTPUT_SIMPLE WaterSimpleVS( VS_INPUT vIn )
{
	VS_OUTPUT_SIMPLE vOut = (VS_OUTPUT_SIMPLE)0;

	//Transform point
	vOut.f4Pos  = mul(float4(vIn.f3Pos, 1), mtxWorldViewProj);
	float4 f4WorldPosition = mul(float4(vIn.f3Pos,1), mtxWorld);

	// Set texture coordinates
	float3 f3WorldTexturePosition = float3(f4WorldPosition.xy, 1);
	vOut.f2BaseTex1 = mul(mtxWaterTexture1, f3WorldTexturePosition);
	vOut.f2BaseTex2 = mul(mtxWaterTexture2, f3WorldTexturePosition);
	vOut.f2GridTex = mul(mtxWaterGrid, f3WorldTexturePosition);
	vOut.f2CoastTex = vIn.f2CoastTex;
	vOut.f2FowTex   = vIn.f2FowTex;

	return vOut;
}

//------------------------------------------------------------------------------------------------
//                          PIXEL SHADER
//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------
// TEXTURES
//------------------------------------------------------------------------------------------------
texture BaseTexture <string NTM = "shader";  int NTMIndex = 0;>;
texture NormalMapTexture <string NTM = "shader";  int NTMIndex = 0;>;
texture CoastFadeTexture <string NTM = "shader";  int NTMIndex = 1;>;
texture FOGTexture		 <string NTM = "shader";  int NTMIndex = 2;>;
texture GridTexture <string NTM = "shader";  int NTMIndex = 3;>;
textureCUBE EnvironmentMapTexture <string NTM = "shader";  int NTMIndex = 4;>;
texture ReflectionMapTexture <string NTM = "shader";  int NTMIndex = 5;>;
texture RefractionMapTexture <string NTM = "shader";  int NTMIndex = 6;>;

//------------------------------------------------------------------------------------------------
//                          SAMPLERS
//------------------------------------------------------------------------------------------------
sampler BaseMap = sampler_state { Texture = (BaseTexture);  AddressU = Wrap; AddressV = Wrap; MagFilter = LINEAR; MipFilter = LINEAR; MinFilter = LINEAR; srgbtexture = false;};
sampler NormalMap = sampler_state { Texture = (NormalMapTexture);  AddressU = Wrap; AddressV = Wrap; MagFilter = LINEAR; MipFilter = LINEAR; MinFilter = LINEAR; srgbtexture = false;};
sampler CoastFade = sampler_state  { Texture = (CoastFadeTexture); AddressU = Wrap; AddressV = Wrap; MagFilter = Linear; MipFilter = None; MinFilter = Linear; srgbtexture = false;};
sampler Fog    	  = sampler_state  { Texture = (FOGTexture);	   AddressU = Wrap;  AddressV = Wrap;  MagFilter = Linear; MipFilter = Linear; MinFilter = Linear; srgbtexture = false;};
sampler FogSRGB   = sampler_state  { Texture = (FOGTexture);	   AddressU = Wrap;  AddressV = Wrap;  MagFilter = Linear; MipFilter = Linear; MinFilter = Linear; srgbtexture = true;};
sampler GridMap = sampler_state { Texture = (GridTexture);  AddressU = WRAP; AddressV = WRAP; MagFilter = LINEAR; MipFilter = LINEAR; MinFilter = LINEAR; srgbtexture = false;};
samplerCUBE EnvironmentMap = sampler_state { Texture = (EnvironmentMapTexture);  AddressU = CLAMP; AddressV = CLAMP; AddressW = CLAMP; MagFilter = LINEAR; MipFilter = LINEAR; MinFilter = LINEAR; srgbtexture = true;};
sampler ReflectionMap = sampler_state { Texture = (ReflectionMapTexture);  AddressU = CLAMP; AddressV = CLAMP; MagFilter = LINEAR; MipFilter = NONE; MinFilter = LINEAR; srgbtexture = true;};
sampler RefractionMap = sampler_state { Texture = (RefractionMapTexture);  AddressU = CLAMP; AddressV = CLAMP; MagFilter = LINEAR; MipFilter = NONE; MinFilter = LINEAR; srgbtexture = true;};

//------------------------------------------------------------------------------------------------
float4 WaterPS( VS_OUTPUT vIn ) : COLOR
{
	float4	f4FinalColor = 0.0f;

	//water constants
	float fCameraZoom = f4WaterConstants.r;
	float fReflectionScale = f4WaterConstants.g;
	float fRefractionScale = f4WaterConstants.b;

	//normal
	float4 f4Normal1 = tex2D( NormalMap, vIn.f4BaseTex.xy);
	float4 f4Normal2 = tex2D( NormalMap, vIn.f4BaseTex.zw);
	f4Normal1.rgb = 2 * f4Normal1.rgb - 1;
	f4Normal2.rgb = 2 * f4Normal2.rgb - 1;
	float3 f3Normal = f4Normal1.rgb + f4Normal2.rgb;
	f3Normal.z /= 2;
	f3Normal = normalize(f3Normal);

	// Get FOW
	float4 f4FOWTex = tex2D( FogSRGB, vIn.f2FowTex );

	//coast alpha
	float coastAlpha = tex2D( CoastFade, vIn.f2CoastTex ).r;

	//environment
	float3 f3ReflectEye = reflect(-vIn.f3EyeVector, f3Normal);
	float4 f4Environment = texCUBE( EnvironmentMap, f3ReflectEye );
	f4Environment.a *= f4Environment.a; //convert specular alpha to linear space

	//reflection
	float2 f2ReflectionTex = vIn.f4ReflectionTex.xy / vIn.f4ReflectionTex.w + f3Normal.xy * fReflectionScale;
	float4 f4Reflection = tex2D( ReflectionMap, f2ReflectionTex );
	float4 f4CombinedReflection = lerp(f4Environment, f4Reflection, f4Reflection.a);
	f4CombinedReflection.a = f4Environment.a * (1 - f4Reflection.a); //specular
	f4CombinedReflection *= f4FOWTex.rgbr;

	//refraction
	float2 f2RefractionTex = vIn.f4RefractionTex.xy / vIn.f4RefractionTex.w + coastAlpha * f3Normal.xy * fRefractionScale;
	float4 f4Refraction = tex2D( RefractionMap, f2RefractionTex );

	//fresnel 0 - refraction, 1 - reflection
	float fDot = dot(vIn.f3EyeVector, f3Normal);
	float fFresnel = (1 - 2 * fDot + fDot * fDot) * 0.6 + 0.1; //adjust bounds
	fFresnel = lerp(fFresnel, 0.5, fCameraZoom);
	fFresnel = fFresnel * coastAlpha;
	f4FinalColor.rgb = lerp(f4Refraction.rgb, f4CombinedReflection.rgb, fFresnel);

	//specular
	f4FinalColor.rgb += f4CombinedReflection.a * coastAlpha * f3SunLightDiffuse * f3SunLightDiffuse; //convert sun color to linear space

	//grid lines
	float4 f4GridTexture = tex2D( GridMap, vIn.f2GridTex );
	f4FinalColor.rgb += f4GridTexture.rgb * f4FOWTex.rgb;

	f4FinalColor.a = 1 / 0.75 * f4Refraction.a; //fixes unwanted transparent parts of refraction
	f4FinalColor.rgb = sqrt(f4FinalColor.rgb); //convert back to gamma space
	return f4FinalColor;
}

float4 WaterSimplePS( VS_OUTPUT_SIMPLE vIn ) : COLOR
{
	float4 f4Base1 = tex2D( BaseMap, vIn.f2BaseTex1);
	float4 f4Base2 = tex2D( BaseMap, vIn.f2BaseTex2);
	float coastAlpha = tex2D( CoastFade, vIn.f2CoastTex ).r;
	float4 f4FOWTex = tex2D( Fog, vIn.f2FowTex );

	float4 f4FinalColor = 0.5 * f4Base1 + 0.5 * f4Base2;
	f4FinalColor.a = f4WaterConstants.a;
	f4FinalColor += f4Base1.a * f4Base2.a;
	f4FinalColor.a *= coastAlpha;
	f4FinalColor *= f4FOWTex;
	return f4FinalColor;
}

//------------------------------------------------------------------------------------------------
//                          TECHNIQUES
//------------------------------------------------------------------------------------------------
technique Water_Shader< string shadername = "Water_Shader"; int implementation=0;>
{
    pass P0
    {
        // Enable depth writing
        ZEnable        = TRUE;
        ZWriteEnable   = TRUE;
        ZFunc          = LESSEQUAL;

        // Enable alpha blending & testing
        AlphaBlendEnable = true;
        AlphaTestEnable	 = true;
		AlphaRef         = 0;
		AlphaFunc        = GREATER;
        SrcBlend         = SRCALPHA;
        DestBlend        = INVSRCALPHA;

   		// Set up textures and texture stage states
        VertexShader = compile vs_1_1 WaterVS();
        PixelShader  = compile ps_2_0 WaterPS();
    }
}

technique WaterSimple_Shader< string shadername = "WaterSimple_Shader"; int implementation=0;>
{
    pass P0
    {
        // Enable depth writing
        ZEnable        = TRUE;
        ZWriteEnable   = TRUE;
        ZFunc          = LESSEQUAL;

        // Enable alpha blending & testing
        AlphaBlendEnable = true;
        AlphaTestEnable	 = true;
		AlphaRef         = 0;
		AlphaFunc        = GREATER;
        SrcBlend         = SRCALPHA;
        DestBlend        = INVSRCALPHA;

   		// Set up textures and texture stage states
        VertexShader = compile vs_1_1 WaterSimpleVS();
        PixelShader  = compile ps_1_1 WaterSimplePS();
    }
}
