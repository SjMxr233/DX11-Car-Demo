#pragma once
#include <DirectXMath.h>
#include "MathHelper.h"
using namespace DirectX;
class Camera
{
public:

	void SetPosition(float x, float y, float z);
	void SetPosition(const DirectX::FXMVECTOR& v);
	void SetLens(float fovY, float aspect, float zn, float zf);
	virtual void UpdateViewMatrix() =0;
	XMMATRIX GetView();
	XMMATRIX GetProj();
	XMFLOAT3 GetPosition();
	XMFLOAT3 GetY();
protected:
	XMFLOAT3 mPosition = { 0.0f, 0.0f, 0.0f };
	XMFLOAT3 mRight = { 1.0f, 0.0f, 0.0f };
	XMFLOAT3 mUp = { 0.0f, 1.0f, 0.0f };
	XMFLOAT3 mForward = { 0.0f, 0.0f, 1.0f };

	XMFLOAT4X4 mView = MathHelper::Identity4x4();
	XMFLOAT4X4 mProj = MathHelper::Identity4x4();

	bool mViewDirty = true;
};
class FirstPersonCamera :public Camera
{
public:
	void UpdateViewMatrix() override;
	void Pitch(float angle) ;
	void RotateY(float angle) ;
};
class ThirdPersonCamera :public Camera
{
public:
	void UpdateViewMatrix() override;
	void Pitch(float angle) ;
	void RotateY(float angle);
	void AdjustRadius(float r);
	void SetTarget(XMFLOAT3 target);
	void SetTarget(const FXMVECTOR& target);
private:
	XMFLOAT3 mTarget;
	float mRadius=6.0f;
	float mPhi=0.0f;
	float mTheta = 1.0f;
};