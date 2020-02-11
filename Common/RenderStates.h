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
	static ComPtr<ID3D11RasterizerState> CullOff;			            // ��դ����״̬���ޱ���ü�ģʽ
	static ComPtr<ID3D11RasterizerState> DepthBias;	            // ��դ����״̬��˳ʱ��ü�ģʽ
	static ComPtr<ID3D11SamplerState> LinearClamp;
	static ComPtr<ID3D11SamplerState> LinearWrap;			            // ������״̬�����Թ���
	static ComPtr<ID3D11SamplerState> PointClamp;			           
	static ComPtr<ID3D11SamplerState> PointWarp;
	static ComPtr<ID3D11SamplerState> ComparisonState;		        // ������״̬�� ��ӰPCF�Ƚ�������
	static ComPtr<ID3D11DepthStencilState> DisableDepthBuffer;
	static ComPtr<ID3D11DepthStencilState> DepthLessEqual;		        // ���/ģ��״̬������������ֵ��ȵ�����
	static ComPtr<ID3D11DepthStencilState> DisableDepthWrite;		        // ��ֹ���д��
};
#endif