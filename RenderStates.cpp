

#include "RenderStates.h"

ID3D11RasterizerState* RenderStates::WireFrameRS = 0;

void RenderStates::Init(ID3D11Device* device)
{
	D3D11_RASTERIZER_DESC wireDesc;
	ZeroMemory(&wireDesc, sizeof(D3D11_RASTERIZER_DESC));
	wireDesc.CullMode = D3D11_CULL_BACK;
	wireDesc.DepthClipEnable = true;
	wireDesc.FillMode = D3D11_FILL_WIREFRAME;
	wireDesc.FrontCounterClockwise = false;
	
	HR(device->CreateRasterizerState(&wireDesc, &WireFrameRS));




	
}

void RenderStates::Destroy()
{
	ReleaseCOM(WireFrameRS);
}