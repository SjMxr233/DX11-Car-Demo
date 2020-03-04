#include "ConstantBuffer.h"
#include "d3dApp.h"
#include "RenderItem.h"
#include "Camera.h"
#include "DirectionLight.h"
#include "ShadowMap.h"
#include "Gbuffer.h"
#include "Ssao.h"
#include "LightBuffer.h"
using namespace DirectX;
enum CameraMode
{
	First,
	Third 
};
class GameMain : public D3DApp
{
public:
	GameMain(HINSTANCE hInstance);
	~GameMain();
	//主要运行函数
	bool Init()override;
	void UpdateScene(float dt)override;
	void DrawScene()override;

	void OnMouseMove(WPARAM btnState, int x, int y) override;//鼠标移动事件
	void OnMouseWheel(WPARAM btnState) override;//鼠标滚轮事件
	//主角,摄像机,灯光控制逻辑
	void PlayerControl(const float dt);
	void CameraControl(const float dt);
	void LightControl(const float dt);
private:
	void BuildGeometry(); //初始化所用模型数据
	void BuildShaderAndInputLayout(); //初始化所用着色器
	void BuildObjectVsCBuffer(); //创建顶点常量缓存区
	void BuildObjectPsCBuffer(); //创建片元常量缓存区
	void BuildTexture(); //初始化所用贴图数据
	void BuildRenderItem(); //场景中所有物体渲染项设置
	void BuildMaterial(); //初始化所用材质
	void BuildLight(); //初始化灯光属性
	void BuildCamera(); 

	//更新所用到的常量缓存区数据
	void UpdateObjectPsCBuffer(); 
	void UpdateObjectVsCBuffer();
	void UpdateSkyBoxCBuffer();
	void UpdateSsaoCBuffer();
	void UpdateSssmCBuffer();
	void UpdateLightingPassCBuffer();

	void RenderGeometry(); //渲染场景中的物体
	void RenderOpaque(); //渲染不透明项
	void RenderShadow(); //得到光源处深度图
	void RenderSSSM();//得到屏幕空间阴影映射纹理
	void RenderSkyBox(); 
	void RenderDebugWindow(); //渲染调试窗口
	void RenderGbuffer();//渲染Gbuffer
	void RenderSSAO();//渲染环境光遮蔽
	void RenderLightingPass(); 
	void RenderFXAA();
	void RenderPostProcessing(); //渲染所有后处理效果
private:

	//存储所有可能用到的材质、贴图、着色器、几何数据
	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
	std::unordered_map<std::string, std::unique_ptr<Material>> mMaterials;
	std::unordered_map<std::string, std::unique_ptr<Shader>> mShaders;
	std::unordered_map<std::string, std::unique_ptr<Texture>> mTextures;

	//小车参数
	std::shared_ptr<RenderItem> mBody;
	std::shared_ptr<RenderItem> mFrontLeftWheel;
	std::shared_ptr<RenderItem> mFrontRightWheel;
	std::shared_ptr<RenderItem> mBackLeftWheel;
	std::shared_ptr<RenderItem> mBackRightWheel;
	float mWheelRoll, mWheelSteer; //记录车轮滚动速度，车轮转向速度
	float mVelocity = 0.0f;
	float mRotVelocity = 0.0f;
	float mMaxRotVelocity = 40.0f;
	float mMaxVelocity = 50.0f;

	std::shared_ptr<RenderItem> mSkyBox;
	std::vector<std::shared_ptr<RenderItem>> mDebugWindows;
	std::vector<std::shared_ptr<RenderItem>> GameObjects;

	//第一人称和第三人称摄像机参数
	FirstPersonCamera mFirstCamera;
	ThirdPersonCamera mThirdCamera;
	CameraMode cameraMode = Third;//默认第三人称
	XMMATRIX mCameraView;
	XMMATRIX mCameraProj;
	XMFLOAT3 EyePos;

	//阴影效果参数
	std::unique_ptr<ShadowMap> mShadowMap;
	std::shared_ptr<RenderItem> mShadowPostQuad; //延迟渲染SSSM
	bool DrawShadow = false;
	//平行光
	std::unique_ptr<DirectionLight> mDirectionLight;

	//Gbuffer
	std::unique_ptr<Gbuffer> mGbuffer;

	//SSAO
	std::shared_ptr<RenderItem> mAoPostQuad;
	std::unique_ptr<Ssao> mSSAOMap;

	//LightingPass
	std::shared_ptr<RenderItem> mLightingPassPostQuad;
	std::unique_ptr<LightBuffer> mLightBuffer;

	//FXAA
	std::shared_ptr<RenderItem> mFxaaPostQuad;

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
	GeometryGenerator::MeshData ramp;
	//存放子模型,共用同一个顶点缓存区

	std::vector<GeometryGenerator::MeshData> Meshdatas_Player; 
	geoGen.CreateBox(2.0f, 0.6f, 3.0f, box);
	box.name = "box";
	Meshdatas_Player.push_back(box);
	geoGen.CreateCylinder(0.5f, 0.5f, 0.5f, 20, 20, cylinder);
	cylinder.name = "cylinder";
	Meshdatas_Player.push_back(cylinder);
	auto player = std::make_unique<MeshGeometry>();
	player->name = "Player";
	player->SetBuffer(md3dDevice.Get(), Meshdatas_Player);

	std::vector<GeometryGenerator::MeshData> Meshdatas_Ramp;
	geoGen.CreateRamp(ramp, 0.1f*MathHelper::Pi, 8.0f, 10.0f);
	ramp.name = "ramp";
	Meshdatas_Ramp.push_back(ramp);
	auto Ramp = std::make_unique<MeshGeometry>();
	Ramp->name = "Ramp";
	Ramp->SetBuffer(md3dDevice.Get(), Meshdatas_Ramp);

	std::vector<GeometryGenerator::MeshData> Meshdatas_Floor;
	geoGen.CreateGrid(200.0f, 200.0f, 2, 2, grid);
	grid.name = "grid";
	Meshdatas_Floor.push_back(grid);
	auto floor = std::make_unique<MeshGeometry>();
	floor->name = "Floor";
	floor->SetBuffer(md3dDevice.Get(), Meshdatas_Floor);

