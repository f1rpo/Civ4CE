//------------------------------------------------------------------------------------------------
//  $Header: $
//------------------------------------------------------------------------------------------------
//  FILE:   Yield Symbol Shader
//
//  PURPOSE: Shader to display terrain yield output.
//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
//                          VARIABLES
//------------------------------------------------------------------------------------------------

// Transformations
float4x4 mtxInvView : INVVIEWTRANSPOSE;
float4x4 mtxViewProj: VIEWPROJECTIONTRANSPOSE;

//------------------------------------------------------------------------------------------------
//                          VERTEX INPUT & OUTPUT FORMATS
//------------------------------------------------------------------------------------------------
struct VS_INPUT
{
   float4 f4Position : POSITION;
   float4 f4Color : COLOR0;
   float2 f2TexCoord : TEXCOORD0;
   float2 f2CenterOffsetCoord : TEXCOORD1;
   float2 f2QuadOffsetCoord : TEXCOORD2;
};

struct VS_OUTPUT
{
	float4 f4Position : POSITION;
	float4 f4Color : COLOR0;
	float2 f2TexCoord : TEXCOORD0;
};

//------------------------------------------------------------------------------------------------
//                          VERTEX SHADER
//------------------------------------------------------------------------------------------------
VS_OUTPUT YieldSymbolVS( VS_INPUT vIn	)
{
    VS_OUTPUT vOut = (VS_OUTPUT) 0;
    
    //expand quad in camera orientation
    float3 f3CameraRight = mul(mtxInvView, float3(1, 0, 0));
    float3 f3CameraUp = cross(float3(0, 0, 1), f3CameraRight);
    float4 f4QuadCenter = float4(vIn.f2CenterOffsetCoord.x * f3CameraRight + vIn.f2CenterOffsetCoord.y * f3CameraUp, 0);
    float4 f4QuadOffset = mul(mtxInvView, float4(vIn.f2QuadOffsetCoord.xy, 0, 0));
    float4 f4WorldPosition = vIn.f4Position + f4QuadCenter + f4QuadOffset;

	//Transform point
   	vOut.f4Position  = mul(mtxViewProj, f4WorldPosition);
   	vOut.f4Color = vIn.f4Color;
   	vOut.f2TexCoord = vIn.f2TexCoord;

	return vOut;
}

//------------------------------------------------------------------------------------------------
texture BaseTexture <string NTM = "base";>;
sampler BaseSampler = sampler_state { Texture = (BaseTexture); AddressU  = WRAP; AddressV  = WRAP; MagFilter = LINEAR; MinFilter = LINEAR; MipFilter = LINEAR; };

//------------------------------------------------------------------------------------------------
float4 YieldSymbolPS( VS_OUTPUT vIn ) : COLOR
{
	// Get Base textures
	float4 f4FinalColor = tex2D(BaseSampler, vIn.f2TexCoord);
	f4FinalColor *= vIn.f4Color;
	return f4FinalColor;
}

//------------------------------------------------------------------------------------------------
//                          TECHNIQUES
//------------------------------------------------------------------------------------------------
technique TYieldSymbol_Shader< string shadername= "TYieldSymbol_Shader"; int implementation=0; bool UsesNiRenderState = true;>
{
    pass P0
    {
        VertexShader = compile vs_1_1 YieldSymbolVS();
        PixelShader  = compile ps_1_1 YieldSymbolPS();
    }
}

