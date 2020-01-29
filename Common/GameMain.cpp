#include "ConstantBuffer.h"
#include "d3dApp.h"
#include "RenderItem.h"
#include "Camera.h"
#include "ShadowMap.h"
using namespace DirectX;
enum CameraMode
{
	First,
	Third 
};
struct BoundingSphere
{
	float Radius;
	XMFLOAT3 Center;
};
class GameMain : public D3DApp
{
public:
	GameMain(HINSTANCE hInstance);
	~GameMain();
	//主要运行函数
	bool Init(); 
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseMove(WPARAM btnState, int x, int y) override;
	void OnMouseWheel(WPARAM btnState) override;
	void PlayerControl(const float dt);
	void CameraControl(const float dt);
	void LightControl(const float dt);
private:
	void BuildGeometry(); //初始化所用模型数据
	void BuildShaderAndInputLayout(); //初始化所用着色器
	void BuildObjectCBuffer(); //创建顶点变换常量缓存区
	void BuildLightCBuffer(); //创建光照计算常量缓存区
	void BuildTexture(); //初始化所用贴图数据
	void BuildRenderItem(); //场景中所有渲染项设置
	void BuildLightAndMaterial(); //初始化灯光属性和所用材质
	void BuildCamera(); 

	//更新所用到的常量缓存区数据
	void UpdateLightCBuffer(); 
	void UpdateObjectCBuffer();
	void UpdateShadowTransform();

	void RenderGeometry(); //渲染场景中的物体
	void RenderOpaque(); //渲染不透明项
	void RenderShadow(); //将可投射阴影的物体渲染到阴影贴图
	void RenderSkyBox(); 
	void RenderDebugWindow(); //渲染调试窗口观察灯光角度的深度图
private:

	//存储所有可能用到的材质、贴图、着色器、几何数据
	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
	std::unordered_map<std::string, std::unique_ptr<Material>> mMaterials;
	std::unordered_map<std::string, std::unique_ptr<Shader>> mShaders;
	std::unordered_map<std::string, std::unique_ptr<Texture>> mTextures;

	//小车参数
	std::shared_ptr<RenderItem> mPlayer;
	std::shared_ptr<RenderItem> mFrontLeftWheel;
	std::shared_ptr<RenderItem> mFrontRightWheel;
	std::shared_ptr<RenderItem> mBackLeftWheel;
	std::shared_ptr<RenderItem> mBackRightWheel;
	float mSteerAngle, mWheelRoll, mWheelSteer; //记录车身转向，车轮滚动，车轮转向角度
	float mVelocity = 0.0f;
	float mMaxVelocity = 35.0f;

	std::shared_ptr<RenderItem> mSkyBox;
	std::shared_ptr<RenderItem> mDebugWindow;
	std::vector<std::shared_ptr<RenderItem>> GameObjects;

	//第一人称和第三人称摄像机参数
	FirstPersonCamera mFirstCamera;
	ThirdPersonCamera mThirdCamera;
	CameraMode cameraMode = Third;
	XMMATRIX mCameraView;
	XMMATRIX mCameraProj;
	 
	DirectionalLight DirectLight;
	XMFLOAT3 EyePos;

	//阴影效果参数
	std::unique_ptr<ShadowMap> mShadowMap;
	BoundingSphere mSceneBound;
	XMMATRIX mLightView ;
	XMMATRIX mLightProj ;
	XMFLOAT4X4 mShadowTransform = MathHelper::Identity4x4();
	bool DrawShadow = false;

	POINT mLastMousePos;
	float mLastDelta;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	GameMain theApp(hInstance);

	if (!theApp.Init())
		return 0;
	return theApp.Run();
}

GameMain::GameMain(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
	
}

GameMain::~GameMain()
{

}
void GameMain::BuildGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData box;
	GeometryGenerator::MeshData grid;
	GeometryGenerator::MeshData sphere;
	GeometryGenerator::MeshData quad;
	GeometryGenerator::MeshData cylinder;
	//存放子模型,共用同一个顶点缓存区
