#pragma once

#include "d3dUtil.h"
class RenderStates
{
public:
	static void Init(ID3D11Device* device);
	static void Destroy();

	static ID3D11RasterizerState	*WireFrameRS;
};