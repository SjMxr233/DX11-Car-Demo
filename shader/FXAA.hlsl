//FXAA3.11 Ëã·¨ ³­×ÔNVIDA D3Dsample
#define FXAA_PC 1
#define FXAA_HLSL_5 1
#define FXAA_QUALITY__PRESET 39
#define FXAA_GREEN_AS_LUMA 1
#include "FXAAhelper.hlsl"
Texture2D gTex:register(t0);
SamplerState gSampLinearClamp:register(s0);
struct a2v
{
	float3 vertex :POSITION;
	float2 uv:TEXCOORD;
};
struct v2f
{
	float4 pos    : SV_POSITION;
	float2 uv:TEXCOORD;
};
float2 texelSize(Texture2D tex)
{
	uint Width, Height;
	tex.GetDimensions(Width, Height);
	return 1.0f / float2(Width, Height);
}
v2f vert(a2v v)
{
	v2f o;
	o.pos = float4(v.vertex, 1.0f);
	o.uv = v.uv;
	return o;
}
float4 frag(v2f i) :SV_Target
{
	float2 rcpFrame = texelSize(gTex);
	FxaaTex InputFXAATex = { gSampLinearClamp, gTex };
	//return gTex.Sample(gSampLinearClamp, i.uv);
	return FxaaPixelShader(
		i.uv,							// FxaaFloat2 pos,
		FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f),		// FxaaFloat4 fxaaConsolePosPos,
		InputFXAATex,							// FxaaTex tex,
		InputFXAATex,							// FxaaTex fxaaConsole360TexExpBiasNegOne,
		InputFXAATex,							// FxaaTex fxaaConsole360TexExpBiasNegTwo,
		rcpFrame,							// FxaaFloat2 fxaaQualityRcpFrame,
		FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f),		// FxaaFloat4 fxaaConsoleRcpFrameOpt,
		FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f),		// FxaaFloat4 fxaaConsoleRcpFrameOpt2,
		FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f),		// FxaaFloat4 fxaaConsole360RcpFrameOpt2,
		0.75f,									// FxaaFloat fxaaQualitySubpix,
		0.166f,									// FxaaFloat fxaaQualityEdgeThreshold,
		0.0833f,								// FxaaFloat fxaaQualityEdgeThresholdMin,
		0.0f,									// FxaaFloat fxaaConsoleEdgeSharpness,
		0.0f,									// FxaaFloat fxaaConsoleEdgeThreshold,
		0.0f,									// FxaaFloat fxaaConsoleEdgeThresholdMin,
		FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f)		// FxaaFloat fxaaConsole360ConstDir,
	);
}