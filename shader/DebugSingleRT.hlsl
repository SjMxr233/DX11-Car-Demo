Texture2D gTex:register(t0);
SamplerState gSampLinearWarp:register(s0);
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
	o.pos = float4(v.vertex,1.0f);
	o.uv = v.uv;
	return o;
}
float4 frag(v2f i):SV_Target
{
	return float4(gTex.Sample(gSampLinearWarp, i.uv).xxx,1.0f);
}

