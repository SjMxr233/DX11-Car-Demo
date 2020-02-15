#include "Transform.h"

Transform::Transform()
{

}

Transform::~Transform()
{

}
void Transform::SetParent(Transform *parent)
{
	Parent = parent;
}
void Transform::Update()
{
	XMMATRIX scale = XMMatrixScaling(Scale.x, Scale.y, Scale.z);
	
	XMMATRIX rotation = XMLoadFloat4x4(&InitRot)*XMMatrixRotationX(Rotation.x)*XMMatrixRotationY(Rotation.y)  * XMMatrixRotationZ(Rotation.z);
	XMMATRIX translation = XMMatrixTranslation(Position.x, Position.y, Position.z);
	XMStoreFloat4x4(&World, scale * rotation * translation);

	XMVECTOR forward = XMVectorSet(0, 0, 1, 0);
	forward = XMVector3TransformNormal(forward, rotation);
	XMStoreFloat3(&Forward, forward);

	if (Parent != nullptr)
	{
		XMStoreFloat4x4(&World, XMLoadFloat4x4(&World)* XMLoadFloat4x4(&Parent->World));
	}
}