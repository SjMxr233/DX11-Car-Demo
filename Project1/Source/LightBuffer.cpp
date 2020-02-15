#include "LightBuffer.h"
LightBuffer::LightBuffer(ID3D11Device* device, int Width, int Height)
{
	BuildResource(device, Width, Height);
}
LightBuffer::~LightBuffer()
{

}
void LightBuffer::BuildResource(ID3D11Device* device, int Width, int Height)
{
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = Width;
	textureDesc.Height = Height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	HR(device->CreateTexture2D(&textureDesc, 0, mLightBufferMap.GetAddressOf()));

	D3D11_RENDER_TARGET_VIEW_DESC RTVDesc;
	ZeroMemory(&RTVDesc, sizeof(RTVDesc));
	RTVDesc.Format = textureDesc.Format;
	RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	RTVDesc.Texture2D.MipSlice = 0;
	HR(device->CreateRenderTargetView(mLightBufferMap.Get(), &RTVDesc, mRTV.GetAddressOf()));

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	ZeroMemory(&SRVDesc, sizeof(SRVDesc));
	SRVDesc.Format = textureDesc.Format;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MostDetailedMip = 0;
	SRVDesc.Texture2D.MipLevels = 1;
	HR(device->CreateShaderResourceView(mLightBufferMap.Get(), &SRVDesc, mSRV.GetAddressOf()));

	mViewPort.Width = static_cast<float>(Width);
	mViewPort.Height = static_cast<float>(Height);
	mViewPort.MinDepth = 0.0f;
	mViewPort.MaxDepth = 1.0f;
	mViewPort.TopLeftX = 0.0f;
	mViewPort.TopLeftY = 0.0f;
}