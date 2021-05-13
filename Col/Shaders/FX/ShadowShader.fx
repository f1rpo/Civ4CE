//Shadow Blur Shader
//Jason Winokur

float4x4 mtxWorldViewProj: WORLDVIEWPROJECTIONTRANSPOSE;
float2 f2ShadowPixelSize : GLOBAL;

static const float fRotatedGrid1 = 0.7;
static const float fRotatedGrid2 = 0.3;
static const float2 f2RotatedGrid[4] = {{fRotatedGrid1, fRotatedGrid2}, {-fRotatedGrid2, fRotatedGrid1}, {-fRotatedGrid1, -fRotatedGrid2}, {fRotatedGrid2, -fRotatedGrid1}};

//------------------------------------------------------------------------------------------------
struct VS_INPUT
{
   float4 f4Position : POSITION;
   float2 f2TexCoord : TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 f4Position : POSITION;
	float2 f2TexCoord[4] : TEXCOORD0;
};

//------------------------------------------------------------------------------------------------
VS_OUTPUT ShadowBlurVS( VS_INPUT vIn	)
{
    VS_OUTPUT vOut = (VS_OUTPUT) 0;

    //Transform point
   	vOut.f4Position  = mul(mtxWorldViewProj, vIn.f4Position);
   	for(int i=0;i<4;i++)
   		vOut.f2TexCoord[i] = vIn.f2TexCoord + f2RotatedGrid[i] * f2ShadowPixelSize;

	return vOut;
}

//------------------------------------------------------------------------------------------------
texture BaseTexture <string NTM = "base";>;
sampler BaseSampler = sampler_state { Texture = (BaseTexture); AddressU  = WRAP; AddressV  = WRAP; MagFilter = LINEAR; MinFilter = LINEAR; MipFilter = LINEAR; };

//------------------------------------------------------------------------------------------------
float4 ShadowBlurPS( VS_OUTPUT vIn ) : COLOR
{
	// Get Base textures
	float4 f4FinalColor = 0;
	for(int i=0;i<4;i++)
		f4FinalColor += tex2D(BaseSampler, vIn.f2TexCoord[i]);
	
	f4FinalColor = saturate(1 - 0.25 * f4FinalColor);
	f4FinalColor *= 0.5 * f4FinalColor + 0.5;
	
	return f4FinalColor;
}

technique TShadowBlurShader
{
	pass P0
    {
		//Ignore ZBuffer
        ZEnable        = FALSE;
        ZWriteEnable   = FALSE;

        //Ignore Alpha Blending
        AlphaBlendEnable = FALSE;
        AlphaTestEnable	 = FALSE;
    
        VertexShader = compile vs_1_1 ShadowBlurVS();
        PixelShader  = compile ps_1_1 ShadowBlurPS();
    }
}