Texture2D gAoTex:register(t0);
Texture2D gNormalDepthTex:register(t1);
SamplerState gSampLinearClamp:register(s0);
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
static const float kGeometryCoeff = 0.8f;
float2 texelSize(Texture2D tex)
{
	uint Width, Height;
	tex.GetDimensions(Width, Height);
	return 1.0f/float2(Width, Height);
}
float CompareNormal(float3 CenterNormal, float3 NeighborNormal)
{
	return smoothstep(kGeometryCoeff, 1.0f, dot(CenterNormal, NeighborNormal));
}
float frag(v2f i) :SV_Target
{
	float2 delta = texelSize(gAoTex)*1.0f;

	float2 p0 = i.uv;
	float2 p1 = i.uv - delta;
	float2 p2 = i.uv + delta;
	float2 p3 = i.uv - delta * 2;
	float2 p4 = i.uv + delta * 2;
	float2 p5 = i.uv - delta * 3.2307692308;
	float2 p6 = i.uv + delta * 3.2307692308;

	float3 n0 = gNormalDepthTex.Sample(gSampLinearClamp, p0).xyz;
	float3 n1 = gNormalDepthTex.Sample(gSampLinearClamp, p1).xyz;
	float3 n2 = gNormalDepthTex.Sample(gSampLinearClamp, p2).xyz;
	float3 n3 = gNormalDepthTex.Sample(gSampLinearClamp, p3).xyz;
	float3 n4 = gNormalDepthTex.Sample(gSampLinearClamp, p4).xyz;
	float3 n5 = gNormalDepthTex.Sample(gSampLinearClamp, p5).xyz;
	float3 n6 = gNormalDepthTex.Sample(gSampLinearClamp, p6).xyz;

	float w0 = 0.37004405286;
	float w1 = CompareNormal(n0, n1)*0.31718061674;
	float w2 = CompareNormal(n0, n2)*0.31718061674;
	float w3 = CompareNormal(n0, n3)*0.19823788546;
	float w4 = CompareNormal(n0, n4)*0.19823788546;
	float w5 = CompareNormal(n0, n5)*0.11453744493;
	float w6 = CompareNormal(n0, n6)*0.11453744493;

	float s;
	s = gAoTex.Sample(gSampLinearClamp, p0).r*w0;
	s += gAoTex.Sample(gSampLinearClamp, p1).r*w1;
	s += gAoTex.Sample(gSampLinearClamp, p2).r*w2;
	s += gAoTex.Sample(gSampLinearClamp, p3).r*w3;
	s += gAoTex.Sample(gSampLinearClamp, p4).r*w4;
	s += gAoTex.Sample(gSampLinearClamp, p5).r*w5;
	s += gAoTex.Sample(gSampLinearClamp, p6).r*w6;

	return s / (w0 + w1 + w2 + w3 + w4+w5+w6);
}

