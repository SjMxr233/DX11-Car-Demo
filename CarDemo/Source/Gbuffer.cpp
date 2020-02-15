#include "Gbuffer.h"
Gbuffer::Gbuffer(ID3D11Device* device, int Width, int Height)
{
	BuildResource(device, Width, Height);
}
Gbuffer::~Gbuffer()
{

}
void Gbuffer::BuildResource(ID3D11Device* device, int Width, int Height)
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

	D3D11_RENDER_TARGET_VIEW_DESC RTVDesc;
	ZeroMemory(&RTVDesc, sizeof(RTVDesc));
	RTVDesc.Format = textureDesc.Format;
	RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	RTVDesc.Texture2D.MipSlice = 0;

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	ZeroMemory(&SRVDesc, sizeof(SRVDesc));
	SRVDesc.Format = textureDesc.Format;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MostDetailedMip = 0;
	SRVDesc.Texture2D.MipLevels = 1;
	for (int i = 0; i < BUFFER_COUNT; i++)
	{
		HR(device->CreateTexture2D(&textureDesc, 0, mTexs[i].ReleaseAndGetAddressOf()));
		HR(device->CreateRenderTargetView(mTexs[i].Get(), &RTVDesc, mRTVs[i].ReleaseAndGetAddressOf()));
		HR(device->CreateShaderResourceView(mTexs[i].Get(), &SRVDesc, mSRVs[i].ReleaseAndGetAddressOf()));
	}
	D3D11_TEXTURE2D_DESC depthDesc;
	ZeroMemory(&depthDesc, sizeof(depthDesc));
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.Width = Width;
	depthDesc.Height = Height;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.SampleDesc.Quality = 0;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthDesc.CPUAccessFlags = 0;
	depthDesc.MiscFlags = 0;
	HR(device->CreateTexture2D(&depthDesc, 0, mDSB.ReleaseAndGetAddressOf()));

	D3D11_DEPTH_STENCIL_VIEW_DESC DSVDesc;
	ZeroMemory(&DSVDesc, sizeof(DSVDesc));
	DSVDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DSVDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	DSVDesc.Texture2D.MipSlice = 0;
	HR(device->CreateDepthStencilView(mDSB.Get(), &DSVDesc, mDSV.ReleaseAndGetAddressOf()));

	mViewPort.Width = static_cast<float>(Width);
	mViewPort.Height = static_cast<float>(Height);
	mViewPort.MinDepth = 0.0f;
	mViewPort.MaxDepth = 1.0f;
	mViewPort.TopLeftX = 0.0f;
	mViewPort.TopLeftY = 0.0f;
}
void Gbuffer::ClearRenderTargets(ID3D11DeviceContext *context,float FarZ)
{
	float color[4] = { 0,0,0,1 };
	context->ClearRenderTargetView(this->mRTVs[0].Get(), color);
	context->ClearRenderTargetView(this->mRTVs[2].Get(), color);

	color[2] = -1;
	color[3] = FarZ;
	context->ClearRenderTargetView(this->mRTVs[1].Get(), color);


}