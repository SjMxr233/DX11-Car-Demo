#ifndef D3DUTIL_H
#define D3DUTIL_H

#include <d3d11.h>
#include <wrl.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <unordered_map>
#include <string>
#include <Windows.h>
#include "MathHelper.h"
#include "GeometryGenerator.h"
#include "DDSTextureLoader.h" 
#include "RenderStates.h"
class d3dUtil
{
public:
	static Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(
		const std::wstring& filename,
		const D3D_SHADER_MACRO* defines,
		const std::string& entrypoint,
		const std::string& target);
};
// ------------------------------
// DXTraceW����
// ------------------------------
// �ڵ�����������������ʽ��������Ϣ����ѡ�Ĵ��󴰿ڵ���(�Ѻ���)
// [In]strFile			��ǰ�ļ�����ͨ�����ݺ�__FILEW__
// [In]hlslFileName     ��ǰ�кţ�ͨ�����ݺ�__LINE__
// [In]hr				����ִ�г�������ʱ���ص�HRESULTֵ
// [In]strMsg			���ڰ������Զ�λ���ַ�����ͨ������L#x(����ΪNULL)
// [In]bPopMsgBox       ���ΪTRUE���򵯳�һ����Ϣ������֪������Ϣ
// ����ֵ: �β�hr
HRESULT WINAPI DXTraceW(_In_z_ const WCHAR* strFile, _In_ DWORD dwLine, _In_ HRESULT hr, _In_opt_ const WCHAR* strMsg, _In_ bool bPopMsgBox);


// ------------------------------
// HR��
// ------------------------------
// Debugģʽ�µĴ���������׷��
#if defined(DEBUG) | defined(_DEBUG)
#ifndef HR
#define HR(x)												\
		{															\
			HRESULT hr = (x);										\
			if(FAILED(hr))											\
			{														\
				DXTraceW(__FILEW__, (DWORD)__LINE__, hr, L#x, true);\
			}														\
		}
#endif
#else
#ifndef HR
#define HR(x) (x)
#endif 
#endif
#endif