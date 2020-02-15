#include "Lighting.hlsl"
Texture2D gWorldPosTex:register(t0);
Texture2D gNormalDepthTex:register(t1);
Texture2D gAoMap:register(t2);
Texture2D gScreenShadowMap:register(t3);
Texture2D gDiffuseTex:register(t4);
SamplerState gSampLinearClamp:register(s0);
cbuffer cbperObject_PS:register(b1)
{
	DirectionalLight gDirLight;
	Material gMaterial;
	float3 gEyePos;
	float pad;
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
v2f vert(a2v v)
{
	v2f o;
	o.pos = float4(v.vertex, 1.0f);
	o.uv = v.uv;
	return o;
}
float4 frag(v2f i) :SV_Target
{
	float3 diffuse=gDiffuseTex.Sample(gSampLinearClamp,i.uv).xyz;
	float3 worldPos = gWorldPosTex.Sample(gSampLinearClamp,i.uv).xyz;
	float3 normal = gNormalDepthTex.Sample(gSampLinearClamp, i.uv).xyz;
	float ao = gAoMap.Sample(gSampLinearClamp, i.uv).r;
	float atten = gScreenShadowMap.Sample(gSampLinearClamp, i.uv).r;

	float3 LightDir = normalize(gDirLight.Direction);
	float3 viewDir = normalize(gEyePos - worldPos);
	float3 halfDir = normalize(viewDir + LightDir);
	float3 ambient = float3(0.4f, 0.4f, 0.4f);
	float3 halfLambert = dot(normal, LightDir)*0.5f + 0.5f;
	diffuse *= gDirLight.LightColor.xyz*gMaterial._Diffuse.xyz*halfLambert;
	float3 specular = gDirLight.LightColor.xyz*gMaterial._Specular.xyz*pow(max(0, dot(normal, halfDir)), 10.0f);

	float3 combinedColor = diffuse + specular+ambient*ao;
	float3 _ShadowColor = float3(0.8f, 0.8f, 1.0f);
	float3 shadowColor = _ShadowColor * combinedColor;
	combinedColor = lerp(shadowColor, combinedColor, atten);
	return float4(combinedColor, 1.0f);

}

