#pragma once
#include "d3dUtil.h"
#include "InputLayouts.h"
#include "Terrain.h"

class BrushShape
{
public:
	BrushShape();
	~BrushShape();

	void InitCircle(ID3D11Device* device,XMVECTOR center, float radius, float theta);	
	void DrawCircle(ID3D11DeviceContext* dc, ID3DX11EffectTechnique* tech);
	
	void SetCenter(XMVECTOR center);
	void SetRadius(float radius);
	void SetTheta(float theta);
	void EnableTerrainHeight(Terrain *t);
	void UnableTerrainHeight();

	XMVECTOR GetCenter();
	float GetRadius();
	float GetTheta();
	
private:

	vector<Vertex::PosColor> mCircleVertices;
	XMFLOAT3 mCenter;
	float mRadius;
	float mTheta;
	UINT mCircleVertexCount;

	Terrain* mTerrain;


	void UpdateCircleVertices();
	ID3D11Buffer* mCircleVB;


};