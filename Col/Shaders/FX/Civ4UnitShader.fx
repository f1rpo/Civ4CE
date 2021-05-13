//------------------------------------------------------------------------------------------------
//  $Header: $
//------------------------------------------------------------------------------------------------
//  FILE:   Unit Shader
//
//  PURPOSE: Shader to display units with shadows.
//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
//                          VARIABLES
//------------------------------------------------------------------------------------------------

// Transformations
float4x4 mtxWorldViewProj: WORLDVIEWPROJECTIONTRANSPOSE;
float4x4    mtxWorld   : WORLD;
int			iShaderIndex : GLOBAL;
float3 f3SunLightDir : GLOBAL;
float3 f3SunLightDiffuse : GLOBAL;
float3 f3SunAmbientColor : GLOBAL;

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

struct VS_OUTPUT
{
	float4 f4Position : POSITION;
	float4 f4Color : COLOR0;
	float3 f3Diffuse : COLOR1;
	float2 f2TexCoord : TEXCOORD0;
};

//------------------------------------------------------------------------------------------------
//                          VERTEX SHADER
//------------------------------------------------------------------------------------------------
VS_OUTPUT UnitVS( VS_INPUT vIn	)
{
    VS_OUTPUT vOut = (VS_OUTPUT) 0;

    //Transform point
   	vOut.f4Position  = mul(mtxWorldViewProj, vIn.f4Position);

   	//lighting
   	float3 f3WorldNormal = normalize(mul(vIn.f3Normal, mtxWorld));
   	vOut.f4Color = vIn.f4Color;
   	vOut.f2TexCoord = vIn.f2TexCoord;

   	float NDotL = dot(f3WorldNormal, -f3SunLightDir);
   	vOut.f3Diffuse = max(NDotL, 0) * f3SunLightDiffuse;

	return vOut;
}

//------------------------------------ TEXTURES ------------------------------------------------------------
texture BaseTexture <string NTM = "base";>;

//------------------------------------ SAMPLERS ------------------------------------------------------------
sampler BaseSampler = sampler_state { Texture = (BaseTexture); AddressU  = WRAP; AddressV  = WRAP; MagFilter = LINEAR; MinFilter = LINEAR; MipFilter = LINEAR; srgbtexture = false;};

//------------------------------------------------------------------------------------------------
float4 UnitPS( VS_OUTPUT vIn ) : COLOR
{
	// Get Base * Vertex Color * Shadow
	float4 f4FinalColor = tex2D( BaseSampler, vIn.f2TexCoord );
	f4FinalColor *= vIn.f4Color;
	f4FinalColor.rgb *= vIn.f3Diffuse + f3SunAmbientColor;
	return f4FinalColor;
}

float4 UnitPS_Shadow( VS_OUTPUT vIn ) : COLOR
{
	float4 f4Base = tex2D( BaseSampler, vIn.f2TexCoord );
	return float4(f4Base.a, f4Base.a, 0, 1);
}

//------------------------------------------------------------------------------------------------
//                          TECHNIQUES
//------------------------------------------------------------------------------------------------
Pixelshader PSArray11[4] =
{
	compile ps_1_1 UnitPS(),
	compile ps_1_1 UnitPS(),
	compile ps_1_1 UnitPS(),
	compile ps_1_1 UnitPS_Shadow()
};

technique TCiv4UnitShader< string shadername= "TCiv4UnitShader"; int implementation=0; bool UsesNiRenderState = true;>
{
    pass P0
    {
		AlphaTestEnable	 = TRUE;
		AlphaRef		 = 0;
        AlphaFunc		 = GREATER;

        VertexShader = compile vs_1_1 UnitVS();
        PixelShader	 = (PSArray11[iShaderIndex]);
    }
}
