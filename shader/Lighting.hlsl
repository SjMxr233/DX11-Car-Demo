struct Material
{
	float4 _Diffuse;
	float4 _Specular;
	float2 _offset;
	float2 _tiling;
};
struct DirectionalLight
{
	float4 LightColor;
	float3 Direction;
	float pad;
};