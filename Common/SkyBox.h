#ifndef SKYBOX_H
#define SKYBOX_H
#include "d3dUtil.h"
using namespace DirectX;
template <typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;
class SkyBox
{
public:
	SkyBox(){}
	SkyBox(ID3D11Device* device,ID3D11DeviceContext *deviceContext,float Radius, ComPtr<ID3D11ShaderResourceView> mTextureView);
	void BuildGeometry(ID3D11Device* device, ID3D11DeviceContext *deviceContext,float Radius);
	void BuildShaderAndInputLayout(ID3D11Device* device);
	void BuildConstantBuffer(ID3D11Device* device, ID3D11DeviceContext *deviceContext);
	void UpdateConstantBuffer(ID3D11DeviceContext *deviceContext, XMMATRIX view, XMMATRIX proj,XMFLOAT3 gEyePos);
	void Draw(ID3D11DeviceContext *deviceContext);
private:
	UINT indexCount = 0;

	ComPtr<ID3D11Buffer> mVertexBuffer;
	ComPtr<ID3D11Buffer> mIndiceBuffer;
	ComPtr<ID3D11Buffer> mConstantBuffer;
	ComPtr<ID3D11ShaderResourceView> mCubeMapView;
	ComPtr<ID3D11VertexShader> mSkyBoxVs;
	ComPtr<ID3D11PixelShader> mSkyBoxPs;
	ComPtr<ID3D11InputLayout> mSkyBoxLayout;
};
#endif SKYBOX_H