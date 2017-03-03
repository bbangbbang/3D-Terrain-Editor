#pragma once
#include "d3dUtil.h"

namespace Vertex
{
	struct PosNorTexTan{

		XMFLOAT3 Position;
		XMFLOAT3 Normal;
		XMFLOAT2 Tex;
		XMFLOAT3 Tan;

		PosNorTexTan() {}

		PosNorTexTan(XMFLOAT3 pos, XMFLOAT3 nor, XMFLOAT3 tan, XMFLOAT2 tex)
			: Position(pos), Normal(nor), Tan(tan), Tex(tex)
		{}

	};
	struct PosNorTex0Tex1Tan{

		XMFLOAT3 Position;
		XMFLOAT3 Normal;
		XMFLOAT2 Tex0;
		XMFLOAT2 Tex1;
		XMFLOAT3 Tan;

		PosNorTex0Tex1Tan() {}

		PosNorTex0Tex1Tan(XMFLOAT3 pos, XMFLOAT3 nor, XMFLOAT3 tan, XMFLOAT2 tex0, XMFLOAT2 tex1)
			: Position(pos), Normal(nor), Tan(tan), Tex0(tex0), Tex1(tex1)
		{}

	};


	struct PosColor {
		XMFLOAT3 Position;
		XMFLOAT4 Color;

		PosColor() {}

		PosColor(XMFLOAT3 pos, XMFLOAT4 color) : Position(pos), Color(color)
		{}
	};
}

class InputLayouts
{

public:
	
	static void Init(ID3D11Device* device);
	static void Destroy();

	static ID3D11InputLayout *PosNormalTexTan;
	static ID3D11InputLayout *PosColor;
	static ID3D11InputLayout *PosNormalTex0Tex1Tan;
};