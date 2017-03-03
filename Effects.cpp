#include "Effects.h"


Effect::Effect(ID3D11Device* device, const std::wstring& fileName)
{
	std::ifstream in(fileName.c_str(), std::ios::binary);

	in.seekg(0, std::ios_base::end);
	int size = in.tellg();
	in.seekg(0, std::ios_base::beg);

	std::vector<char> data(size);

	in.read(&data[0], size);
	in.close();

	HR(D3DX11CreateEffectFromMemory(&data[0], size, 0, device, &mFX));

	
}


Effect::Effect(ID3D11Device* device, const BYTE* data, int size)
{
	

	HR(D3DX11CreateEffectFromMemory(data, size, 0, device, &mFX));


}

Effect::~Effect()
{
	ReleaseCOM(mFX);
}


DisplacementMapEffect::DisplacementMapEffect(ID3D11Device *device, const std::wstring& fileName)
: Effect(device , fileName)
{
	
	Init();

}

DisplacementMapEffect::DisplacementMapEffect(ID3D11Device * device, const BYTE * data, int size)
	:Effect(device, data, size)
{
	Init();
}

void DisplacementMapEffect::Init()
{

	BasicTech = mFX->GetTechniqueByName("BasicTech");

	World = mFX->GetVariableByName("gWorld")->AsMatrix();
	WorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
	LayerMapArray = mFX->GetVariableByName("gLayerMapArray")->AsShaderResource();
	BlendMap = mFX->GetVariableByName("gBlendMap")->AsShaderResource();
	EyePosW = mFX->GetVariableByName("gEyePosW")->AsVector();
	DirLight = mFX->GetVariableByName("gDirLight");
	Material = mFX->GetVariableByName("gMaterial");
}

DisplacementMapEffect::~DisplacementMapEffect(){


}


ColorEffect::ColorEffect(ID3D11Device *device, const std::wstring& fileName)
	: Effect(device, fileName)
{
	Init();
}

void ColorEffect::Init()
{

	BasicTech = mFX->GetTechniqueByName("BasicTech");

	World = mFX->GetVariableByName("gWorld")->AsMatrix();
	WorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
}

ColorEffect::ColorEffect(ID3D11Device * device, const BYTE* data, int size)
	:Effect(device,data, size)
{
	Init();
}

ColorEffect::~ColorEffect() {


}

void Effects::Init(ID3D11Device *device)
{

	//DisplacementMapFX = new DisplacementMapEffect(device, L"FX/DisplacementMap.fxo");
	//ColorFX = new ColorEffect(device, L"FX/Color.fxo");
	DisplacementMapFX = new DisplacementMapEffect(device, DisplacementMap, sizeof(DisplacementMap));
	ColorFX = new ColorEffect(device, color, sizeof(color));
}

void Effects::Destroy()
{
	delete DisplacementMapFX;
	delete ColorFX;
}


DisplacementMapEffect*	Effects::DisplacementMapFX = 0;
ColorEffect*			Effects::ColorFX = 0;