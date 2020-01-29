#pragma once
#include "d3dUtil.h"

class ShadowMap
{
	template <typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;
public:
	ShadowMap(ID3D11Device* device,int Width, int Height);
	~ShadowMap();
	void BuildResource(ID3D11Device* device,int Width, int Height);
	
	ComPtr<ID3D11Texture2D> mShadowMap;
	ComPtr<ID3D11ShaderResourceView> mSRV;
	ComPtr<ID3D11DepthStencilView> mDSV;
	D3D11_VIEWPORT mViewPort;
	int mWidth, mHeight;
};
