#ifndef RENDERSTATES_H
#define RENDERSTATES_H


#include <d3d11.h>
#include <wrl.h>
using namespace Microsoft::WRL;

class RenderStates
{
	template <typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;
public:
	static void Init(ID3D11Device * device);
	static ComPtr<ID3D11RasterizerState> CullOff;			            // 光栅化器状态：无背面裁剪模式
	static ComPtr<ID3D11RasterizerState> DepthBias;	            // 光栅化器状态：顺时针裁剪模式
	static ComPtr<ID3D11SamplerState> LinearClamp;
	static ComPtr<ID3D11SamplerState> LinearWrap;			            // 采样器状态：线性过滤
	static ComPtr<ID3D11SamplerState> PointClamp;			           
	static ComPtr<ID3D11SamplerState> PointWarp;
	static ComPtr<ID3D11SamplerState> ComparisonState;		        // 采样器状态： 阴影PCF比较器采样
	static ComPtr<ID3D11DepthStencilState> DisableDepthBuffer;
	static ComPtr<ID3D11DepthStencilState> DepthLessEqual;		        // 深度/模板状态：允许绘制深度值相等的像素
	static ComPtr<ID3D11DepthStencilState> DisableDepthWrite;		        // 禁止深度写入
};
#endif