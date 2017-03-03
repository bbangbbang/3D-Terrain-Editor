#pragma once
#include "d3dUtil.h"
#include "Light.h"
#include <fstream>

#include "DisplacementMap.h"

#include "Color.h"


class Effect
{
public:
	Effect(ID3D11Device* device , const std::wstring& fileName);
	Effect(ID3D11Device *device, const BYTE* data, int size);
	virtual ~Effect();

protected:
	ID3DX11Effect* mFX;

};

class DisplacementMapEffect : public Effect
{
public:
	DisplacementMapEffect(ID3D11Device *device, const std::wstring& fileName ); 
	DisplacementMapEffect(ID3D11Device *device, const BYTE* data, int size);

	~DisplacementMapEffect();


	void SetWorld(CXMMATRIX world)						{ World->SetMatrix(reinterpret_cast<const float*>(&world)); }
	void SetWorldViewProj(CXMMATRIX worldViewProj)		{ WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&worldViewProj)); }
	void SetLayerMapArray(ID3D11ShaderResourceView* srv) { LayerMapArray->SetResource(srv); }
	void SetBlendMap(ID3D11ShaderResourceView* srv) { BlendMap->SetResource(srv); }
	void SetDirLight(DirectionalLight L) { DirLight->SetRawValue(&L, 0, sizeof(L)); }
	void SetMaterial(Material M) { Material->SetRawValue(&M, 0, sizeof(M)); }
	void SetEysPosW(XMFLOAT3 E) { EyePosW->SetRawValue(&E, 0, sizeof(E)); }

	
	ID3DX11EffectTechnique *BasicTech;
	ID3DX11EffectMatrixVariable *World;
	ID3DX11EffectMatrixVariable *WorldViewProj;
	ID3DX11EffectVectorVariable *EyePosW;
	ID3DX11EffectVariable* DirLight;
	ID3DX11EffectVariable* Material;
	ID3DX11EffectShaderResourceVariable *LayerMapArray;
	ID3DX11EffectShaderResourceVariable *BlendMap;


private:
	void Init();
};

class ColorEffect : public Effect
{
public:
	ColorEffect(ID3D11Device *device, const std::wstring& fileName);
	ColorEffect(ID3D11Device *device, const BYTE* data, int size);
	~ColorEffect();


	void SetWorld(CXMMATRIX world) { World->SetMatrix(reinterpret_cast<const float*>(&world)); }
	void SetWorldViewProj(CXMMATRIX worldViewProj) { WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&worldViewProj)); }


	ID3DX11EffectTechnique *BasicTech;

	ID3DX11EffectMatrixVariable *World;
	ID3DX11EffectMatrixVariable *WorldViewProj;

private:

	void Init();
};


class Effects
{
public:

	static void Init(ID3D11Device* device);
	static void Destroy();

	static DisplacementMapEffect	*DisplacementMapFX;
	static ColorEffect				*ColorFX;

};