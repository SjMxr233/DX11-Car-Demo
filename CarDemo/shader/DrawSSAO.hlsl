Texture2D gWorldPosTex:register(t0);
Texture2D gNormalDepthTex:register(t1);
Texture2D gRandomVecTex:register(t2);
SamplerState gSampLinearClamp:register(s0);
SamplerState gSampLinearWrap:register(s1);
#define sampleRadius 0.6f
#define depthBias  0.3f
cbuffer cbperObject_PS:register(b1)
{
	float4x4 gView;
	float4x4 gProj;
	float4x4 gViewInvTranspose;
	float4 gOffset[64];
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
float frag(v2f i) :SV_Target
{
	uint Width, Height;
	gWorldPosTex.GetDimensions(Width, Height);
	float3 worldPos = gWorldPosTex.Sample(gSampLinearClamp,i.uv).xyz;
	float3 viewPos = mul(float4(worldPos, 1.0f), gView).xyz;
	float3 worldNormal = gNormalDepthTex.Sample(gSampLinearClamp, i.uv).xyz;
	float3 viewNormal = mul(worldNormal, (float3x3)gViewInvTranspose).xyz;
	float3 randVec = 2.0f*gRandomVecTex.Sample(gSampLinearWrap, i.uv*float2(Width, Height) / 4.0).xyz-1.0f;
	float3 tangent = normalize(randVec - viewNormal * dot(randVec, viewNormal));
	float3 bitangent = normalize(cross(viewNormal, tangent));
	float3x3 TBN = float3x3(tangent, bitangent, viewNormal);
	float oc = 0.0f;
	for (int i = 0; i < 64; i++)
	{
		float3 offset = mul(gOffset[i].xyz, TBN);
		//float flip = sign(dot(offset, viewNormal));
		float3 randPos = viewPos + sampleRadius * offset;
		float4 ndcPos = mul(float4(randPos, 1.0f), gProj);
		ndcPos /= ndcPos.w;
		float2 screenPos = ndcPos.xy*0.5f+0.5f;
		screenPos.y = 1.0f - screenPos.y;
		float randDepth = gNormalDepthTex.Sample(gSampLinearClamp, screenPos).a;
		float range = smoothstep(0.0f, 1.0f, sampleRadius / abs(randDepth - viewPos.z));
		float ao = (randDepth + depthBias < viewPos.z) ? 1.0f : 0.0f;
		oc += ao * range;
	}
	oc = 1.0f - oc / 64;
	return saturate(pow(oc,2.2f));
}

