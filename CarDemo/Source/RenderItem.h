#pragma once
#include "ConstantBuffer.h"
#include "Transform.h"
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
	UINT Stride = sizeof(Vertex);
	UINT Offset = 0;
	std::unordered_map<std::string, SubmeshGeometry> DrawAgrs;
	void SetBuffer(ID3D11Device *device, const std::vector<GeometryGenerator::MeshData>& Meshdatas);

};
class RenderItem
{
public:
	RenderItem();
	~RenderItem();
	RenderItem(const RenderItem& rhs) = delete;
	D3D11_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	Transform *transform=new Transform();

	Material *mat=nullptr;
	MeshGeometry *mesh=nullptr;
	Texture *tex=nullptr;
	Shader *shader = nullptr;

	
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	INT BaseVertexLocation = 0;

	Microsoft::WRL::ComPtr<ID3D11Buffer> mVsCB=nullptr;
	Microsoft::WRL::ComPtr<ID3D11Buffer> mPsCB=nullptr;

	void SetMeshAgrs(std::string SubMeshName);
	void SetRenderDefaultAgrs(ID3D11DeviceContext *context);
	void CreatObjectPsCBuffer(ID3D11Device *device, UINT ByteWidth);
	void CreatObjectVsCBuffer(ID3D11Device *device, UINT ByteWidth);

};