Texture2D gWorldPosTex:register(t0);
Texture2D gShadowMap:register(t1);
SamplerState gSampLinearClamp:register(s0);
SamplerComparisonState gsamShadow : register(s1);
cbuffer cbperObject_PS:register(b1)
{
	float4x4 gShadowTransform;
};
struct a2v
{
	float3 vertex :POSITION;
	float2 uv:TEXCOORD;
};
struct v2f
{
	float4 pos    : SV_POSITION;
	float2 uv:TEXCOORD;
};
float CalcShadowFactor(float4 shadowPos)
{
	float depth = shadowPos.z;
	uint width, height, numMips;
	gShadowMap.GetDimensions(0, width, height, numMips);
	float dx = 1.0f / (float)width;
	float percentLit = 0.0f;
	//5x5 tap
	for (int i = -2; i <= 2; i++)
	{
		for (int j = -2; j <= 2; j++)
		{
			percentLit += gShadowMap.SampleCmpLevelZero(gsamShadow,
				shadowPos.xy + float2(i,j)*dx , depth).r;
		}
	}
	//return gShadowMap.SampleCmpLevelZero(gsamShadow,shadowPos.xy, depth).r;
	return percentLit / 25.0f;
}
v2f vert(a2v v)
{
	v2f o;
	o.pos = float4(v.vertex, 1.0f);
	o.uv = v.uv;
	return o;
}
float frag(v2f i) :SV_Target
{
	float4 worldPos = gWorldPosTex.Sample(gSampLinearClamp,i.uv);
	float4 shadowPos = mul(worldPos, gShadowTransform);
	float atten = CalcShadowFactor(shadowPos);
	return atten;
}