#pragma region Player

	std::vector<GeometryGenerator::MeshData> Meshdatas_Player; 
	geoGen.CreateBox(2.0f, 0.6f, 3.0f, box);
	box.name = "box";
	Meshdatas_Player.push_back(box);
	//geoGen.CreateSphere(1.0f, 20, 20, cylinder);
	geoGen.CreateCylinder(0.5f, 0.5f, 0.5f, 20, 20, cylinder);
	cylinder.name = "cylinder";
	Meshdatas_Player.push_back(cylinder);
	auto player = std::make_unique<MeshGeometry>();
	player->name = "Player";
	player->Stride = sizeof(Vertex);
	player->Offet = 0;
	player->SetBuffer(md3dDevice.Get(), Meshdatas_Player);
	mGeometries[player->name] = std::move(player);
#pragma endregion
#pragma region floor

	std::vector<GeometryGenerator::MeshData> Meshdatas_Floor;
	geoGen.CreateGrid(100.0f, 100.0f, 2, 2, grid);
	grid.name = "grid";
	Meshdatas_Floor.push_back(grid);
	auto floor = std::make_unique<MeshGeometry>();
	floor->name = "Floor";
	floor->Stride = sizeof(Vertex);
	floor->Offet = 0;
	floor->SetBuffer(md3dDevice.Get(), Meshdatas_Floor);
	mGeometries[floor->name] = std::move(floor);
#pragma endregion
#pragma region DebugWindow
	std::vector<GeometryGenerator::MeshData> Meshdatas_DebugWindow;
	geoGen.CreateScreenQuad(quad, 0.2, XMFLOAT2(0.8, -0.8));
	quad.name = "quad";
	Meshdatas_DebugWindow.push_back(quad);
	auto debugwindow = std::make_unique<MeshGeometry>();
	debugwindow->name = "DebugWindow";
	debugwindow->Stride = sizeof(Vertex);
	debugwindow->Offet = 0;
	debugwindow->SetBuffer(md3dDevice.Get(), Meshdatas_DebugWindow);
	mGeometries[debugwindow->name] = std::move(debugwindow);
#pragma endregion
#pragma region SkyBox
	std::vector<GeometryGenerator::MeshData> Meshdatas_SkyBox;
	geoGen.CreateBox(200.0f, 200.0f, 200.0f, box);
	box.name = "box";
	Meshdatas_SkyBox.push_back(box);
	auto skybox = std::make_unique<MeshGeometry>();
	skybox->name = "SkyBox";
	skybox->Stride = sizeof(Vertex);
	skybox->Offet = 0;
	skybox->SetBuffer(md3dDevice.Get(), Meshdatas_SkyBox);
	mGeometries[skybox->name] = std::move(skybox);
