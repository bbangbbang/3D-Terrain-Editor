

#include "LightHelper.fx"

cbuffer cbPerFrame
{
	DirectionalLight gDirLight;
	float3 gEyePosW;
};

cbuffer cbPerObject
{
	float4x4 gWorld;
	float4x4 gWorldViewProj;
	Material gMaterial;
};

struct VertexIn
{
	float3 PosL  : POSITION;
	float3 NormalL: NORMAL;
	float2 Tex : TEXCOORD0;
	float2 TileTex : TEXCOORD1;
	float3 Tan : TANGENT;
	
};

struct VertexOut
{
	float4 PosH  : SV_POSITION;
	float3 PosW  : POSITION;
	float3 NormalW : NORMAL;
	float2 Tex : TEXCOORD0;
	float2 TileTex : TEXCOORD1;
	
};

SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;

	AddressU = WRAP;
	AddressV = WRAP;
};

Texture2DArray gLayerMapArray;
Texture2D gBlendMap;
Texture2D gNormalMap;

VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	vout.PosW = mul(float4(vin.PosL, 1.0f), gWorld);
	vout.NormalW = mul(vin.NormalL,(float3x3)gWorld);
	vout.Tex = vin.Tex;
	vout.TileTex = vin.TileTex;

	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{

	pin.NormalW = normalize(pin.NormalW);

	float3 toEyeW = normalize(gEyePosW - pin.PosW);

	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float4 A, D, S;

	ComputeDirectionalLight(gMaterial, gDirLight, pin.NormalW, toEyeW, A, D, S);
	ambient += A;
	diffuse += D;
	spec += S;


	float4 c0 = gLayerMapArray.Sample(samLinear, float3(pin.Tex*16, 0.0f));
	float4 c1 = gLayerMapArray.Sample(samLinear, float3(pin.TileTex, 1.0f));
	float4 c2 = gLayerMapArray.Sample(samLinear, float3(pin.TileTex, 2.0f));
	float4 c3 = gLayerMapArray.Sample(samLinear, float3(pin.TileTex, 3.0f));
	float4 c4 = gLayerMapArray.Sample(samLinear, float3(pin.TileTex, 4.0f));
	float4 t = gBlendMap.Sample(samLinear, pin.Tex);

	float4 texColor = c0;
	texColor = lerp(texColor, c1, t.r);
	texColor = lerp(texColor, c2, t.g);
	texColor = lerp(texColor, c3, t.b);
	texColor = lerp(texColor, c4, t.a);

	float4 litColor = texColor * (ambient + diffuse) + spec;
	return litColor;
}

technique11 BasicTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}