	std::vector<GeometryGenerator::MeshData> Meshdatas_DebugWindow;
	geoGen.CreateScreenQuad(quad, 0.2, XMFLOAT2(0.8, -0.8));
	quad.name = "quad"; //阴影
	Meshdatas_DebugWindow.push_back(quad);
	geoGen.CreateScreenQuad(quad, 0.2, XMFLOAT2(-0.8, -0.8));
	quad.name = "quad1";//颜色
	Meshdatas_DebugWindow.push_back(quad);
	geoGen.CreateScreenQuad(quad, 0.2, XMFLOAT2(-0.4, -0.8));
	quad.name = "quad2";//法线
	Meshdatas_DebugWindow.push_back(quad);
	geoGen.CreateScreenQuad(quad, 0.2, XMFLOAT2(0, -0.8));
	quad.name = "quad3";//位置
	Meshdatas_DebugWindow.push_back(quad);
	geoGen.CreateScreenQuad(quad, 0.2, XMFLOAT2(0.4, -0.8));
	quad.name = "quad4";//Ao
	Meshdatas_DebugWindow.push_back(quad);
	auto debugwindow = std::make_unique<MeshGeometry>();
	debugwindow->name = "DebugWindow";
	debugwindow->SetBuffer(md3dDevice.Get(), Meshdatas_DebugWindow);
	
	std::vector<GeometryGenerator::MeshData> Meshdatas_SkyBox;
	geoGen.CreateBox(200.0f, 200.0f, 200.0f, box);
	box.name = "box";
	Meshdatas_SkyBox.push_back(box);
	auto skybox = std::make_unique<MeshGeometry>();
	skybox->name = "SkyBox";
	skybox->SetBuffer(md3dDevice.Get(), Meshdatas_SkyBox);
	
	std::vector<GeometryGenerator::MeshData> Meshdatas_PostQuad;
	geoGen.CreateScreenQuad(quad, 1.0f, XMFLOAT2(0, 0));
	quad.name = "quad";
	Meshdatas_PostQuad.push_back(quad);
	auto postquad= std::make_unique<MeshGeometry>();
	postquad->name = "PostQuad";
	postquad->SetBuffer(md3dDevice.Get(), Meshdatas_PostQuad);

