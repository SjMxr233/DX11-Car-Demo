cbuffer cbperObject_VS:register(b0)
{
	float4x4 gModel;
	float4x4 gView;
	float4x4 gProj;
};
struct a2v
{
	float3 vertex :POSITION;
};
struct v2f
{
	float4 pos:SV_POSITION;
	float3 modelPos:POSITION;
};
TextureCube gCubeMap:register(t0);
SamplerState gSampLinearWarp:register(s0);
v2f vert(a2v v)
{
	v2f o;
	o.pos = mul(float4(v.vertex,1.0f) , gModel);
	o.pos = mul(o.pos, gView);
	o.pos = mul(o.pos, gProj).xyww;
	o.modelPos = v.vertex;
	return o;
}
float4 frag(v2f i) :SV_Target
{
	return gCubeMap.Sample(gSampLinearWarp,i.modelPos);//提前预设立方体顶点模型坐标，坐标插值到像素作为uv采样cubemap
}