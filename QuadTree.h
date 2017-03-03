#pragma once
#include "d3dUtil.h"
#include "InputLayouts.h"
#include "Collider.h"
class QuadTree
{
public:
	
	enum CornerType {CORNER_TL, CORNER_TR, CORNER_BL, CORNER_BR};
	enum NeighborType {NEIGHBOR_LEFT, NEIGHBOR_TOP, NEIGHBOR_RIGHT, NEIGHBOR_BOTTOM};
	QuadTree();
	QuadTree(QuadTree*);

	~QuadTree();

	void Init(int m, int n, float height, int nTileGap);

	void GenerateIndex(vector<int> &vIndices, vector<int>& indexLevel, vector<vector<int>>& subLevel, vector<int>& tileShape,vector<Vertex::PosNorTex0Tex1Tan>& vertices,
		Collider::Frustum& frustum, XMVECTOR camPos, float maxDist, int indexBufferCount);

	void Build(vector<Vertex::PosNorTex0Tex1Tan>& vertices);
	
	int CalcShapeLevel(vector<Vertex::PosNorTex0Tex1Tan>& vertices, XMVECTOR camPos, float maxDist, int indexBufferCount);

	void AddHeightInShape(Collider::Sphere s, vector<Vertex::PosNorTex0Tex1Tan>& inVertices, float h, vector<int>& vbIndex, vector<bool>& bChange, bool bFlat);

	void	IsIntersectVertexWithRay(FXMVECTOR origin, FXMVECTOR dir, vector<Vertex::PosNorTex0Tex1Tan>& vertices,float& t, bool& bIsintersect);
	void	FindVertexIndexInShape(Collider::Sphere s, vector<Vertex::PosNorTex0Tex1Tan> &vin,vector<int>& vIndices, vector<bool>& bChange);
	
private:

	QuadTree* FindNode(int, int, int, int);

	

	int GetNodeIndex(int ,int&, int&, int&, int&, int, int);
	void BuildNeighborNode(int m, int n);
	void BuildTree(vector<Vertex::PosNorTex0Tex1Tan>& vertices);

	void FrustumCulling(vector<Vertex::PosNorTex0Tex1Tan>& vertices, Collider::Frustum& frustum);

	void GenTriIndex(std::vector<int>& vIndices, vector<int>& indexLevel, vector<vector<int>>& subLevel, vector<int>& tileShape,
		vector<Vertex::PosNorTex0Tex1Tan>& vertices, XMVECTOR camPos, float maxDist, int indexBufferCount);

	bool IsLeafNode();

	bool SubDivide();
	QuadTree* AddChild(int CornerTL, int CornerTR, int CornerBL, int CornerBR);
	void SetCorners(int CornerTL, int CornerTR, int CornerBL, int CornerBR);

	

	int mCorner[4];
	int mCenter;

	QuadTree* mNeighbor[4];
	QuadTree* mChild[4];
	QuadTree* mParent;

	bool mCulling;
	int mShapeLevel;

	Collider::AABB mAABB;
	
	static float mTerrainHeight;
	static int mRowVertexCount;
	static int mTileGap;
	static QuadTree *mRoot;


};