	mGeometries[player->name] = std::move(player);
	mGeometries[Ramp->name] = std::move(Ramp);
	mGeometries[floor->name] = std::move(floor);
	mGeometries[debugwindow->name] = std::move(debugwindow);
	mGeometries[skybox->name] = std::move(skybox);
	mGeometries[postquad->name] = std::move(postquad);
}
void GameMain::BuildShaderAndInputLayout()
{
	auto ForwardDefault = std::make_unique<Shader>();
	ForwardDefault->name = "ForwardDefault";
	ForwardDefault->CreatShaderAndLayout(md3dDevice.Get(), L"shader\\ForwardDefault.hlsl");

	auto Sky = std::make_unique<Shader>();
	Sky->name = "Sky";
	Sky->CreatShaderAndLayout(md3dDevice.Get(), L"shader\\SkyBox.hlsl");

	auto LightDepth = std::make_unique<Shader>();
	LightDepth->name = "LightDepth";
	LightDepth->CreatShaderAndLayout(md3dDevice.Get(), L"shader\\LightDepth.hlsl");

	auto DebugSingleRT = std::make_unique<Shader>();
	DebugSingleRT->name = "DebugSingleRT";
	DebugSingleRT->CreatShaderAndLayout(md3dDevice.Get(), L"shader\\DebugSingleRT.hlsl");

	auto DebugRT = std::make_unique<Shader>();
	DebugRT->name = "DebugRT";
	DebugRT->CreatShaderAndLayout(md3dDevice.Get(), L"shader\\DebugRT.hlsl");

	auto DrawGbuffer = std::make_unique<Shader>();
	DrawGbuffer->name = "DrawGbuffer";
	DrawGbuffer->CreatShaderAndLayout(md3dDevice.Get(), L"shader\\DrawGbuffer.hlsl");

	auto DrawSSAO = std::make_unique<Shader>();
	DrawSSAO->name = "DrawSSAO";
	DrawSSAO->CreatShaderAndLayout(md3dDevice.Get(), L"shader\\DrawSSAO.hlsl");

	auto BlurSSAO = std::make_unique<Shader>();
	BlurSSAO->name = "BlurSSAO";
	BlurSSAO->CreatShaderAndLayout(md3dDevice.Get(), L"shader\\BlurSSAO.hlsl");

	auto DrawSSSM = std::make_unique<Shader>();
	DrawSSSM->name = "DrawSSSM";
	DrawSSSM->CreatShaderAndLayout(md3dDevice.Get(), L"shader\\DrawSSSM.hlsl");

	auto LightingPass = std::make_unique<Shader>();
	LightingPass->name = "LightingPass";
	LightingPass->CreatShaderAndLayout(md3dDevice.Get(), L"shader\\DefferredLightPass.hlsl");

	auto FXAA = std::make_unique<Shader>();
	FXAA->name = "FXAA";
	FXAA->CreatShaderAndLayout(md3dDevice.Get(), L"shader\\FXAA.hlsl");

	mShaders[ForwardDefault->name] = std::move(ForwardDefault);
	mShaders[Sky->name] = std::move(Sky);
	mShaders[LightDepth->name] = std::move(LightDepth);
	mShaders[DebugSingleRT->name] = std::move(DebugSingleRT);
	mShaders[DebugRT->name] = std::move(DebugRT);
	mShaders[DrawGbuffer->name] = std::move(DrawGbuffer);
	mShaders[DrawSSAO->name] = std::move(DrawSSAO);
	mShaders[BlurSSAO->name] = std::move(BlurSSAO);
	mShaders[DrawSSSM->name] = std::move(DrawSSSM);
	mShaders[LightingPass->name] = std::move(LightingPass);
	mShaders[FXAA->name] = std::move(FXAA);
}
void GameMain::BuildObjectVsCBuffer()
{
	for (auto &e : GameObjects)
	{
		e->CreatObjectVsCBuffer(md3dDevice.Get(), sizeof(ObjectConstant));
	}
	mSkyBox->CreatObjectVsCBuffer(md3dDevice.Get(), sizeof(SkyBoxConstant));
}
void GameMain::BuildObjectPsCBuffer()
{
	for (auto &e : GameObjects)
	{
		e->CreatObjectPsCBuffer(md3dDevice.Get(), sizeof(ObjectLight));
	}
	mAoPostQuad->CreatObjectPsCBuffer(md3dDevice.Get(), sizeof(SSAOConstant));
	mShadowPostQuad->CreatObjectPsCBuffer(md3dDevice.Get(), sizeof(SSSMConstant));
	mLightingPassPostQuad->CreatObjectPsCBuffer(md3dDevice.Get(), sizeof(ObjectLight));
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
	floor->uvST = XMFLOAT4(6, 6, 0, 0);
	HR(CreateDDSTextureFromFile(md3dDevice.Get(), L"Tex\\floor.dds", nullptr, floor->mTextureView.GetAddressOf()));

	auto sky = std::make_unique<Texture>();
	sky->name = "skybox";
	HR(CreateDDSTextureFromFile(md3dDevice.Get(), L"Tex\\skybox.dds", nullptr, sky->mTextureView.GetAddressOf()));

	mShadowMap = std::make_unique<ShadowMap>(md3dDevice.Get(), 4096, 4096, mClientWidth, mClientWidth);
	auto shadow = std::make_unique<Texture>();
	shadow->name = "shadow";
	shadow->mTextureView = mShadowMap->mSRV;
	auto SSSM = std::make_unique<Texture>();
	SSSM->name = "SSSM";
	SSSM->mTextureView = mShadowMap->mScreenSpaceShadowMapSRV;

	//按后台缓存区大小渲染Gbuffer
	mGbuffer = std::make_unique<Gbuffer>(md3dDevice.Get(), mClientWidth, mClientHeight);
	auto defferredDiffuseMap = std::make_unique<Texture>();
	defferredDiffuseMap->name = "defferredDiffuseMap";
	defferredDiffuseMap->mTextureView = mGbuffer->mSRVs[0];//与写入Gbuffer MRT索引吻合
	auto defferredNormalMap = std::make_unique<Texture>();
	defferredNormalMap->name = "defferredNormalDepthMap";
	defferredNormalMap->mTextureView = mGbuffer->mSRVs[1];
	auto defferredPositionMap= std::make_unique<Texture>();
	defferredPositionMap->name = "defferredPositionMap";
	defferredPositionMap->mTextureView = mGbuffer->mSRVs[2];

	mSSAOMap = std::make_unique<Ssao>(md3dDevice.Get(), mClientWidth, mClientHeight);
	auto ssaoMap = std::make_unique<Texture>();
	ssaoMap->name = "SSAO";
	ssaoMap->mTextureView = mSSAOMap->mAoSRV;
	auto randVecMap = std::make_unique<Texture>();
	randVecMap->name = "RandVectorMap";
	randVecMap->mTextureView = mSSAOMap->mRandSRV;
	auto BlurSSAOMap = std::make_unique<Texture>();
	BlurSSAOMap->name = "BlurSSAOMap";
	BlurSSAOMap->mTextureView = mSSAOMap->mAoBlurSRV;

	mLightBuffer = std::make_unique<LightBuffer>(md3dDevice.Get(), mClientWidth, mClientHeight);
	auto LightBufferMap = std::make_unique<Texture>();
	LightBufferMap->name = "LightBuffer";
	LightBufferMap->mTextureView = mLightBuffer->mSRV;

	mTextures[sky->name] = std::move(sky);
	mTextures[zero->name] = std::move(zero);
	mTextures[wheel->name] = std::move(wheel);
	mTextures[floor->name] = std::move(floor);
	mTextures[shadow->name] = std::move(shadow);
	mTextures[SSSM->name] = std::move(SSSM);
	mTextures[defferredDiffuseMap->name] = std::move(defferredDiffuseMap);
	mTextures[defferredNormalMap->name] = std::move(defferredNormalMap);
	mTextures[defferredPositionMap->name] = std::move(defferredPositionMap);
	mTextures[ssaoMap->name] = std::move(ssaoMap);
	mTextures[BlurSSAOMap->name] = std::move(BlurSSAOMap);
	mTextures[randVecMap->name] = std::move(randVecMap);
	mTextures[LightBufferMap->name] = std::move(LightBufferMap);
}
void GameMain::BuildLight()
{
	XMFLOAT4 LightColor= XMFLOAT4(0.7f, 0.7f, 0.6f, 1.0f);
	XMFLOAT3 LightDirection = XMFLOAT3(-1.0f, 0.7f, -0.3f);
	//根据场景最宽的物体决定包围球大小
	//ps:场景过大,导致阴影质量下降.TODO:CSM
	float SphereRadius = sqrt(100 * 100 + 100 * 100);
	auto directLight = std::make_unique<DirectionLight>(LightColor, LightDirection);
	directLight->SetLightBounding(XMFLOAT3(0, 0, 0), SphereRadius);
	directLight->SetLightViewAndProj();
	mDirectionLight = std::move(directLight);
}
void GameMain::BuildMaterial()
{
	auto zero = std::make_unique<Material>();
	zero->_Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	zero->_Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);

	auto floor = std::make_unique<Material>();
	floor->_Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	floor->_Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);

	mMaterials["zero"] = std::move(zero);
	mMaterials["floor"] = std::move(floor);
}
void GameMain::BuildRenderItem()
{

#pragma region SceneObject

	auto rampObject = std::make_shared<RenderItem>();
	rampObject->transform->Position = XMFLOAT3(0, 0.4f, 10.0f);
	rampObject->mesh = mGeometries["Ramp"].get();
	rampObject->mat = mMaterials["zero"].get();
	rampObject->tex = mTextures["zero"].get();
	rampObject->SetMeshAgrs("ramp");
	GameObjects.push_back(rampObject);

	auto bodyObject = std::make_shared<RenderItem>();
	bodyObject->mesh = mGeometries["Player"].get();
	bodyObject->mat = mMaterials["zero"].get();
	bodyObject->tex = mTextures["zero"].get();
	bodyObject->SetMeshAgrs("box");
	mBody = bodyObject;
	GameObjects.push_back(bodyObject);
	
	auto flwheelObject = std::make_shared<RenderItem>();
	flwheelObject->transform->SetParent(bodyObject->transform);
	flwheelObject->transform->Position = XMFLOAT3(-1.5f, -0.5f, 1.6f);
	XMStoreFloat4x4(&flwheelObject->transform->InitRot,XMMatrixRotationZ(0.5f*MathHelper::Pi));
	flwheelObject->mesh = mGeometries["Player"].get();
	flwheelObject->mat = mMaterials["zero"].get();
	flwheelObject->tex = mTextures["wheel"].get();
	flwheelObject->SetMeshAgrs("cylinder");
	mFrontLeftWheel = flwheelObject;
	GameObjects.push_back(flwheelObject);

	auto frwheelObject = std::make_shared<RenderItem>();
	frwheelObject->transform->SetParent(bodyObject->transform);
	frwheelObject->transform->Position = XMFLOAT3(1.5f, -0.5f, 1.6f);
	XMStoreFloat4x4(&frwheelObject->transform->InitRot, XMMatrixRotationZ(0.5f*MathHelper::Pi));
	frwheelObject->mesh = mGeometries["Player"].get();
	frwheelObject->mat = mMaterials["zero"].get();
	frwheelObject->tex = mTextures["wheel"].get();
	frwheelObject->SetMeshAgrs("cylinder");
	mFrontRightWheel = frwheelObject;
	GameObjects.push_back(frwheelObject);

	auto blwheelObject = std::make_shared<RenderItem>();
	blwheelObject->transform->SetParent(bodyObject->transform);
	blwheelObject->transform->Position = XMFLOAT3(-1.5f, -0.5f, -1.6f);
	XMStoreFloat4x4(&blwheelObject->transform->InitRot, XMMatrixRotationZ(0.5f*MathHelper::Pi));
	blwheelObject->mesh = mGeometries["Player"].get();
	blwheelObject->mat = mMaterials["zero"].get();
	blwheelObject->tex = mTextures["wheel"].get();
	blwheelObject->SetMeshAgrs("cylinder");
	mBackLeftWheel = blwheelObject;
	GameObjects.push_back(blwheelObject);

	auto brwheelObject = std::make_shared<RenderItem>();
	brwheelObject->transform->SetParent(bodyObject->transform);
	brwheelObject->transform->Position = XMFLOAT3(1.5f, -0.5f, -1.6f);
	XMStoreFloat4x4(&brwheelObject->transform->InitRot, XMMatrixRotationZ(0.5f*MathHelper::Pi));
	brwheelObject->mesh = mGeometries["Player"].get();
	brwheelObject->mat = mMaterials["zero"].get();
	brwheelObject->tex = mTextures["wheel"].get();
	brwheelObject->SetMeshAgrs("cylinder");
	mBackRightWheel = brwheelObject;
	GameObjects.push_back(brwheelObject);

	auto gridObject = std::make_shared<RenderItem>();
	gridObject->transform->Position = XMFLOAT3(0, -1.0f, 0);
	gridObject->mesh = mGeometries["Floor"].get();
	gridObject->mat = mMaterials["floor"].get();
	gridObject->tex = mTextures["floor"].get();
	gridObject->SetMeshAgrs("grid");
	GameObjects.push_back(gridObject);

	auto SkyObject = std::make_shared<RenderItem>();
	SkyObject->mesh = mGeometries["SkyBox"].get();
	SkyObject->tex = mTextures["skybox"].get();
	SkyObject->shader = mShaders["Sky"].get();
	SkyObject->SetMeshAgrs("box");
	mSkyBox = SkyObject;

	for (auto &e : GameObjects)
	{
		e->transform->Update();
	}
#pragma endregion

#pragma region PostProcessingQuad
	//全屏quad进行后处理效果
	auto AoQuadObject = std::make_shared<RenderItem>();
	AoQuadObject->mesh = mGeometries["PostQuad"].get();
	AoQuadObject->tex = mTextures["defferredPositionMap"].get();
	AoQuadObject->shader = mShaders["DrawSSAO"].get();
	AoQuadObject->SetMeshAgrs("quad");
	mAoPostQuad = AoQuadObject;

	auto ShadowQuadObject = std::make_shared<RenderItem>();
	ShadowQuadObject->mesh = mGeometries["PostQuad"].get();
	ShadowQuadObject->tex = mTextures["defferredPositionMap"].get();
	ShadowQuadObject->shader = mShaders["DrawSSSM"].get();
	ShadowQuadObject->SetMeshAgrs("quad");
	mShadowPostQuad = ShadowQuadObject;

	auto LightingPassQuadObject = std::make_shared<RenderItem>();
	LightingPassQuadObject->mesh = mGeometries["PostQuad"].get();
	LightingPassQuadObject->tex = mTextures["defferredPositionMap"].get();
	LightingPassQuadObject->shader = mShaders["LightingPass"].get();
	LightingPassQuadObject->SetMeshAgrs("quad");
	mLightingPassPostQuad = LightingPassQuadObject;

	auto FxaaQuadObject = std::make_shared<RenderItem>();
	FxaaQuadObject->mesh = mGeometries["PostQuad"].get();
	FxaaQuadObject->tex = mTextures["LightBuffer"].get();
	FxaaQuadObject->shader = mShaders["FXAA"].get();
	FxaaQuadObject->SetMeshAgrs("quad");
	mFxaaPostQuad = FxaaQuadObject;
#pragma endregion

#pragma region DebugRenderItem
	auto DebugShadow = std::make_shared<RenderItem>();
	DebugShadow->mesh = mGeometries["DebugWindow"].get();
	DebugShadow->tex = mTextures["SSSM"].get();
	DebugShadow->shader = mShaders["DebugSingleRT"].get();
	DebugShadow->SetMeshAgrs("quad");
	mDebugWindows.push_back(DebugShadow);

	auto DebugDiffuse= std::make_shared<RenderItem>();
	DebugDiffuse->mesh = mGeometries["DebugWindow"].get();
	DebugDiffuse->tex = mTextures["defferredDiffuseMap"].get();
	DebugDiffuse->shader = mShaders["DebugRT"].get();
	DebugDiffuse->SetMeshAgrs("quad1");
	mDebugWindows.push_back(DebugDiffuse);

	auto DebugNormal = std::make_shared<RenderItem>();
	DebugNormal->mesh = mGeometries["DebugWindow"].get();
	DebugNormal->tex = mTextures["defferredNormalDepthMap"].get();
	DebugNormal->shader = mShaders["DebugRT"].get();
	DebugNormal->SetMeshAgrs("quad2");
	mDebugWindows.push_back(DebugNormal);

	auto DebugPosition = std::make_shared<RenderItem>();
	DebugPosition->mesh = mGeometries["DebugWindow"].get();
	DebugPosition->tex = mTextures["defferredPositionMap"].get();
	DebugPosition->shader = mShaders["DebugRT"].get();
	DebugPosition->SetMeshAgrs("quad3");
	mDebugWindows.push_back(DebugPosition);

	auto DebugSSAO = std::make_shared<RenderItem>();
	DebugSSAO->mesh = mGeometries["DebugWindow"].get();
	DebugSSAO->tex = mTextures["BlurSSAOMap"].get();
	DebugSSAO->shader = mShaders["DebugSingleRT"].get();
	DebugSSAO->SetMeshAgrs("quad4");
	mDebugWindows.push_back(DebugSSAO);
#pragma endregion

	BuildObjectVsCBuffer();
	BuildObjectPsCBuffer();
}
void GameMain::BuildCamera()
{
	mFirstCamera.SetLens(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	mThirdCamera.SetLens(0.1f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
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

	//鼠标在窗口循环
	POINT ScreenMousePos;
	GetCursorPos(&ScreenMousePos);
	RECT rect;
	GetWindowRect(mhMainWnd, &rect);
	if (x > mClientWidth-5 && dx>0)
	{
		SetCursorPos(rect.left, ScreenMousePos.y);
		mLastMousePos.x = 0;
		mLastMousePos.y = y;
	}
	else if (x < 5 && dx<0)
	{
		SetCursorPos(rect.right, ScreenMousePos.y);
		mLastMousePos.x = mClientWidth;
		mLastMousePos.y = y;
	}
	else if (y > mClientHeight - 5 && dy > 0)
	{
		SetCursorPos(ScreenMousePos.x, rect.top);
		mLastMousePos.x = x;
		mLastMousePos.y = 0;
	}
	else if (y < 5 && dy < 0)
	{
		SetCursorPos(ScreenMousePos.x, rect.bottom);
		mLastMousePos.x = x;
		mLastMousePos.y = mClientHeight;
	}
	else
	{
		mLastMousePos.x = x;
		mLastMousePos.y = y;
	}
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
	XMVECTOR Direction = XMLoadFloat3(&mDirectionLight->Direction);
	Direction = XMVector3TransformNormal(Direction, XMMatrixRotationY(0.8f*dt));
	XMStoreFloat3(&mDirectionLight->Direction, Direction);
	mDirectionLight->SetLightViewAndProj();
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
		XMMATRIX w = XMLoadFloat4x4(&mBody->transform->World)*XMMatrixTranslation(0,3.0f,0);
		XMVECTOR target = XMVector3TransformCoord(XMVectorZero(), w);
		mFirstCamera.SetPosition(target);
		mFirstCamera.UpdateViewMatrix();
		mCameraView = mFirstCamera.GetView();
		mCameraProj = mFirstCamera.GetProj();
		EyePos = mFirstCamera.GetPosition();
	}
	if (cameraMode == Third)
	{
		XMMATRIX w = XMLoadFloat4x4(&mBody->transform->World);
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
	XMVECTOR position = XMLoadFloat3(&mBody->transform->Position);
	XMVECTOR forward = XMLoadFloat3(&mBody->transform->Forward);
	mWheelRoll = 1.5f*mVelocity;
	mWheelSteer = 2.0f;
	if (GetAsyncKeyState('W') & 0x8000)
	{
		//线性插值曲线提速
		mVelocity = MathHelper::Lerp(mVelocity, mMaxVelocity, 0.3f*dt);
		mRotVelocity = MathHelper::Lerp(mRotVelocity, mMaxRotVelocity, 0.3f*dt);
		//简单的车身绕轴旋转
		if (GetAsyncKeyState('A') & 0x8000)
			mBody->transform->Rotation.y -= 0.06f*mRotVelocity *dt;
		if (GetAsyncKeyState('D') & 0x8000)
			mBody->transform->Rotation.y += 0.06f* mRotVelocity *dt;
	}
	if (GetAsyncKeyState('S') & 0x8000)
	{
		mVelocity = MathHelper::Lerp(mVelocity, -mMaxVelocity, 0.3f*dt);
		mRotVelocity = MathHelper::Lerp(mRotVelocity, -mMaxRotVelocity, 0.3f*dt);
		if (GetAsyncKeyState('A') & 0x8000)
			mBody->transform->Rotation.y -= 0.06f*mRotVelocity * dt;
		if (GetAsyncKeyState('D') & 0x8000)
			mBody->transform->Rotation.y += 0.06f*mRotVelocity * dt;
	}
	if (GetAsyncKeyState('A') & 0x8000)
	{
		//车轮旋转
		mFrontLeftWheel->transform->Rotation.y -= mWheelSteer *dt;
		mFrontRightWheel->transform->Rotation.y -= mWheelSteer * dt;
		mFrontLeftWheel->transform->Rotation.y = MathHelper::Clamp(mFrontLeftWheel->transform->Rotation.y, -0.2f*MathHelper::Pi, 0.0f);
		mFrontRightWheel->transform->Rotation.y = MathHelper::Clamp(mFrontRightWheel->transform->Rotation.y, -0.2f*MathHelper::Pi, 0.0f);
	}
	else if (GetAsyncKeyState('D') & 0x8000)
	{
		mFrontLeftWheel->transform->Rotation.y += mWheelSteer * dt;
		mFrontRightWheel->transform->Rotation.y += mWheelSteer * dt;
		mFrontLeftWheel->transform->Rotation.y = MathHelper::Clamp(mFrontLeftWheel->transform->Rotation.y, 0.0f, 0.2f*MathHelper::Pi);
		mFrontRightWheel->transform->Rotation.y = MathHelper::Clamp(mFrontRightWheel->transform->Rotation.y, 0.0f, 0.2f*MathHelper::Pi);
	}
	else
	{
		mFrontLeftWheel->transform->Rotation.y = MathHelper::Lerp(mFrontLeftWheel->transform->Rotation.y, 0.0f, 2.5f*dt);
		mFrontRightWheel->transform->Rotation.y = MathHelper::Lerp(mFrontRightWheel->transform->Rotation.y, 0.0f, 2.5f*dt);
	}
	position += forward * mVelocity *dt;
	//车轮滚动
	mFrontLeftWheel->transform->Rotation.x += mWheelRoll * dt;
	mFrontRightWheel->transform->Rotation.x += mWheelRoll * dt;
	mBackLeftWheel->transform->Rotation.x += mWheelRoll * dt;
	mBackRightWheel->transform->Rotation.x += mWheelRoll * dt;

	mVelocity = MathHelper::Lerp(mVelocity, 0.0f, 0.8f*dt);
	mRotVelocity = MathHelper::Lerp(mRotVelocity, 0.0f, 0.8f*dt);
	XMStoreFloat3(&mBody->transform->Position, position);

	mBody->transform->Update();
	mFrontLeftWheel->transform->Update();
	mFrontRightWheel->transform->Update();
	mBackLeftWheel->transform->Update();
	mBackRightWheel->transform->Update();
}
void GameMain::UpdateObjectPsCBuffer()
{
	D3D11_MAPPED_SUBRESOURCE mapData;
	for (auto &e : GameObjects)
	{
		ObjectLight LightData;
		LightData.gDirLight.Direction = mDirectionLight->Direction;
		LightData.gDirLight.LightColor = mDirectionLight->Color;
		LightData.gMaterial._Diffuse = e->mat->_Diffuse;
		LightData.gMaterial._Specular = e->mat->_Specular;
		LightData.gEyePos = EyePos;
		HR(md3dImmediateContext->Map(e->mPsCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapData));
		memcpy_s(mapData.pData, sizeof(LightData), &LightData, sizeof(LightData));
		md3dImmediateContext->Unmap(e->mPsCB.Get(), 0);
	}
}
void GameMain::UpdateObjectVsCBuffer()
{
	D3D11_MAPPED_SUBRESOURCE mapData;
	mShadowMap->UpdateShadowTransform(mDirectionLight->GetLightView(),mDirectionLight->GetLightProj());
	for (auto &e : GameObjects)
	{
		ObjectConstant ObjectData;
		XMMATRIX world = XMLoadFloat4x4(&e->transform->World);
		XMMATRIX view, proj;
		if (DrawShadow)
		{
			view = mDirectionLight->GetLightView();
			proj = mDirectionLight->GetLightProj();
		}
		else
		{
			view = mCameraView;
			proj = mCameraProj;
		}
		XMMATRIX S = XMLoadFloat4x4(&mShadowMap->mShadowTransform);
		ObjectData.uvST = e->tex->uvST;
		XMStoreFloat4x4(&ObjectData.gModel, XMMatrixTranspose(world));
		XMStoreFloat4x4(&ObjectData.gView, XMMatrixTranspose(view));
		XMStoreFloat4x4(&ObjectData.gProj, XMMatrixTranspose(proj));
		XMStoreFloat4x4(&ObjectData.gModelInvTranspose, XMMatrixInverse(nullptr, world));
		XMStoreFloat4x4(&ObjectData.gShadowTransform, XMMatrixTranspose(S));
		HR(md3dImmediateContext->Map(e->mVsCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapData));
		memcpy_s(mapData.pData, sizeof(ObjectData), &ObjectData, sizeof(ObjectData));
		md3dImmediateContext->Unmap(e->mVsCB.Get(), 0);
	}
}
void GameMain::UpdateSkyBoxCBuffer()
{
	SkyBoxConstant VSConstant;
	XMMATRIX view, proj, world;
	//天空盒跟随相机
	XMVECTOR eye = XMLoadFloat3(&EyePos);
	world = XMMatrixTranslationFromVector(eye);
	view = mCameraView;
	proj = mCameraProj;
	XMStoreFloat4x4(&VSConstant.gModel, XMMatrixTranspose(world));
	XMStoreFloat4x4(&VSConstant.gView, XMMatrixTranspose(view));
	XMStoreFloat4x4(&VSConstant.gProj, XMMatrixTranspose(proj));
	D3D11_MAPPED_SUBRESOURCE mapData;
	HR(md3dImmediateContext->Map(mSkyBox->mVsCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapData));
	memcpy_s(mapData.pData, sizeof(VSConstant), &VSConstant, sizeof(VSConstant));
	md3dImmediateContext->Unmap(mSkyBox->mVsCB.Get(), 0);
}
void GameMain::UpdateSsaoCBuffer()
{
	SSAOConstant PSConstant;
	XMMATRIX view, proj;
	view = mCameraView;
	proj = mCameraProj;	
	XMStoreFloat4x4(&PSConstant.gView, XMMatrixTranspose(view));
	XMStoreFloat4x4(&PSConstant.gProj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&PSConstant.gViewInvTranspose, XMMatrixInverse(nullptr, view));
	for (int i = 0; i < 64; i++)
	{
		PSConstant.gOffset[i]=mSSAOMap->mOffsets[i];
	}
	D3D11_MAPPED_SUBRESOURCE mapData;
	HR(md3dImmediateContext->Map(mAoPostQuad->mPsCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapData));
	memcpy_s(mapData.pData, sizeof(PSConstant), &PSConstant, sizeof(PSConstant));
	md3dImmediateContext->Unmap(mAoPostQuad->mPsCB.Get(), 0);
}
void GameMain::UpdateSssmCBuffer()
{
	SSSMConstant PsConstant;
	mShadowMap->UpdateShadowTransform(mDirectionLight->GetLightView(), mDirectionLight->GetLightProj());
	XMMATRIX S = XMLoadFloat4x4(&mShadowMap->mShadowTransform);
	XMStoreFloat4x4(&PsConstant.gShadowTransform, XMMatrixTranspose(S));
	D3D11_MAPPED_SUBRESOURCE mapData;
	HR(md3dImmediateContext->Map(mShadowPostQuad->mPsCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapData));
	memcpy_s(mapData.pData, sizeof(PsConstant), &PsConstant, sizeof(PsConstant));
	md3dImmediateContext->Unmap(mShadowPostQuad->mPsCB.Get(), 0);
}
void GameMain::UpdateLightingPassCBuffer()
{
	ObjectLight LightData;
	LightData.gDirLight.Direction = mDirectionLight->Direction;
	LightData.gDirLight.LightColor = mDirectionLight->Color;
	LightData.gMaterial._Diffuse = mMaterials["zero"]->_Diffuse;
	LightData.gMaterial._Specular = mMaterials["zero"]->_Specular;
	LightData.gEyePos = EyePos;
	D3D11_MAPPED_SUBRESOURCE mapData;
	HR(md3dImmediateContext->Map(mLightingPassPostQuad->mPsCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapData));
	memcpy_s(mapData.pData, sizeof(LightData), &LightData, sizeof(LightData));
	md3dImmediateContext->Unmap(mLightingPassPostQuad->mPsCB.Get(), 0);
}
void GameMain::RenderGeometry()
{
	UpdateObjectVsCBuffer();
	UpdateObjectPsCBuffer();
	for (auto &e : GameObjects)
	{
		e->SetRenderDefaultAgrs(md3dImmediateContext.Get());
		md3dImmediateContext->PSSetShaderResources(1, 1, mTextures["shadow"]->mTextureView.GetAddressOf());
		md3dImmediateContext->PSSetSamplers(1, 1, RenderStates::ComparisonState.GetAddressOf());
		md3dImmediateContext->DrawIndexed(e->IndexCount, e->StartIndexLocation, e->BaseVertexLocation);
	}
}
void GameMain::RenderDebugWindow()
{
	for (auto &e : mDebugWindows)
	{
		e->SetRenderDefaultAgrs(md3dImmediateContext.Get());
		md3dImmediateContext->DrawIndexed(e->IndexCount, e->StartIndexLocation, e->BaseVertexLocation);
	}

}
void GameMain::RenderGbuffer()
{
	md3dImmediateContext->RSSetViewports(1, &mGbuffer->mViewPort);
	md3dImmediateContext->RSSetState(nullptr);
	md3dImmediateContext->OMSetDepthStencilState(nullptr, 0);
	md3dImmediateContext->OMSetRenderTargets(BUFFER_COUNT, mGbuffer->mRTVs[0].GetAddressOf(), mGbuffer->mDSV.Get());
	md3dImmediateContext->ClearDepthStencilView(mGbuffer->mDSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	if (cameraMode == First)
		mGbuffer->ClearRenderTargets(md3dImmediateContext.Get(), mFirstCamera.GetFarZ());
	if (cameraMode == Third)
		mGbuffer->ClearRenderTargets(md3dImmediateContext.Get(), mThirdCamera.GetFarZ());

	for (auto &e : GameObjects)
	{
		e->shader = mShaders["DrawGbuffer"].get();
	}
	RenderGeometry();
}
void GameMain::RenderSSAO()
{
	float Black[] = { 0,0,0,1 };
	md3dImmediateContext->RSSetViewports(1, &mSSAOMap->mViewPort);
	md3dImmediateContext->RSSetState(nullptr);

	UpdateSsaoCBuffer();

	md3dImmediateContext->OMSetRenderTargets(1, mSSAOMap->mAoRTV.GetAddressOf(),0);
	md3dImmediateContext->ClearRenderTargetView(mSSAOMap->mAoRTV.Get(), Black);
	mAoPostQuad->shader = mShaders["DrawSSAO"].get();
	mAoPostQuad->SetRenderDefaultAgrs(md3dImmediateContext.Get());
	md3dImmediateContext->PSSetSamplers(0, 1, RenderStates::LinearClamp.GetAddressOf());
	md3dImmediateContext->PSSetSamplers(1, 1, RenderStates::LinearWrap.GetAddressOf());
	md3dImmediateContext->PSSetShaderResources(1, 1, mTextures["defferredNormalDepthMap"]->mTextureView.GetAddressOf());
	md3dImmediateContext->PSSetShaderResources(2, 1, mTextures["RandVectorMap"]->mTextureView.GetAddressOf());
	md3dImmediateContext->DrawIndexed(mAoPostQuad->IndexCount, mAoPostQuad->StartIndexLocation, mAoPostQuad->BaseVertexLocation);

	md3dImmediateContext->OMSetRenderTargets(1, mSSAOMap->mAoBlurRTV.GetAddressOf(),0);
	md3dImmediateContext->ClearRenderTargetView(mSSAOMap->mAoBlurRTV.Get(), Black);
	mAoPostQuad->shader = mShaders["BlurSSAO"].get();
	mAoPostQuad->SetRenderDefaultAgrs(md3dImmediateContext.Get());
	md3dImmediateContext->PSSetSamplers(0, 1, RenderStates::LinearClamp.GetAddressOf());
	md3dImmediateContext->PSSetShaderResources(1, 1, mTextures["defferredNormalDepthMap"]->mTextureView.GetAddressOf());
	md3dImmediateContext->PSSetShaderResources(0, 1, mTextures["SSAO"]->mTextureView.GetAddressOf());
	md3dImmediateContext->DrawIndexed(mAoPostQuad->IndexCount, mAoPostQuad->StartIndexLocation, mAoPostQuad->BaseVertexLocation);


}
void GameMain::RenderShadow()
{
	//Reverse-Z 反转深度缓存得到更高精度
	md3dImmediateContext->RSSetState(RenderStates::DepthBias.Get());
	md3dImmediateContext->OMSetDepthStencilState(RenderStates::DepthGreaterEqual.Get(), 0);

	md3dImmediateContext->OMSetRenderTargets(0, 0, mShadowMap->mDSV.Get());
	md3dImmediateContext->RSSetViewports(1, &mShadowMap->mShadowMapViewPort);
	md3dImmediateContext->ClearDepthStencilView(mShadowMap->mDSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.0f, 0);
	for (auto &e : GameObjects)
	{
		e->shader = mShaders["LightDepth"].get();
	}

	DrawShadow = true;
	RenderGeometry();
	RenderSSSM();
	DrawShadow = false;
}
void GameMain::RenderSSSM()
{
	float Black[] = { 0,0,0,1 };
	md3dImmediateContext->RSSetState(nullptr);

	UpdateSssmCBuffer();

	md3dImmediateContext->OMSetRenderTargets(1, mShadowMap->mScreenSpaceShadowMapRTV.GetAddressOf(), 0);
	md3dImmediateContext->RSSetViewports(1, &mShadowMap->mScreenSpaceShadowMapViewPort);
	md3dImmediateContext->ClearRenderTargetView(mShadowMap->mScreenSpaceShadowMapRTV.Get(), Black);
	mShadowPostQuad->SetRenderDefaultAgrs(md3dImmediateContext.Get());
	md3dImmediateContext->PSSetSamplers(0, 1, RenderStates::LinearClamp.GetAddressOf());
	md3dImmediateContext->PSSetSamplers(1, 1, RenderStates::ComparisonState.GetAddressOf());
	md3dImmediateContext->PSSetShaderResources(1, 1, mTextures["shadow"]->mTextureView.GetAddressOf());
	md3dImmediateContext->DrawIndexed(mShadowPostQuad->IndexCount, mShadowPostQuad->StartIndexLocation, mShadowPostQuad->BaseVertexLocation);

}
void GameMain::RenderLightingPass()
{
	float Black[] = { 0,0,0,1 };
	md3dImmediateContext->RSSetState(nullptr);
	md3dImmediateContext->OMSetRenderTargets(1, mLightBuffer->mRTV.GetAddressOf(), 0);
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView.Get(), Black);
	md3dImmediateContext->RSSetViewports(1, &mLightBuffer->mViewPort);

	UpdateLightingPassCBuffer();

	mLightingPassPostQuad->SetRenderDefaultAgrs(md3dImmediateContext.Get());
	md3dImmediateContext->PSSetSamplers(0, 1, RenderStates::LinearClamp.GetAddressOf());
	md3dImmediateContext->PSSetShaderResources(1, 1, mTextures["defferredNormalDepthMap"]->mTextureView.GetAddressOf());
	md3dImmediateContext->PSSetShaderResources(2, 1, mTextures["BlurSSAOMap"]->mTextureView.GetAddressOf());
	md3dImmediateContext->PSSetShaderResources(3, 1, mTextures["SSSM"]->mTextureView.GetAddressOf());
	md3dImmediateContext->PSSetShaderResources(4, 1, mTextures["defferredDiffuseMap"]->mTextureView.GetAddressOf());
	md3dImmediateContext->DrawIndexed(mLightingPassPostQuad->IndexCount, mLightingPassPostQuad->StartIndexLocation, mLightingPassPostQuad->BaseVertexLocation);
}
void GameMain::RenderFXAA()
{
	float Black[] = { 0,0,0,1 };
	md3dImmediateContext->RSSetState(nullptr);
	md3dImmediateContext->OMSetDepthStencilState(RenderStates::DisableDepthBuffer.Get(), 0);
	md3dImmediateContext->OMSetRenderTargets(1, mRenderTargetView.GetAddressOf(), mDepthStencilView.Get());
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView.Get(), Black);
	md3dImmediateContext->RSSetViewports(1, &mScreenViewport);
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	mFxaaPostQuad->SetRenderDefaultAgrs(md3dImmediateContext.Get());
	md3dImmediateContext->PSSetSamplers(0, 1, RenderStates::LinearClamp.GetAddressOf());
	md3dImmediateContext->DrawIndexed(mFxaaPostQuad->IndexCount, mFxaaPostQuad->StartIndexLocation, mFxaaPostQuad->BaseVertexLocation);
}
void GameMain::RenderPostProcessing()
{
	RenderFXAA();
}
void GameMain::RenderOpaque()
{

	/*前向渲染
		float Black[] = { 0,0,0,1 };
		md3dImmediateContext->RSSetState(nullptr);
		md3dImmediateContext->OMSetDepthStencilState(nullptr, 0);
		md3dImmediateContext->OMSetRenderTargets(1, mRenderTargetView.GetAddressOf(), mDepthStencilView.Get());
		md3dImmediateContext->ClearRenderTargetView(mRenderTargetView.Get(), Black);
		md3dImmediateContext->RSSetViewports(1, &mScreenViewport);
		md3dImmediateContext->ClearDepthStencilView(mDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		for (auto &e : GameObjects)
		{
			e->shader = mShaders["ForwardDefault"].get();
		}
		RenderGeometry();
	*/
	RenderLightingPass();

}
void GameMain::RenderSkyBox()
{
	md3dImmediateContext->RSSetState(RenderStates::CullOff.Get());
	md3dImmediateContext->OMSetDepthStencilState(RenderStates::DepthLessEqual.Get(), 0);
	md3dImmediateContext->OMSetRenderTargets(1, mLightBuffer->mRTV.GetAddressOf(), mGbuffer->mDSV.Get());//使用渲染Gbuffer时的zbuffer

	UpdateSkyBoxCBuffer();

	mSkyBox->SetRenderDefaultAgrs(md3dImmediateContext.Get());
	md3dImmediateContext->DrawIndexed(mSkyBox->IndexCount, mSkyBox->StartIndexLocation, mSkyBox->BaseVertexLocation);
}
bool GameMain::Init()
{
	if (!D3DApp::Init())
		return false;
	RenderStates::Init(md3dDevice.Get());
	BuildTexture();
	BuildShaderAndInputLayout();
	BuildGeometry();
	BuildLight();
	BuildMaterial();
	BuildRenderItem();
	BuildCamera();

	return true;
}
void GameMain::UpdateScene(float dt)
{
	
	PlayerControl(dt);
	CameraControl(dt);
	//LightControl(dt);
}
void GameMain::DrawScene()
{

	float Black[] = { 0,0,0,1 };
	assert(md3dImmediateContext);
	assert(mSwapChain);

	//前向管线顺序:不透->天空盒(在不透后防止overdraw)->半透->后处理

	//延迟管线顺序:提前渲染所需几何信息,组合AO SSSM在lightingPass中加上光照,屏幕空间渲染场景不透明物体->天空盒->半透明->后处理
	RenderGbuffer();
	RenderSSAO();
	RenderShadow();
	RenderOpaque();

	//渲染Gbuffer时开启zbuffer,利用此zbuffer渲染天空盒
	RenderSkyBox();

	/*如需渲染半透明物体,前向渲染半透明,暂无需求
	RenderTransParent();
	*/

	//其余后处理效果
	RenderPostProcessing();

	//中间buffer调试窗口
	RenderDebugWindow();

	HR(mSwapChain->Present(0, 0));
}