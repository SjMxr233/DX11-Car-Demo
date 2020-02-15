#pragma once
#include "d3dUtil.h"

class ShadowMap
{
	template <typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;
public:
	ShadowMap(ID3D11Device* device,int Width, int Height,int SSSMwidth, int SSSMheight);
	~ShadowMap();
	void BuildResource(ID3D11Device* device,int Width, int Height,int SSSMwidth, int SSSMheight);
	void XM_CALLCONV UpdateShadowTransform(FXMMATRIX LightView, FXMMATRIX LightProj);
	ComPtr<ID3D11Texture2D> mShadowMap, mScreenSpaceShadowMap;
	ComPtr<ID3D11ShaderResourceView> mSRV, mScreenSpaceShadowMapSRV;
	ComPtr<ID3D11RenderTargetView> mScreenSpaceShadowMapRTV;
	ComPtr<ID3D11DepthStencilView> mDSV;
	D3D11_VIEWPORT mShadowMapViewPort,mScreenSpaceShadowMapViewPort;
	XMFLOAT4X4 mShadowTransform = MathHelper::Identity4x4();
};
