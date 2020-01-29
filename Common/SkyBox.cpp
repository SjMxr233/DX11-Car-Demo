#include "SkyBox.h"
struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 Tex;
};
struct cbFrame
{
	XMFLOAT4X4 MVP = MathHelper::Identity4x4();
	XMFLOAT4X4 gModel;
	XMFLOAT3 gEyePos;
	float pad;
};
SkyBox::SkyBox(ID3D11Device* device, ID3D11DeviceContext *deviceContext,float Radius, ComPtr<ID3D11ShaderResourceView> mTextureView)
{
	mCubeMapView = mTextureView;
	BuildConstantBuffer(device, deviceContext);
	BuildShaderAndInputLayout(device);
	BuildGeometry(device, deviceContext, Radius);

}
void SkyBox::UpdateConstantBuffer(ID3D11DeviceContext *deviceContext, XMMATRIX view, XMMATRIX proj, XMFLOAT3 gEyePos)
{
	cbFrame cb;
	XMMATRIX mvp = view * proj;
	XMStoreFloat4x4(&cb.MVP, XMMatrixTranspose(mvp));
	cb.gModel = MathHelper::Identity4x4();
	cb.gEyePos = gEyePos;

	D3D11_MAPPED_SUBRESOURCE mapData;
	HR(deviceContext->Map(mConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapData));
	memcpy_s(mapData.pData, sizeof(cb), &cb, sizeof(cb));
	deviceContext->Unmap(mConstantBuffer.Get(), 0);
}
void SkyBox::BuildConstantBuffer(ID3D11Device* device, ID3D11DeviceContext *deviceContext)
{
	D3D11_BUFFER_DESC cbd;
	ZeroMemory(&cbd, sizeof(cbd));
	cbd.ByteWidth = sizeof(cbFrame);
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbd.MiscFlags = 0;
	HR(device->CreateBuffer(&cbd, nullptr, mConstantBuffer.GetAddressOf()));

}
void SkyBox::BuildShaderAndInputLayout(ID3D11Device* device)
{
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] = {
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,24,D3D11_INPUT_PER_VERTEX_DATA,0}
	};
	ComPtr<ID3DBlob> vsByteCode = nullptr;
	ComPtr<ID3DBlob> psByteCode = nullptr;
	vsByteCode = d3dUtil::CompileShader(L"shader\\SkyBox.hlsl", nullptr, "vert", "vs_5_0");
	psByteCode = d3dUtil::CompileShader(L"shader\\SkyBox.hlsl", nullptr, "frag", "ps_5_0");

	HR(device->CreateVertexShader(vsByteCode->GetBufferPointer(), vsByteCode->GetBufferSize(), nullptr, mSkyBoxVs.GetAddressOf()));
	HR(device->CreatePixelShader(psByteCode->GetBufferPointer(), psByteCode->GetBufferSize(), nullptr, mSkyBoxPs.GetAddressOf()));
	HR(device->CreateInputLayout(vertexDesc, ARRAYSIZE(vertexDesc), vsByteCode->GetBufferPointer(), vsByteCode->GetBufferSize(),mSkyBoxLayout.GetAddressOf()));
}
void SkyBox::BuildGeometry(ID3D11Device* device, ID3D11DeviceContext *deviceContext,float Radius)
{
	GeometryGenerator Geo;
	GeometryGenerator::MeshData Sphere;
	Geo.CreateSphere(Radius, 20, 20, Sphere);
	//Geo.CreateBox(20.0f, 20.0f, 20.0f, Sphere);
	std::vector<Vertex> vertices(Sphere.Vertices.size());
	for (size_t i = 0; i < Sphere.Vertices.size(); ++i)
	{
		vertices[i].Pos = Sphere.Vertices[i].Position;
		vertices[i].Normal = Sphere.Vertices[i].Normal;
		vertices[i].Tex = Sphere.Vertices[i].TexC;
	}
	std::vector<UINT> indices;
	indices.insert(indices.end(), Sphere.Indices.begin(), Sphere.Indices.end());
	indexCount = indices.size();
	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));
	vbd.ByteWidth = sizeof(Vertex)*vertices.size();
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd, sizeof(ibd));
	ibd.ByteWidth = sizeof(UINT)*indices.size();
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	D3D11_SUBRESOURCE_DATA IinitData;
	IinitData.pSysMem = &indices[0];


	HR(device->CreateBuffer(&vbd, &vinitData, mVertexBuffer.GetAddressOf()));
	HR(device->CreateBuffer(&ibd, &IinitData, mIndiceBuffer.GetAddressOf()));


}
void SkyBox::Draw(ID3D11DeviceContext *deviceContext)
{
	UINT stride[1] = { sizeof(Vertex) };
	UINT offset[1] = { 0 };
	deviceContext->IASetVertexBuffers(0, 1, mVertexBuffer.GetAddressOf(), stride, offset);
	deviceContext->IASetIndexBuffer(mIndiceBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->IASetInputLayout(mSkyBoxLayout.Get());
	deviceContext->VSSetConstantBuffers(0, 1, mConstantBuffer.GetAddressOf());
	deviceContext->PSSetShaderResources(0, 1, mCubeMapView.GetAddressOf());//cubemap视图绑定到管线
	deviceContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
	deviceContext->VSSetShader(mSkyBoxVs.Get(), nullptr, 0);
	deviceContext->PSSetShader(mSkyBoxPs.Get(), nullptr, 0);
	deviceContext->DrawIndexed(indexCount, 0, 0);
}