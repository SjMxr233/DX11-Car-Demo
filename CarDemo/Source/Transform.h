#pragma once
#include "MathHelper.h"
#include <DirectXMath.h>
using namespace DirectX;
class Transform
{
public:
	Transform();
	~Transform();
	void SetParent(Transform *parent);
	void Update();

	XMFLOAT3 Position = XMFLOAT3(0, 0, 0);
	XMFLOAT3 Rotation = XMFLOAT3(0, 0, 0);

	XMFLOAT3 Scale = XMFLOAT3(1, 1, 1);

	XMFLOAT3 Forward = XMFLOAT3(0, 0, 1);
	XMFLOAT3 Up = XMFLOAT3(0, 1, 0);
	XMFLOAT3 Right = XMFLOAT3(1, 0, 0);

	XMFLOAT4X4 InitRot=MathHelper::Identity4x4();
	XMFLOAT4X4 World = MathHelper::Identity4x4();
	Transform* Parent = nullptr;

};
