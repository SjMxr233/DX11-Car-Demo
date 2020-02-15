#pragma once
#include "d3dUtil.h"

class LightBuffer
{
	template <typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;
public:
	LightBuffer(ID3D11Device* device, int Width, int Height);
	~LightBuffer();
	void BuildResource(ID3D11Device* device, int Width, int Height);
	ComPtr<ID3D11Texture2D> mLightBufferMap;
	ComPtr<ID3D11ShaderResourceView> mSRV;
	ComPtr<ID3D11RenderTargetView> mRTV;
	D3D11_VIEWPORT mViewPort;
};