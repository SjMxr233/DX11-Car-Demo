#pragma once
#include "d3dUtil.h"
#include <wrl.h>
#include <DirectXMath.h>
using namespace DirectX;
struct DirectLightCB
{
	DirectLightCB() { ZeroMemory(this, sizeof(this)); }

	DirectX::XMFLOAT4 LightColor;
	DirectX::XMFLOAT3 Direction;
	float pad;
};
struct Material
{
	Material() { ZeroMemory(this, sizeof(this)); }
	DirectX::XMFLOAT4 _Diffuse;
	DirectX::XMFLOAT4 _Specular;
};
struct Shader
{
	std::string name;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> mVertexLayout;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> mVertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> mPixelShader;
	void CreatShaderAndLayout(ID3D11Device *device, std::wstring shaderPath)
	{
		D3D11_INPUT_ELEMENT_DESC vertexDesc[] = {
			{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
			{"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0},
			{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,24,D3D11_INPUT_PER_VERTEX_DATA,0},
		};
		Microsoft::WRL::ComPtr<ID3DBlob> vsByteCode;
		Microsoft::WRL::ComPtr<ID3DBlob> psByteCode;
		vsByteCode = d3dUtil::CompileShader(shaderPath, nullptr, "vert", "vs_5_0");
		psByteCode = d3dUtil::CompileShader(shaderPath, nullptr, "frag", "ps_5_0");
		HR(device->CreateVertexShader(vsByteCode->GetBufferPointer(), vsByteCode->GetBufferSize(), nullptr, mVertexShader.GetAddressOf()));
		HR(device->CreatePixelShader(psByteCode->GetBufferPointer(), psByteCode->GetBufferSize(), nullptr, mPixelShader.GetAddressOf()));
		HR(device->CreateInputLayout(vertexDesc, ARRAYSIZE(vertexDesc), vsByteCode->GetBufferPointer(), vsByteCode->GetBufferSize(), mVertexLayout.GetAddressOf()));
	}
};
struct Texture
{
	std::string name;
	XMFLOAT4 uvST = XMFLOAT4(1, 1, 0, 0);
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mTextureView;
};
struct ObjectConstant
{
	XMFLOAT4X4 gModel;
	XMFLOAT4X4 gView;
	XMFLOAT4X4 gProj;
	XMFLOAT4X4 gModelInvTranspose;
	XMFLOAT4X4 gShadowTransform;
	XMFLOAT4 uvST;//xy通道用于tiling,zw通道offset
};
struct SkyBoxConstant
{
	XMFLOAT4X4 gModel;
	XMFLOAT4X4 gView;
	XMFLOAT4X4 gProj;
};
struct SSAOConstant
{
	XMFLOAT4X4 gView;
	XMFLOAT4X4 gProj;
	XMFLOAT4X4 gViewInvTranspose;
	XMFLOAT4 gOffset[64];
};
struct SSSMConstant
{
	XMFLOAT4X4 gShadowTransform;
};
struct ObjectLight
{
	DirectLightCB gDirLight;
	Material gMaterial;
	XMFLOAT3 gEyePos;
	float pad;
};