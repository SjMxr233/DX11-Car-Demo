#include "Lighting.hlsl"
cbuffer cbperObject:register(b0)
{
	float4x4 gModelViewProj;
	float4x4 gModel;
	float4x4 gModelInvTranspose;
	float4x4 gShadowTransform;
};
cbuffer cbperLight:register(b1)
{
	DirectionalLight gDirLight;
	Material gMaterial;
	float3 gEyePos;
	float pad;
};
struct a2v
{
	float3 vertex :POSITION;
	float3 normal:NORMAL;
	float2 uv:TEXCOORD;
};
struct v2f
{
	float4 pos:SV_POSITION;
	float3 worldnormal:NORMAL;
	float4 shadowpos:POSITION0;
	float3 worldpos:POSITION1;
	float2 uv:TEXCOORD;
};
Texture2D gTex:register(t0);
SamplerState gSampLinear:register(s0);
Texture2D gShadowMap:register(t1);
SamplerComparisonState gsamShadow : register(s1);
float CalcShadowFactor(float4 shadowPos)
{
	float depth = shadowPos.z;
	uint width, height, numMips;
	gShadowMap.GetDimensions(0, width, height, numMips);
	float dx = 1.0f / (float)width;
	float percentLit = 0.0f;
	//3x3 tap均值采样
	const float2 offsets[9] =
	{
		float2(-dx,  -dx), float2(0.0f,  -dx), float2(dx,  -dx),
		float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
		float2(-dx,  +dx), float2(0.0f,  +dx), float2(dx,  +dx)
	};
	[unroll]
	for (int i = 0; i < 9; ++i)
	{
		percentLit += gShadowMap.SampleCmpLevelZero(gsamShadow,
			shadowPos.xy + offsets[i], depth).r;
	}
	return percentLit / 9.0f;
}

v2f vert(a2v v)
{
	v2f o;
	float4 vertex = float4(v.vertex, 1.0f);
	o.pos = mul(vertex, gModelViewProj);
	o.worldpos = mul(vertex, gModel).xyz;
	o.shadowpos = mul(float4(o.worldpos, 1.0f), gShadowTransform);
	o.worldnormal = mul(float4(v.normal,1.0f), gModelInvTranspose).xyz;
	o.uv = v.uv;
	return o;
}
float4 frag(v2f i):SV_Target
{
	float3 LightDir = normalize(gDirLight.Direction);
	float3 normal = normalize(i.worldnormal);
	float3 ambient = float3(0.1f,0.1f,0.1f);
	float3 halfLambert = dot(normal, LightDir)*0.5f + 0.5f;
	float4 tex= gTex.Sample(gSampLinear, i.uv*gMaterial._tiling+ gMaterial._offset);
	float3 diffuse = gDirLight.LightColor.xyz*gMaterial._Diffuse.xyz*halfLambert*tex.xyz;
	float3 viewDir = normalize(gEyePos - i.worldpos);
	float3 halfDir = normalize(viewDir + LightDir);
	float atten = CalcShadowFactor(i.shadowpos);
	float3 specular = gDirLight.LightColor.xyz*gMaterial._Specular.xyz*pow(max(0, dot(normal, halfDir)),10.0f);
	float3 combinedColor = diffuse + specular;
	float3 _ShadowColor = float3(0.8f, 0.8f, 1.0f);
	float3 shadowColor = _ShadowColor *combinedColor;
	combinedColor = lerp(shadowColor, combinedColor, atten);
	return float4(combinedColor,1.0f);
}