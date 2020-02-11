#include "RenderItem.h"
void MeshGeometry::SetBuffer(ID3D11Device *device, const std::vector<GeometryGenerator::MeshData>& Meshdatas)
{
	UINT totalVertexCount = 0;
	UINT totalIndexCount = 0;
	for (auto &mesh : Meshdatas)
	{
		totalVertexCount += mesh.Vertices.size();
		totalIndexCount += mesh.Indices.size();
	}
	std::vector<Vertex> vertices(totalVertexCount);
	std::vector<UINT> indices;
	UINT k = 0;
	UINT vertexOffset = 0;;
	UINT indexOffset = 0;
	for (auto &mesh : Meshdatas)
	{
		SubmeshGeometry Submesh;
		Submesh.BaseVertexLocation = vertexOffset;
		Submesh.StartIndexLocation = indexOffset;
		Submesh.IndexCount = mesh.Indices.size();
		for (size_t i = 0; i < mesh.Vertices.size(); ++i, ++k)
		{
			vertices[k].Pos = mesh.Vertices[i].Position;
			vertices[k].Normal = mesh.Vertices[i].Normal;
			vertices[k].Tex = mesh.Vertices[i].TexC;
		}
		indices.insert(indices.end(), mesh.Indices.begin(), mesh.Indices.end());
		vertexOffset += mesh.Vertices.size();
		indexOffset += mesh.Indices.size();
		DrawAgrs[mesh.name] = Submesh;
	}
	D3D11_BUFFER_DESC vbd;//描述创建的顶点缓存区
	vbd.ByteWidth = sizeof(Vertex)*totalVertexCount;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;//D3D11_USAGE_IMMUTABLE创建资源后内容不变,Gpu以只读的方式读取.
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;//0时cpu无需读写缓存
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vinitData;//初始化顶点缓存区数据
	vinitData.pSysMem = &vertices[0];

	D3D11_BUFFER_DESC ibd;
	ibd.ByteWidth = sizeof(UINT)*totalIndexCount;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA IinitData;
	IinitData.pSysMem = &indices[0];

	HR(device->CreateBuffer(&ibd, &IinitData, mIndexBuffer.GetAddressOf()));
	HR(device->CreateBuffer(&vbd, &vinitData, mVertexBuffer.GetAddressOf()));
}
void RenderItem::CreatObjectPsCBuffer(ID3D11Device *device, UINT ByteWidth)
{
	D3D11_BUFFER_DESC cbd;
	ZeroMemory(&cbd, sizeof(cbd));
	cbd.ByteWidth = ByteWidth;
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbd.MiscFlags = 0;

	HR(device->CreateBuffer(&cbd, nullptr, mPsCB.GetAddressOf()));
}
void RenderItem::CreatObjectVsCBuffer(ID3D11Device *device, UINT ByteWidth)
{
	D3D11_BUFFER_DESC cbd;
	ZeroMemory(&cbd, sizeof(cbd));
	cbd.ByteWidth = ByteWidth;
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbd.MiscFlags = 0;

	HR(device->CreateBuffer(&cbd, nullptr, mVsCB.GetAddressOf()));
}
void XM_CALLCONV RenderItem::SetWorld(FXMMATRIX w)
{
	XMStoreFloat4x4(&World,w);
}
void XM_CALLCONV RenderItem::SetLocalToModel(FXMMATRIX m)
{
	XMStoreFloat4x4(&LocalToModelMatrix, m);
}
void RenderItem::SetMeshAgrs(std::string SubMeshName)
{
	if (mesh)
	{
		this->IndexCount = mesh->DrawAgrs[SubMeshName].IndexCount;
		this->BaseVertexLocation = mesh->DrawAgrs[SubMeshName].BaseVertexLocation;
		this->StartIndexLocation = mesh->DrawAgrs[SubMeshName].StartIndexLocation;
	}
}
void RenderItem::SetRenderDefaultAgrs(ID3D11DeviceContext *Context)
{
	if (mesh)
	{
		auto stride = mesh->Stride;
		auto offset = mesh->Offet;
		Context->IASetVertexBuffers(0, 1, mesh->mVertexBuffer.GetAddressOf(), &stride, &offset);
		Context->IASetIndexBuffer(mesh->mIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		Context->IASetPrimitiveTopology(PrimitiveType);
		Context->IASetInputLayout(shader->mVertexLayout.Get());

		Context->PSSetShaderResources(0, 1, tex->mTextureView.GetAddressOf());
		Context->PSSetSamplers(0, 1, RenderStates::LinearWrap.GetAddressOf());

		Context->PSSetShader(shader->mPixelShader.Get(), nullptr, 0);
		Context->PSSetConstantBuffers(1, 1, mPsCB.GetAddressOf());//b1

		Context->VSSetShader(shader->mVertexShader.Get(), nullptr, 0);
		Context->VSSetConstantBuffers(0, 1, mVsCB.GetAddressOf());//b0-0
	}
}