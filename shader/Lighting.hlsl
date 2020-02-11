struct Material
{
	float4 _Diffuse;
	float4 _Specular;
};
struct DirectionalLight
{
	float4 LightColor;
	float3 Direction;
	float pad;
};