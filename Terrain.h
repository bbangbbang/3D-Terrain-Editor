#pragma once
#include "d3dUtil.h"
#include "Effects.h"
#include "QuadTree.h"
#include "Collider.h"
#include "InputLayouts.h"
class Terrain
{
public:

	

	Terrain();
	~Terrain();

	void	Init(float width, float height, float depth,int m, int n, int nCell, ID3D11Device *device, ID3D11DeviceContext* dc);
	void	InitNormalMap();
	void	Draw(ID3D11DeviceContext* dc, ID3DX11EffectTechnique* tech, Collider::Frustum frustum, XMVECTOR camPos, bool bSLOD);
	void	SetDiffuseMapSRV(ID3D11ShaderResourceView* diffuseMapSRV);
	

	float	GetHeight(float x, float z);
	void	AddHeightInShape(Collider::Sphere s, float f, bool bFlat, bool bSmooth);
	void	FindVertexIndexInShape(Collider::Sphere s, vector<int>& vIndices);

	bool	IsIntersectVertexWithRay(FXMVECTOR origin, FXMVECTOR dir, XMVECTOR& v);

	void	Smooth(vector<int>& vIndices);
	void	UpdateNormal(vector<int>& vIndices);
	void	LoadHeightMap(TCHAR* filePath, int width, int depth, int height, int m, int format, bool bFlipVertical, bool bSignAbility);
	void	SaveHeightMapTo8bitRaw(TCHAR* filePath, int format);
	ID3D11Texture2D*	GetNormalMap();

private:
	
	int mVertexBufferCount;

	ID3D11DeviceContext* mDC;
	ID3D11Device* mDevice;
	UINT mTriCount;
	int mCellCount;
	int mLineVertexCount;
	int mEdgeTriCount;

	vector<Vertex::PosNorTex0Tex1Tan> mVertices;

	vector<ID3D11Buffer*> mVBs;
	vector<vector<vector<ID3D11Buffer*> > > mEdgeIBs;
	vector<vector<ID3D11Buffer*> >mIBs;
	
	UINT mIndexBufferLevelCount;
	float mMaxDist;

	ID3D11ShaderResourceView	*mDiffuseMapSRV;

	QuadTree mQuadTree;

	ID3D11Texture2D* mNormalTex;
	void CreateBuffer(float width, float depth, int m, int n, int nCell, ID3D11Device *device);
	void CreateIndexBuffer(ID3D11Device *device,int nCell);


	float mWidth;
	float mDepth;
	float mHeight;
	int mColCount;
	int mRowCount;

	float mCellSpaceX;
	float mCellSpaceZ;

};
