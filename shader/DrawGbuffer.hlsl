Texture2D gTex:register(t0);
SamplerState gSampLinearWarp:register(s0);
cbuffer cbperObject_VS:register(b0)
{
	float4x4 gModel;
	float4x4 gView;
	float4x4 gProj;
	float4x4 gModelInvTranspose;
	float4x4 gShadowTransform;
	float4 uvST;
};
struct a2v
{
	float3 vertex :POSITION;
	float3 normal:NORMAL;
	float2 uv:TEXCOORD;
};
struct v2f
{
	float4 pos : SV_POSITION;
	float3 worldnormal:NORMAL;
	float4 posW:POSITION0;
	float4 posV:POSITION1;
	float2 uv:TEXCOORD;
};
struct Gbuffer
{
	float4 diffuse:SV_Target0; //使用RGB通道,A通道不使用
	float4 normal:SV_Target1;//RGB通道保存世界空间法线,A通道保存视角空间深度
	float4 worldpos:SV_Target2;
};
v2f vert(a2v v)
{
	v2f o;
	o.posW = mul(float4(v.vertex, 1.0f), gModel);
	o.posV = mul(o.posW, gView);
	o.pos = mul(o.posV, gProj);
	o.worldnormal = mul(float4(v.normal, 1.0f), gModelInvTranspose).xyz;
	o.uv = v.uv*uvST.xy + uvST.zw;
	return o;
}
Gbuffer frag(v2f i):SV_Target
{
	Gbuffer o;
	o.diffuse= gTex.Sample(gSampLinearWarp, i.uv);
	o.normal = float4(normalize(i.worldnormal), i.posV.z);
	o.worldpos = float4(i.posW.xyz, 1.0f);
	return o;
}

