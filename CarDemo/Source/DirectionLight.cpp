#include "DirectionLight.h"
DirectionLight::DirectionLight(XMFLOAT4 color, XMFLOAT3 direction)
	:Color(color),Direction(direction)
{

}
DirectionLight::~DirectionLight()
{

}
XMMATRIX DirectionLight::GetLightView()
{
	return XMLoadFloat4x4(&mLightView);
}
XMMATRIX DirectionLight::GetLightProj()
{
	return XMLoadFloat4x4(&mLightProj);
}
void DirectionLight::SetLightBounding(XMFLOAT3 center, float r)
{
	mSceneBound.Radius = r;
	mSceneBound.Center = center;
}
void DirectionLight::SetLightViewAndProj()
{
	XMVECTOR lightDir = XMLoadFloat3(&Direction);
	XMVECTOR lightPos = 1.0f*mSceneBound.Radius*lightDir;
	XMVECTOR targetPos = XMLoadFloat3(&mSceneBound.Center);
	XMVECTOR lightUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMStoreFloat4x4(&mLightView, XMMatrixLookAtLH(lightPos, targetPos, lightUp));
	// Transform bounding sphere to light space.
	XMFLOAT3 sphereCenterLS;
	XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(targetPos, GetLightView()));

	// Ortho frustum in light space encloses scene.
	float l = sphereCenterLS.x - mSceneBound.Radius;
	float b = sphereCenterLS.y - mSceneBound.Radius;
	float n = sphereCenterLS.z - mSceneBound.Radius;
	float r = sphereCenterLS.x + mSceneBound.Radius;
	float t = sphereCenterLS.y + mSceneBound.Radius;
	float f = sphereCenterLS.z + mSceneBound.Radius;
	XMStoreFloat4x4(&mLightProj, XMMatrixOrthographicOffCenterLH(l, r, b, t, f, n));//交换投影矩阵远近平面,reverse-z
}