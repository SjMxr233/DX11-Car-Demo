#include "Camera.h"
XMFLOAT3 Camera::GetY()
{
	return mUp;
}
float Camera::GetFarZ()
{
	return mFarZ;
}
void Camera::SetPosition(float x, float y, float z)
{
	mPosition = XMFLOAT3(x, y, z);
	mViewDirty = true;
}
void Camera::SetPosition(const FXMVECTOR& v)
{
	XMStoreFloat3(&mPosition, v);
	mViewDirty = true;
}

XMFLOAT3 Camera::GetPosition()
{
	return mPosition;
}
void Camera::SetLens(float fovY, float aspect, float zn, float zf)
{
	mNearZ = zn;
	mFarZ = zf;
	XMMATRIX p = XMMatrixPerspectiveFovLH(fovY, aspect, zn, zf);
	XMStoreFloat4x4(&mProj, p);
	mViewDirty = true;
}
XMMATRIX Camera::GetView()
{
	XMMATRIX v = XMLoadFloat4x4(&mView);
	return v;
}
XMMATRIX Camera::GetProj()
{
	XMMATRIX p = XMLoadFloat4x4(&mProj);
	return p;
}
void FirstPersonCamera::Pitch(float angle)
{
	XMMATRIX R = XMMatrixRotationAxis(XMLoadFloat3(&mRight), angle);
	XMVECTOR Forward = XMVector3TransformNormal(XMLoadFloat3(&mForward), R);
	XMVECTOR up= XMVector3TransformNormal(XMLoadFloat3(&mUp), R);
	float cosTheta = XMVectorGetY(up); 
	if (cosTheta < 0.233f) return;
	XMStoreFloat3(&mUp,up);
	XMStoreFloat3(&mForward, Forward);
	mViewDirty = true;
}
void FirstPersonCamera::RotateY(float angle)
{
	XMMATRIX R = XMMatrixRotationY(angle);
	XMStoreFloat3(&mRight, XMVector3TransformNormal(XMLoadFloat3(&mRight), R));
	XMStoreFloat3(&mForward, XMVector3TransformNormal(XMLoadFloat3(&mForward), R));

	mViewDirty = true;
}
void FirstPersonCamera::UpdateViewMatrix()
{
	if (mViewDirty)
	{
		XMVECTOR R = XMLoadFloat3(&mRight);
		XMVECTOR U = XMLoadFloat3(&mUp);
		XMVECTOR L = XMLoadFloat3(&mForward);
		XMVECTOR P = XMLoadFloat3(&mPosition);

		// Keep camera's axes orthogonal to each other and of unit length.
		L = XMVector3Normalize(L);
		U = XMVector3Normalize(XMVector3Cross(L, R));

		// U, L already ortho-normal, so no need to normalize cross product.
		R = XMVector3Cross(U, L);

		// Fill in the view matrix entries.
		float x = -XMVectorGetX(XMVector3Dot(P, R));
		float y = -XMVectorGetX(XMVector3Dot(P, U));
		float z = -XMVectorGetX(XMVector3Dot(P, L));

		XMStoreFloat3(&mRight, R);
		XMStoreFloat3(&mUp, U);
		XMStoreFloat3(&mForward, L);

		mView(0, 0) = mRight.x;
		mView(1, 0) = mRight.y;
		mView(2, 0) = mRight.z;
		mView(3, 0) = x;

		mView(0, 1) = mUp.x;
		mView(1, 1) = mUp.y;
		mView(2, 1) = mUp.z;
		mView(3, 1) = y;

		mView(0, 2) = mForward.x;
		mView(1, 2) = mForward.y;
		mView(2, 2) = mForward.z;
		mView(3, 2) = z;

		mView(0, 3) = 0.0f;
		mView(1, 3) = 0.0f;
		mView(2, 3) = 0.0f;
		mView(3, 3) = 1.0f;

		mViewDirty = false;
	}
}
void ThirdPersonCamera::Pitch(float angle)
{
	mPhi += angle;
	mPhi = MathHelper::Clamp(mPhi, 0.1f, 1.5f);
	mViewDirty = true;
}
void ThirdPersonCamera::RotateY(float angle)
{
	mTheta += angle;
	mViewDirty = true;
}
void ThirdPersonCamera::SetTarget(XMFLOAT3 target)
{
	mTarget = target;
	mViewDirty = true;
}
void ThirdPersonCamera::SetTarget(const FXMVECTOR& target)
{
	XMStoreFloat3(&mTarget, target);
	mViewDirty = true;
}
void ThirdPersonCamera::AdjustRadius(float r)
{
	mRadius -= r;
	mRadius = MathHelper::Clamp(mRadius, 4.0f, 8.0f);
	mViewDirty = true;
}
void ThirdPersonCamera::UpdateViewMatrix()
{
	if (mViewDirty)
	{
		XMVECTOR T = XMLoadFloat3(&mTarget);
		XMVECTOR P = MathHelper::SphericalToCartesian(mRadius, mTheta, mPhi)+T;
		XMVECTOR L = XMVectorSubtract(T, P);
		XMStoreFloat3(&mPosition, P);
		XMVECTOR R = XMVector3Normalize(XMVector3Cross(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), L));
		XMVECTOR U = XMLoadFloat3(&mUp);
		U = XMVector3Normalize(XMVector3Cross(L, R));

		float x = -XMVectorGetX(XMVector3Dot(P, R));
		float y = -XMVectorGetX(XMVector3Dot(P, U));
		float z = -XMVectorGetX(XMVector3Dot(P, L));

		XMStoreFloat3(&mRight, R);
		XMStoreFloat3(&mUp, U);
		XMStoreFloat3(&mForward, L);

		mView(0, 0) = mRight.x;
		mView(1, 0) = mRight.y;
		mView(2, 0) = mRight.z;
		mView(3, 0) = x;

		mView(0, 1) = mUp.x;
		mView(1, 1) = mUp.y;
		mView(2, 1) = mUp.z;
		mView(3, 1) = y;

		mView(0, 2) = mForward.x;
		mView(1, 2) = mForward.y;
		mView(2, 2) = mForward.z;
		mView(3, 2) = z;

		mView(0, 3) = 0.0f;
		mView(1, 3) = 0.0f;
		mView(2, 3) = 0.0f;
		mView(3, 3) = 1.0f;

		mViewDirty = false;
	}

}