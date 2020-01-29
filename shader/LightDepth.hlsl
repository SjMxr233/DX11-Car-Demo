cbuffer cbperObject:register(b0)
{
	float4x4 gModelViewProj;
	float4x4 gModel;
	float4x4 gModelInvTranspose;
	float4x4 gShadowTransform;
};
struct a2v
{
	float3 vertex :POSITION;
};
struct v2f
{
	float4 pos : SV_POSITION;
};
v2f vert(a2v v)
{
	v2f o;
	o.pos = mul(float4(v.vertex, 1.0f), gModelViewProj);
	return o;
}
void frag(v2f i)
{

}

