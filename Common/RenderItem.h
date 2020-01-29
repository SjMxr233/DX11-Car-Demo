#pragma once
#include "ConstantBuffer.h"
#include "d3dUtil.h"
struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 Tex;
};
struct SubmeshGeometry
{
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	INT BaseVertexLocation = 0;
};
struct MeshGeometry
{
	std::string name;
	Microsoft::WRL::ComPtr<ID3D11Buffer> mVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> mIndexBuffer;
	UINT Stride, Offet;
	std::unordered_map<std::string, SubmeshGeometry> DrawAgrs;
	void SetBuffer(ID3D11Device *device, const std::vector<GeometryGenerator::MeshData>& Meshdatas);

};
class RenderItem
{
public:
	RenderItem() = default;
	RenderItem(const RenderItem& rhs) = delete;

	XMFLOAT4X4 LocalToModelMatrix = MathHelper::Identity4x4();
	XMFLOAT4X4 World = MathHelper::Identity4x4();

	D3D11_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	Material *mat;
	MeshGeometry *mesh;
	Texture *tex;
	Shader *shader;


	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	INT BaseVertexLocation = 0;

	XMFLOAT3 mForward = XMFLOAT3(0, 0, 1);
	XMFLOAT3 mPosition = XMFLOAT3(0, 0, 0);

	Microsoft::WRL::ComPtr<ID3D11Buffer> mLightCbuffer=nullptr;
	Microsoft::WRL::ComPtr<ID3D11Buffer> mObjectCbuffer=nullptr;

	void XM_CALLCONV SetWorld(FXMMATRIX w);
	void XM_CALLCONV SetLocalToModel(FXMMATRIX m);
	void SetMeshAgrs(std::string SubMeshName);
	void SetRenderDefaultAgrs(ID3D11DeviceContext *context);
	void CreatLightCBuffer(ID3D11Device *device, UINT ByteWidth);
	void CreatObjectCBuffer(ID3D11Device *device, UINT ByteWidth);

};