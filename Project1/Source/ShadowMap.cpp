#include "ShadowMap.h"

ShadowMap::ShadowMap(ID3D11Device* device, int Width, int Height,int SSSMwidth, int SSSMheight) 
{
	BuildResource(device,Width, Height, SSSMwidth,SSSMheight);
}
ShadowMap::~ShadowMap()
{

}
void XM_CALLCONV ShadowMap::UpdateShadowTransform(FXMMATRIX LightView,FXMMATRIX LightProj)
{
	XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	XMMATRIX S = LightView * LightProj*T;
	XMStoreFloat4x4(&mShadowTransform, S);
}
void ShadowMap::BuildResource(ID3D11Device* device, int Width, int Height, int SSSMwidth, int SSSMheight)
{
	D3D11_TEXTURE2D_DESC depthDesc;
	ZeroMemory(&depthDesc, sizeof(depthDesc));
	depthDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	depthDesc.Width = Width;
	depthDesc.Height = Height;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.SampleDesc.Quality = 0;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	depthDesc.CPUAccessFlags = 0;
	depthDesc.MiscFlags = 0;
	HR(device->CreateTexture2D(&depthDesc, 0,mShadowMap.ReleaseAndGetAddressOf()));

	D3D11_TEXTURE2D_DESC SSSMDesc;
	ZeroMemory(&SSSMDesc, sizeof(SSSMDesc));
	SSSMDesc.Format = DXGI_FORMAT_R32_FLOAT;
	SSSMDesc.Width = SSSMwidth;
	SSSMDesc.Height = SSSMheight;
	SSSMDesc.MipLevels = 1;
	SSSMDesc.ArraySize = 1;
	SSSMDesc.SampleDesc.Count = 1;
	SSSMDesc.SampleDesc.Quality = 0;
	SSSMDesc.Usage = D3D11_USAGE_DEFAULT;
	SSSMDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	SSSMDesc.CPUAccessFlags = 0;
	SSSMDesc.MiscFlags = 0;
	HR(device->CreateTexture2D(&SSSMDesc, 0, mScreenSpaceShadowMap.ReleaseAndGetAddressOf()));

	D3D11_RENDER_TARGET_VIEW_DESC RTVDesc;
	ZeroMemory(&RTVDesc, sizeof(RTVDesc));
	RTVDesc.Format = SSSMDesc.Format;
	RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	RTVDesc.Texture2D.MipSlice = 0;
	HR(device->CreateRenderTargetView(mScreenSpaceShadowMap.Get(), &RTVDesc, mScreenSpaceShadowMapRTV.ReleaseAndGetAddressOf()));

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	ZeroMemory(&SRVDesc, sizeof(SRVDesc));
	SRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MostDetailedMip = 0;
	SRVDesc.Texture2D.MipLevels = 1;
	HR(device->CreateShaderResourceView(mShadowMap.Get(),&SRVDesc, mSRV.GetAddressOf()));
	SRVDesc.Format = SSSMDesc.Format;
	HR(device->CreateShaderResourceView(mScreenSpaceShadowMap.Get(), &SRVDesc, mScreenSpaceShadowMapSRV.ReleaseAndGetAddressOf()));

	D3D11_DEPTH_STENCIL_VIEW_DESC DSVDesc;
	ZeroMemory(&DSVDesc, sizeof(DSVDesc));
	DSVDesc.Format = DXGI_FORMAT_D32_FLOAT;
	DSVDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	DSVDesc.Texture2D.MipSlice = 0;
	HR(device->CreateDepthStencilView(mShadowMap.Get(),&DSVDesc,mDSV.ReleaseAndGetAddressOf()));

	mShadowMapViewPort.Width = static_cast<float>(Width);
	mShadowMapViewPort.Height = static_cast<float>(Height);
	mShadowMapViewPort.MinDepth = 0.0f;
	mShadowMapViewPort.MaxDepth = 1.0f;
	mShadowMapViewPort.TopLeftX = 0.0f;
	mShadowMapViewPort.TopLeftY = 0.0f;

	mScreenSpaceShadowMapViewPort.Width = static_cast<float>(SSSMwidth);
	mScreenSpaceShadowMapViewPort.Height = static_cast<float>(SSSMheight);
	mScreenSpaceShadowMapViewPort.MinDepth = 0.0f;
	mScreenSpaceShadowMapViewPort.MaxDepth = 1.0f;
	mScreenSpaceShadowMapViewPort.TopLeftX = 0.0f;
	mScreenSpaceShadowMapViewPort.TopLeftY = 0.0f;

}
