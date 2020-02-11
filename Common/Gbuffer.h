#pragma once
#include "d3dUtil.h"
const int BUFFER_COUNT=3;
//世界空间法线&视角空间深度，世界空间位置，漫反射
class Gbuffer
{
	template <typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;
public:
	Gbuffer(ID3D11Device* device, int Width, int Height);
	~Gbuffer();
	void BuildResource(ID3D11Device* device, int Width, int Height);
	void ClearRenderTargets(ID3D11DeviceContext *context,float FarZ);

	ComPtr<ID3D11Texture2D> mTexs[BUFFER_COUNT];
	ComPtr<ID3D11ShaderResourceView> mSRVs[BUFFER_COUNT];
	ComPtr<ID3D11RenderTargetView> mRTVs[BUFFER_COUNT];

	ComPtr<ID3D11Texture2D> mDSB;
	ComPtr<ID3D11DepthStencilView> mDSV;
	D3D11_VIEWPORT mViewPort;
};