#pragma endregion
}
void GameMain::BuildShaderAndInputLayout()
{
	auto Color = std::make_unique<Shader>();
	Color->name = "Color";
	Color->CreatShaderAndLayout(md3dDevice.Get(), L"shader\\Color.hlsl");

	auto Sky = std::make_unique<Shader>();
	Sky->name = "Sky";
	Sky->CreatShaderAndLayout(md3dDevice.Get(), L"shader\\SkyBox.hlsl");

	auto LightDepth = std::make_unique<Shader>();
	LightDepth->name = "LightDepth";
	LightDepth->CreatShaderAndLayout(md3dDevice.Get(), L"shader\\LightDepth.hlsl");

	auto DebugShadow = std::make_unique<Shader>();
	DebugShadow->name = "DebugShadow";
	DebugShadow->CreatShaderAndLayout(md3dDevice.Get(), L"shader\\DebugShadow.hlsl");

	mShaders[Color->name] = std::move(Color);
	mShaders[Sky->name] = std::move(Sky);
	mShaders[LightDepth->name] = std::move(LightDepth);
	mShaders[DebugShadow->name] = std::move(DebugShadow);
}
void GameMain::BuildObjectCBuffer()
{
	for (auto &e : GameObjects)
	{
		e->CreatObjectCBuffer(md3dDevice.Get(), sizeof(ObjectConstant));
	}
	mSkyBox->CreatObjectCBuffer(md3dDevice.Get(), sizeof(ObjectConstant));
}
void GameMain::BuildLightCBuffer()
{
	for (auto &e : GameObjects)
	{
		e->CreatLightCBuffer(md3dDevice.Get(), sizeof(ObjectConstant));
	}
}
void GameMain::BuildTexture()
{
	auto zero = std::make_unique<Texture>();
	zero->name = "zero";
	HR(CreateDDSTextureFromFile(md3dDevice.Get(), L"Tex\\zero.dds", nullptr, zero->mTextureView.GetAddressOf()));

	auto wheel = std::make_unique<Texture>();
	wheel->name = "wheel";
	HR(CreateDDSTextureFromFile(md3dDevice.Get(), L"Tex\\wheel.dds", nullptr, wheel->mTextureView.GetAddressOf()));

	auto floor = std::make_unique<Texture>();
	floor->name = "floor";
	HR(CreateDDSTextureFromFile(md3dDevice.Get(), L"Tex\\floor.dds", nullptr, floor->mTextureView.GetAddressOf()));

	auto sky = std::make_unique<Texture>();
	sky->name = "skybox";
	HR(CreateDDSTextureFromFile(md3dDevice.Get(), L"Tex\\skybox.dds", nullptr, sky->mTextureView.GetAddressOf()));

	auto shadowMap = std::make_unique<ShadowMap>(md3dDevice.Get(), 4096, 4096);
	mShadowMap = std::move(shadowMap);
	auto shadow = std::make_unique<Texture>();
	shadow->name = "shadow";
	shadow->mTextureView = mShadowMap->mSRV;

	mTextures[sky->name] = std::move(sky);
	mTextures[zero->name] = std::move(zero);
	mTextures[wheel->name] = std::move(wheel);
	mTextures[floor->name] = std::move(floor);
	mTextures[shadow->name] = std::move(shadow);
}
void GameMain::BuildLightAndMaterial()
{
	DirectLight.Direction = XMFLOAT3(0.8f, 0.5f, 0.5f);
	DirectLight.LightColor = XMFLOAT4(0.8f, 0.8f, 0.7f, 1.0f);

	auto zero = std::make_unique<Material>();
	zero->_Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	zero->_Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	zero->_offset = XMFLOAT2(0, 0);
	zero->_tiling = XMFLOAT2(1, 1);

	auto floor = std::make_unique<Material>();
	floor->_Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	floor->_Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	floor->_offset = XMFLOAT2(0, 0);
	floor->_tiling = XMFLOAT2(3, 3);

	mMaterials["zero"] = std::move(zero);
	mMaterials["floor"] = std::move(floor);
}
void GameMain::BuildRenderItem()
{
	auto bodyObject = std::make_shared<RenderItem>();
	bodyObject->mesh = mGeometries["Player"].get();
	bodyObject->mat = mMaterials["zero"].get();
	bodyObject->tex = mTextures["zero"].get();
	bodyObject->shader = mShaders["Color"].get();
	bodyObject->SetMeshAgrs("box");
	mPlayer = bodyObject;
	GameObjects.push_back(bodyObject);
	
	auto flwheelObject = std::make_shared<RenderItem>();
	flwheelObject->mesh = mGeometries["Player"].get();
	flwheelObject->mat = mMaterials["zero"].get();
	flwheelObject->tex = mTextures["wheel"].get();
	flwheelObject->shader = mShaders["Color"].get();
	flwheelObject->SetMeshAgrs("cylinder");
	mFrontLeftWheel = flwheelObject;
	GameObjects.push_back(flwheelObject);

	auto frwheelObject = std::make_shared<RenderItem>();
	frwheelObject->mesh = mGeometries["Player"].get();
	frwheelObject->mat = mMaterials["zero"].get();
	frwheelObject->tex = mTextures["wheel"].get();
	frwheelObject->shader = mShaders["Color"].get();
	frwheelObject->SetMeshAgrs("cylinder");
	mFrontRightWheel = frwheelObject;
	GameObjects.push_back(frwheelObject);

	auto blwheelObject = std::make_shared<RenderItem>();
	blwheelObject->mesh = mGeometries["Player"].get();
	blwheelObject->mat = mMaterials["zero"].get();
	blwheelObject->tex = mTextures["wheel"].get();
	blwheelObject->shader = mShaders["Color"].get();
	blwheelObject->SetMeshAgrs("cylinder");
	mBackLeftWheel = blwheelObject;
	GameObjects.push_back(blwheelObject);

	auto brwheelObject = std::make_shared<RenderItem>();
	brwheelObject->mesh = mGeometries["Player"].get();
	brwheelObject->mat = mMaterials["zero"].get();
	brwheelObject->tex = mTextures["wheel"].get();
	brwheelObject->shader = mShaders["Color"].get();
	brwheelObject->SetMeshAgrs("cylinder");
	mBackRightWheel = brwheelObject;
	GameObjects.push_back(brwheelObject);

	auto gridObject = std::make_shared<RenderItem>();
	gridObject->SetWorld(XMMatrixTranslation(0, -1.0f, 0));
	gridObject->mesh = mGeometries["Floor"].get();
	gridObject->mat = mMaterials["floor"].get();
	gridObject->tex = mTextures["floor"].get();
	gridObject->shader = mShaders["Color"].get();
	gridObject->SetMeshAgrs("grid");
	GameObjects.push_back(gridObject);

	auto DebugObject = std::make_shared<RenderItem>();
	DebugObject->mesh = mGeometries["DebugWindow"].get();
	DebugObject->tex = mTextures["shadow"].get();
	DebugObject->shader = mShaders["DebugShadow"].get();
	DebugObject->SetMeshAgrs("quad");
	mDebugWindow = DebugObject;

	auto SkyObject = std::make_shared<RenderItem>();
	SkyObject->mesh = mGeometries["SkyBox"].get();
	SkyObject->tex = mTextures["skybox"].get();
	SkyObject->shader = mShaders["Sky"].get();
	SkyObject->SetMeshAgrs("box");
	mSkyBox = SkyObject;

	BuildObjectCBuffer();
	BuildLightCBuffer();
}
void GameMain::BuildCamera()
{
	mFirstCamera.SetLens(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	mThirdCamera.SetLens(0.1f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
}
bool GameMain::Init()
{
	if (!D3DApp::Init())
		return false;
	RenderStates::Init(md3dDevice.Get());
	mSceneBound.Center = XMFLOAT3(0, 0, 0);
	//根据最宽的物体网格的大小决定整个场景包围球的大小
	mSceneBound.Radius = sqrt(50 * 50 + 50 * 50);
	BuildTexture();
	BuildShaderAndInputLayout();
	BuildGeometry();
	BuildLightAndMaterial();
	BuildRenderItem();
	BuildCamera();

	return true;
}
void GameMain::OnMouseMove(WPARAM btnState, int x, int y)
{
	float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
	float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

	if (cameraMode == First)
	{
		mFirstCamera.Pitch(dy);
		mFirstCamera.RotateY(dx);
	}
	if (cameraMode == Third)
	{
		mThirdCamera.Pitch(dy);
		mThirdCamera.RotateY(dx);
	}
	mLastMousePos.x = x;
	mLastMousePos.y = y;
}
void GameMain::OnMouseWheel(WPARAM btnState)
{
	if (cameraMode == Third)
	{
		float zDelta = GET_WHEEL_DELTA_WPARAM(btnState);
		mThirdCamera.AdjustRadius(zDelta/240);
	}
}
void GameMain::LightControl(const float dt)
{
	XMVECTOR Direction = XMLoadFloat3(&DirectLight.Direction);
	Direction = XMVector3TransformNormal(Direction, XMMatrixRotationY(0.88f*dt));
	XMStoreFloat3(&DirectLight.Direction, Direction);
}
void GameMain::CameraControl(const float dt)
{
	if (GetAsyncKeyState('T') & 0x8000 && cameraMode == First)
	{
		cameraMode = Third;
		mThirdCamera.SetLens(0.1f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	}
	if (GetAsyncKeyState('F') & 0x8000 && cameraMode == Third)
	{
		cameraMode = First;
		mFirstCamera.SetLens(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	}
	if (cameraMode == First)
	{
		XMMATRIX w = XMLoadFloat4x4(&mPlayer->World)*XMMatrixTranslation(0,3.0f,0);
		XMVECTOR target = XMVector3TransformCoord(XMVectorZero(), w);
		mFirstCamera.SetPosition(target);
		mFirstCamera.UpdateViewMatrix();
		mCameraView = mFirstCamera.GetView();
		mCameraProj = mFirstCamera.GetProj();
		EyePos = mFirstCamera.GetPosition();
	}
	if (cameraMode == Third)
	{
		XMMATRIX w = XMLoadFloat4x4(&mPlayer->World);
		XMVECTOR target = XMVector3TransformCoord(XMVectorZero(), w);
		mThirdCamera.SetTarget(target);
		mThirdCamera.UpdateViewMatrix();
		mCameraView = mThirdCamera.GetView();
		mCameraProj = mThirdCamera.GetProj();
		EyePos = mThirdCamera.GetPosition();
	}
}
void GameMain::PlayerControl(const float dt)
{
	//设定绑定点,创建绑定点到车身模型空间的变换矩阵
	XMFLOAT3 offset = XMFLOAT3(1.5f, -0.5f, 1.6f);
	XMVECTOR flBindPos = XMVectorSet(-offset.x, offset.y, offset.z, 1);
	XMVECTOR frBindPos = XMVectorSet(offset.x, offset.y, offset.z, 1);
	XMVECTOR blBindPos = XMVectorSet(-offset.x, offset.y, -offset.z, 1);
	XMVECTOR brBindPos = XMVectorSet(offset.x, offset.y, -offset.z, 1);
	XMMATRIX Rot = XMMatrixRotationZ(0.5f*MathHelper::Pi);
	XMMATRIX RwheelHorizontal = XMMatrixRotationX(mWheelSteer);
	XMMATRIX RwheelVertical = XMMatrixRotationY(mWheelRoll);
	mFrontLeftWheel->SetLocalToModel(RwheelVertical * RwheelHorizontal*Rot*XMMatrixTranslationFromVector(flBindPos));
	mFrontRightWheel->SetLocalToModel(RwheelVertical * RwheelHorizontal*Rot*XMMatrixTranslationFromVector(frBindPos));
	mBackLeftWheel->SetLocalToModel(RwheelVertical * Rot*XMMatrixTranslationFromVector(blBindPos));
	mBackRightWheel->SetLocalToModel(RwheelVertical * Rot*XMMatrixTranslationFromVector(brBindPos));

	XMVECTOR position = XMLoadFloat3(&mPlayer->mPosition);
	XMMATRIX Rbody = XMMatrixRotationY(mSteerAngle);
	XMVECTOR forward = XMLoadFloat3(&mPlayer->mForward);
	forward = XMVector3TransformNormal(forward, Rbody);
	if (GetAsyncKeyState('W') & 0x8000)
	{
		//线性提速
		mVelocity = MathHelper::Lerp(mVelocity, mMaxVelocity, 0.0005f);
		if (GetAsyncKeyState('A') & 0x8000)
		{
			mSteerAngle -= 0.07f*mVelocity *dt;
		}
		else if (GetAsyncKeyState('D') & 0x8000)
		{
			mSteerAngle += 0.07f* mVelocity *dt;
		}
	}
	if (GetAsyncKeyState('S') & 0x8000)
	{
		mVelocity = MathHelper::Lerp(mVelocity, -mMaxVelocity, 0.0005f);
		if (GetAsyncKeyState('A') & 0x8000)
		{
			mSteerAngle -= 0.07f*mVelocity * dt;
		}
		if (GetAsyncKeyState('D') & 0x8000)
		{
			mSteerAngle += 0.07f*mVelocity * dt;
		}
	}
	if (GetAsyncKeyState('A') & 0x8000)
	{
		mWheelSteer -= 2.0f*dt;
		mWheelSteer = MathHelper::Clamp(mWheelSteer, -0.2f*MathHelper::Pi, 0.0f);
	}
	else if (GetAsyncKeyState('D') & 0x8000)
	{
		mWheelSteer += 2.0f*dt;
		mWheelSteer = MathHelper::Clamp(mWheelSteer, 0.0f, 0.2f*MathHelper::Pi);
	}
	else
	{
		mWheelSteer = MathHelper::Lerp(mWheelSteer, 0.0f, 0.02f);
	}
	position += forward * mVelocity *dt;
	mWheelRoll -= 1.5f*dt* mVelocity;
	mVelocity = MathHelper::Lerp(mVelocity, 0.0f, 0.002f);

	XMMATRIX W = Rbody*XMMatrixTranslationFromVector(position);
	XMStoreFloat3(&mPlayer->mPosition, position);
	mPlayer->SetWorld(W);
	//网易游戏学院3d基础视频中的绑定方法:
	//绑定点到车身模型坐标系矩阵*车身世界矩阵=车轮世界矩阵
	mFrontLeftWheel->SetWorld(XMLoadFloat4x4(&mFrontLeftWheel->LocalToModelMatrix)*W);
	mFrontRightWheel->SetWorld(XMLoadFloat4x4(&mFrontRightWheel->LocalToModelMatrix)*W);
	mBackLeftWheel->SetWorld(XMLoadFloat4x4(&mBackLeftWheel->LocalToModelMatrix)*W);
	mBackRightWheel->SetWorld(XMLoadFloat4x4(&mBackRightWheel->LocalToModelMatrix)*W);
}
void GameMain::OnResize()
{
	D3DApp::OnResize();	
	if(cameraMode==First)
		mFirstCamera.SetLens(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	if(cameraMode==Third)
		mThirdCamera.SetLens(0.1f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);

}
void GameMain::UpdateShadowTransform()
{
	XMVECTOR lightDir = XMLoadFloat3(&DirectLight.Direction);
	XMVECTOR lightPos = 2.0f*mSceneBound.Radius*lightDir;
	XMVECTOR targetPos = XMLoadFloat3(&mSceneBound.Center);
	XMVECTOR lightUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	mLightView = XMMatrixLookAtLH(lightPos, targetPos, lightUp);

	// Transform bounding sphere to light space.
	XMFLOAT3 sphereCenterLS;
	XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(targetPos, mLightView));

	// Ortho frustum in light space encloses scene.
	float l = sphereCenterLS.x - mSceneBound.Radius;
	float b = sphereCenterLS.y - mSceneBound.Radius;
	float n = sphereCenterLS.z - mSceneBound.Radius;
	float r = sphereCenterLS.x + mSceneBound.Radius;
	float t = sphereCenterLS.y + mSceneBound.Radius;
	float f = sphereCenterLS.z + mSceneBound.Radius;
	mLightProj = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);

	// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
	XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	XMMATRIX S = mLightView * mLightProj*T;
	XMStoreFloat4x4(&mShadowTransform, S);
}
void GameMain::UpdateLightCBuffer()
{
	D3D11_MAPPED_SUBRESOURCE mapData;
	for (auto &e : GameObjects)
	{
		ObjectLight LightData;
		LightData.gDirLight.Direction = DirectLight.Direction;
		LightData.gDirLight.LightColor = DirectLight.LightColor;
		LightData.gMaterial._Diffuse = e->mat->_Diffuse;
		LightData.gMaterial._Specular = e->mat->_Specular;
		LightData.gMaterial._offset = e->mat->_offset;
		LightData.gMaterial._tiling = e->mat->_tiling;
		LightData.gEyePos = EyePos;
		HR(md3dImmediateContext->Map(e->mLightCbuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapData));
		memcpy_s(mapData.pData, sizeof(LightData), &LightData, sizeof(LightData));
		md3dImmediateContext->Unmap(e->mLightCbuffer.Get(), 0);
	}
}
void GameMain::UpdateObjectCBuffer()
{
	D3D11_MAPPED_SUBRESOURCE mapData;
	UpdateShadowTransform();
	for (auto &e : GameObjects)
	{
		ObjectConstant ObjectData;
		XMMATRIX world = XMLoadFloat4x4(&e->World);
		XMMATRIX view, proj;
		if (DrawShadow)
		{
			view = mLightView;
			proj = mLightProj;
		}
		else
		{
			view = mCameraView;
			proj = mCameraProj;
		}
		XMMATRIX mvp = world * view * proj;
		XMMATRIX S = XMLoadFloat4x4(&mShadowTransform);
		XMStoreFloat4x4(&ObjectData.MVP, XMMatrixTranspose(mvp));
		XMStoreFloat4x4(&ObjectData.gModel, XMMatrixTranspose(world));
		XMStoreFloat4x4(&ObjectData.gModelInvTranspose, XMMatrixInverse(nullptr, world));
		XMStoreFloat4x4(&ObjectData.gShadowTransform, XMMatrixTranspose(S));
		HR(md3dImmediateContext->Map(e->mObjectCbuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapData));
		memcpy_s(mapData.pData, sizeof(ObjectData), &ObjectData, sizeof(ObjectData));
		md3dImmediateContext->Unmap(e->mObjectCbuffer.Get(), 0);
	}
}
void GameMain::UpdateScene(float dt)
{
	PlayerControl(dt);
	CameraControl(dt);
	LightControl(dt);
}
void GameMain::RenderGeometry()
{
	UpdateObjectCBuffer();
	UpdateLightCBuffer();
	for (auto &e : GameObjects)
	{
		e->SetRenderDefaultAgrs(md3dImmediateContext.Get());
		md3dImmediateContext->PSSetShaderResources(1, 1, mShadowMap->mSRV.GetAddressOf());
		md3dImmediateContext->PSSetSamplers(1, 1, RenderStates::ComparisonState.GetAddressOf());
		md3dImmediateContext->DrawIndexed(e->IndexCount, e->StartIndexLocation, e->BaseVertexLocation);
	}
}
void GameMain::RenderDebugWindow()
{
	mDebugWindow->SetRenderDefaultAgrs(md3dImmediateContext.Get());
	md3dImmediateContext->DrawIndexed(mDebugWindow->IndexCount, mDebugWindow->StartIndexLocation, mDebugWindow->BaseVertexLocation);
}
void GameMain::RenderShadow()
{
	md3dImmediateContext->RSSetState(RenderStates::DepthBias.Get());
	md3dImmediateContext->OMSetDepthStencilState(nullptr, 0);

	for (auto &e : GameObjects)
	{
		e->shader = mShaders["LightDepth"].get();
	}

	DrawShadow = true;
	RenderGeometry();
}
void GameMain::RenderOpaque()
{
	md3dImmediateContext->RSSetState(nullptr);
	md3dImmediateContext->OMSetDepthStencilState(nullptr, 0);

	for (auto &e : GameObjects)
	{
		e->shader = mShaders["Color"].get();
	}

	DrawShadow = false;
	RenderGeometry();
	RenderDebugWindow();

}
void GameMain::RenderSkyBox()
{
	md3dImmediateContext->RSSetState(RenderStates::CullOff.Get());
	md3dImmediateContext->OMSetDepthStencilState(RenderStates::DepthLessEqual.Get(), 0);

	ObjectConstant VSConstant;
	XMMATRIX view, proj, world;
	XMVECTOR eye = XMLoadFloat3(&EyePos);
	world = XMMatrixTranslationFromVector(eye);
	view = mCameraView;
	proj = mCameraProj;
	XMMATRIX mvp = world*view * proj;
	XMStoreFloat4x4(&VSConstant.MVP, XMMatrixTranspose(mvp));
	D3D11_MAPPED_SUBRESOURCE mapData;
	HR(md3dImmediateContext->Map(mSkyBox->mObjectCbuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapData));
	memcpy_s(mapData.pData, sizeof(VSConstant), &VSConstant, sizeof(VSConstant));
	md3dImmediateContext->Unmap(mSkyBox->mObjectCbuffer.Get(), 0);

	mSkyBox->SetRenderDefaultAgrs(md3dImmediateContext.Get());
	md3dImmediateContext->DrawIndexed(mSkyBox->IndexCount, mSkyBox->StartIndexLocation, mSkyBox->BaseVertexLocation);
}
void GameMain::DrawScene()
{
	float Black[] = { 0,0,0,0 };
	assert(md3dImmediateContext);
	assert(mSwapChain);

	md3dImmediateContext->OMSetRenderTargets(0, 0, mShadowMap->mDSV.Get());
	md3dImmediateContext->RSSetViewports(1, &mShadowMap->mViewPort);
	md3dImmediateContext->ClearDepthStencilView(mShadowMap->mDSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	RenderShadow();

	md3dImmediateContext->OMSetRenderTargets(1, mRenderTargetView.GetAddressOf(), mDepthStencilView.Get());
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView.Get(),Black);
	md3dImmediateContext->RSSetViewports(1, &mScreenViewport);
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	RenderOpaque();
	RenderSkyBox();

	HR(mSwapChain->Present(0, 0));
}