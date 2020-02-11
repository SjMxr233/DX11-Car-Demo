#include "Ssao.h"

Ssao::Ssao(ID3D11Device* device, int Width, int Height)
{
	BuildResource(device, Width, Height);
}
Ssao::~Ssao()
{

}
void Ssao::BuildResource(ID3D11Device* device, int Width, int Height)
{
	BuildOffsetVectors();
	BuildRandVecMap(device);
	D3D11_TEXTURE2D_DESC AoMapDesc;
	ZeroMemory(&AoMapDesc, sizeof(AoMapDesc));
	AoMapDesc.Width = Width;
	AoMapDesc.Height = Height;
	AoMapDesc.MipLevels = 1;
	AoMapDesc.ArraySize = 1;
	AoMapDesc.Format = DXGI_FORMAT_R32_FLOAT;
	AoMapDesc.SampleDesc.Count = 1;
	AoMapDesc.SampleDesc.Quality = 0;
	AoMapDesc.Usage = D3D11_USAGE_DEFAULT;
	AoMapDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	AoMapDesc.CPUAccessFlags = 0;
	AoMapDesc.MiscFlags = 0;
	HR(device->CreateTexture2D(&AoMapDesc,0,mAoMap.ReleaseAndGetAddressOf()));
	HR(device->CreateTexture2D(&AoMapDesc, 0, mAoBlurMap.ReleaseAndGetAddressOf()));

	D3D11_RENDER_TARGET_VIEW_DESC RTVDesc;
	ZeroMemory(&RTVDesc, sizeof(RTVDesc));
	RTVDesc.Format = AoMapDesc.Format;
	RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	RTVDesc.Texture2D.MipSlice = 0;
	HR(device->CreateRenderTargetView(mAoMap.Get(), &RTVDesc, mAoRTV.ReleaseAndGetAddressOf()));
	HR(device->CreateRenderTargetView(mAoBlurMap.Get(), &RTVDesc, mAoBlurRTV.ReleaseAndGetAddressOf()));


	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	ZeroMemory(&SRVDesc, sizeof(SRVDesc));
	SRVDesc.Format = AoMapDesc.Format;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MostDetailedMip = 0;
	SRVDesc.Texture2D.MipLevels = 1;
	HR(device->CreateShaderResourceView(mAoMap.Get(), &SRVDesc, mAoSRV.ReleaseAndGetAddressOf()));
	HR(device->CreateShaderResourceView(mAoBlurMap.Get(), &SRVDesc, mAoBlurSRV.ReleaseAndGetAddressOf()));

	mViewPort.Width = static_cast<float>(Width);
	mViewPort.Height = static_cast<float>(Height);
	mViewPort.MinDepth = 0.0f;
	mViewPort.MaxDepth = 1.0f;
	mViewPort.TopLeftX = 0.0f;
	mViewPort.TopLeftY = 0.0f;

}
void Ssao::BuildRandVecMap(ID3D11Device* device)
{
	D3D11_TEXTURE2D_DESC RandMapDesc;
	ZeroMemory(&RandMapDesc, sizeof(RandMapDesc));
	RandMapDesc.Width = 256;
	RandMapDesc.Height = 256;
	RandMapDesc.MipLevels = 1;
	RandMapDesc.ArraySize = 1;
	RandMapDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	RandMapDesc.SampleDesc.Count = 1;
	RandMapDesc.SampleDesc.Quality = 0;
	RandMapDesc.Usage = D3D11_USAGE_IMMUTABLE;
	RandMapDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	RandMapDesc.CPUAccessFlags = 0;
	RandMapDesc.MiscFlags = 0;

	XMFLOAT3 initData[256 * 256];
	for (int i = 0; i < 256; ++i)
	{
		for (int j = 0; j < 256; ++j)
		{
			// Random vector in [0,1].  We will decompress in shader to [-1,1].
			XMFLOAT3 v(MathHelper::RandF(), MathHelper::RandF(), MathHelper::RandF());

			initData[i * 256 + j] = XMFLOAT3(v.x, v.y, v.z);
		}
	}
	D3D11_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pSysMem = initData;
	subResourceData.SysMemPitch = 256 * sizeof(XMFLOAT3);
	HR(device->CreateTexture2D(&RandMapDesc, &subResourceData, mRandMap.ReleaseAndGetAddressOf()));
	HR(device->CreateShaderResourceView(mRandMap.Get(), 0, mRandSRV.ReleaseAndGetAddressOf()));
}
void Ssao::BuildOffsetVectors()
{
	for (int i = 0; i < 64; ++i)
	{
		float randomX = MathHelper::RandF() * 2.0f - 1.0f;
		float randomY = MathHelper::RandF()* 2.0f - 1.0f;
		float randomZ = MathHelper::RandF();
		XMVECTOR random = XMVectorSet(randomX, randomY, randomZ, 0.0f);
		random = XMVector3Normalize(random);
		float randomFactor = MathHelper::RandF();
		random = XMVectorMultiply(random, XMVectorSet(randomFactor, randomFactor, randomFactor, 0.0f));
		float scale = float(i) / 64.0;
		scale = MathHelper::Lerp(0.1f, 1.0f, scale*scale);
		random = XMVectorMultiply(random, XMVectorSet(scale, scale, scale, 0.0f));
		XMStoreFloat4(&mOffsets[i], random);
	}
}
