#pragma once
#include "d3dUtil.h"
struct BoundingSphere
{
	float Radius;
	XMFLOAT3 Center;
};
class DirectionLight
{
public:
	DirectionLight(XMFLOAT4 color,XMFLOAT3 direction);
	~DirectionLight();

	void SetLightBounding(XMFLOAT3 center, float r);
	void SetLightViewAndProj();

	XMMATRIX GetLightView();
	XMMATRIX GetLightProj();
	XMFLOAT4 Color;
	XMFLOAT3 Direction;

protected:
	XMFLOAT4X4 mLightView;
	XMFLOAT4X4 mLightProj;
	BoundingSphere mSceneBound;

};