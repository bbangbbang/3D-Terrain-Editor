#pragma once
#include "d3dUtil.h"

struct DirectionalLight
{
	DirectionalLight() { ZeroMemory(this, sizeof(this)); }
	XMFLOAT4 Diffuse;
	XMFLOAT4 Ambient;
	XMFLOAT4 Specular;
	XMFLOAT3 Direction;
	float pad;
};

struct Material
{
	Material() { ZeroMemory(this, sizeof(this)); }
	XMFLOAT4 Diffuse;
	XMFLOAT4 Ambient;
	XMFLOAT4 Specular;
	XMFLOAT4 Reflect;
};