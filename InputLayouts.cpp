#include "InputLayouts.h"
#include "Effects.h"
void InputLayouts::Init(ID3D11Device* device)
{
	D3DX11_PASS_DESC passDesc;
	/*
	D3D11_INPUT_ELEMENT_DESC PosNormalTexTanDesc[4] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	Effects::DisplacementMapFX->BasicTech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(device->CreateInputLayout(PosNormalTexTanDesc, 4, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &PosNormalTexTan));
		*/

	D3D11_INPUT_ELEMENT_DESC PosColorDesc[2] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	Effects::ColorFX->BasicTech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(device->CreateInputLayout(PosColorDesc, 2, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &PosColor));


	D3D11_INPUT_ELEMENT_DESC PosNormalTex0Tex1TanDesc[5] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	Effects::DisplacementMapFX->BasicTech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(device->CreateInputLayout(PosNormalTex0Tex1TanDesc, 5, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &PosNormalTex0Tex1Tan));




}

void InputLayouts::Destroy()
{
	ReleaseCOM(PosNormalTexTan);
	ReleaseCOM(PosColor);
	ReleaseCOM(PosNormalTex0Tex1Tan);
}

ID3D11InputLayout *InputLayouts::PosColor = 0;
ID3D11InputLayout *InputLayouts::PosNormalTexTan = 0;
ID3D11InputLayout *InputLayouts::PosNormalTex0Tex1Tan = 0;