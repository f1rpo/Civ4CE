//Shadow Map Helper functions
//Jason Winokur

float4x4	mtxShadow  : GLOBAL;
float4		f4ShadowOffsets : GLOBAL;
float3 f3SunLightDir : GLOBAL = {-0.57, 0.43, -0.70};
float3 f3SunLightDiffuse : GLOBAL = {1.00, 0.92, 0.74};
float3 f3SunAmbientColor : GLOBAL = {0.35, 0.40, 0.52};

float4 getShadowTexCoord(float4 f4WorldPosition)
{
	return mul(f4WorldPosition, mtxShadow);
}

float getShadowFactor(uniform sampler ShadowSampler, float4 f4ShadowTexCoord, float4 f4Channels)
{
	float4 f4ShadowTex = tex2D( ShadowSampler, f4ShadowTexCoord.xy );
	return dot(f4ShadowTex, f4Channels);
}

technique ShadowDummyTechnique
{
}