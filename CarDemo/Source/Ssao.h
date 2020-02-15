#pragma once
#include "d3dUtil.h"

class Ssao
{
	template <typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;
public:
	Ssao(ID3D11Device* device, int Width, int Height);
	~Ssao();
	void BuildResource(ID3D11Device* device, int Width, int Height);
	void BuildRandVecMap(ID3D11Device* device);
	void BuildOffsetVectors();
	ComPtr<ID3D11Texture2D> mAoMap, mRandMap,mAoBlurMap;
	ComPtr<ID3D11ShaderResourceView> mAoSRV, mRandSRV, mAoBlurSRV;
	ComPtr<ID3D11RenderTargetView> mAoRTV, mAoBlurRTV;

	D3D11_VIEWPORT mViewPort;
	XMFLOAT4 mOffsets[64];
};