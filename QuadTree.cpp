#include "QuadTree.h"
#include <Windows.h>
#include <stdio.h>
#include "MathHelper.h"
#include "SLOD.h"

QuadTree* QuadTree::mRoot = 0;
int QuadTree::mTileGap = 0;
int QuadTree::mRowVertexCount = 0;
float QuadTree::mTerrainHeight = 0.0f;
QuadTree::QuadTree()
{
	mShapeLevel = 0;
	mParent = 0;
	for (int i = 0; i < 4; i++) {
		mChild[i] = 0;
		mCorner[i] = 0;
		mNeighbor[i] = 0;
	}
	mCulling = false;
}

QuadTree::QuadTree(QuadTree* pParent)
{
	QuadTree();
	mParent = pParent;
	
}

QuadTree::~QuadTree()
{
	for (int i = 0; i < 4; i ++)
		delete mChild[i];
}

void QuadTree::Init(int m, int n, float height, int nTileGap)
{
	mCorner[CORNER_TL] = 0;
	mCorner[CORNER_TR] = m - 1;
	mCorner[CORNER_BL] = (m - 1) * n;
	mCorner[CORNER_BR] = m * n - 1;
	
	mRowVertexCount = m;
	mRoot = this;
	mTileGap = nTileGap;
	mTerrainHeight = height;

}

void QuadTree::Build(vector<Vertex::PosNorTex0Tex1Tan>& vertices)
{

	BuildTree(vertices);

	BuildNeighborNode(mCorner[CORNER_TR] + 1, (mCorner[CORNER_BR]+1) / (mCorner[CORNER_TR] + 1));
	
}

void QuadTree::BuildTree(vector<Vertex::PosNorTex0Tex1Tan>& vertices)
{

	int n = mCorner[CORNER_TR] - mCorner[CORNER_TL];

	XMVECTOR vMax = XMLoadFloat3(&vertices[mCorner[CORNER_TL]].Position);
	XMVECTOR vMin = XMLoadFloat3(&vertices[mCorner[CORNER_TL]].Position);

	for (int z = 0; z <= n; z++)
	{
		for (int x = 0; x <= n; x++)
		{
			int off = mCorner[CORNER_TL] + z*mRowVertexCount + x;
			XMVECTOR V = XMLoadFloat3(&vertices[off].Position);

			vMax = XMVectorMax(vMax, V);
			vMin = XMVectorMin(vMin, V);
		}
	}

	XMStoreFloat3(&mAABB.Max, vMax);
	XMStoreFloat3(&mAABB.Min, vMin);

	mAABB.Max.y = mTerrainHeight/2.0f;
	mAABB.Min.y = -mTerrainHeight/2.0f;

	if (SubDivide())
	{
		mChild[CORNER_TL]->BuildTree(vertices);
		mChild[CORNER_TR]->BuildTree(vertices);
		mChild[CORNER_BL]->BuildTree(vertices);
		mChild[CORNER_BR]->BuildTree(vertices);
	}

}

void QuadTree::BuildNeighborNode(int m, int n)
{
	int TL, TR, BL, BR;
	if (IsLeafNode()) {
		for (int i = NEIGHBOR_LEFT; i < 4; i++)
		{
			if (GetNodeIndex(i, TL, TR, BL, BR, m, n) != -1)
				mNeighbor[i] = mRoot->FindNode(TL, TR, BL, BR);
		}
	}

	if (mChild[CORNER_TL]) mChild[CORNER_TL]->BuildNeighborNode(m, n);
	if (mChild[CORNER_TR]) mChild[CORNER_TR]->BuildNeighborNode(m, n);
	if (mChild[CORNER_BL]) mChild[CORNER_BL]->BuildNeighborNode(m, n);
	if (mChild[CORNER_BR]) mChild[CORNER_BR]->BuildNeighborNode(m, n);
	
}

int QuadTree::GetNodeIndex(int type, int& TL, int& TR, int& BL, int& BR, int m, int n)
{
	int gap = mCorner[CORNER_TR] - mCorner[CORNER_TL];

	switch (type)
	{
	case NEIGHBOR_LEFT:
		if (mCorner[CORNER_TL] % m == 0) return -1;
		TL = mCorner[CORNER_TL] - gap;
		TR = mCorner[CORNER_TL];
		BL = mCorner[CORNER_BL] - gap;
		BR = mCorner[CORNER_BL];
		break;
	
	case NEIGHBOR_TOP:
		TL = mCorner[CORNER_TL] - gap * m;
		TR = mCorner[CORNER_TR] - gap * m;
		BL = mCorner[CORNER_TL];
		BR = mCorner[CORNER_TR];
		if (TL < 0) return -1;
		break;
	
	case NEIGHBOR_RIGHT:
		if ((mCorner[CORNER_TR]+1) % m == 0) return -1;
		TL = mCorner[CORNER_TR];
		TR = mCorner[CORNER_TR] + gap;
		BL = mCorner[CORNER_BR];
		BR = mCorner[CORNER_BR] + gap;
		break;

	case NEIGHBOR_BOTTOM:
		TL = mCorner[CORNER_BL];
		TR = mCorner[CORNER_BR];
		BL = mCorner[CORNER_BL] + gap * m;
		BR = mCorner[CORNER_BR] + gap * m;
		if (BL >= m*n) return -1;
		break;
	}

	return 0;
}

QuadTree* QuadTree::FindNode(int TL, int TR, int BL, int BR)
{
	static QuadTree *node = NULL;

	if (mCorner[CORNER_TL] == TL && mCorner[CORNER_TR] == TR && mCorner[CORNER_BL] == BL && mCorner[CORNER_BR] == BR)
		return this;



	if (mChild[CORNER_TL]) node = mChild[CORNER_TL]->FindNode(TL, TR, BL, BR);
	if (mChild[CORNER_TR]) node = mChild[CORNER_TR]->FindNode(TL, TR, BL, BR);
	if (mChild[CORNER_BL]) node = mChild[CORNER_BL]->FindNode(TL, TR, BL, BR);
	if (mChild[CORNER_BR]) node = mChild[CORNER_BR]->FindNode(TL, TR, BL, BR);

	return node;
}

bool QuadTree::SubDivide()
{

	
	int topCenterEdge;
	int leftCenterEdge;
	int rightCenterEdge;
	int bottomCenterEdge;
	int centerEdge;

	topCenterEdge = (mCorner[CORNER_TR] + mCorner[CORNER_TL]) / 2;
	leftCenterEdge = (mCorner[CORNER_TL] + mCorner[CORNER_BL]) / 2;
	rightCenterEdge = (mCorner[CORNER_TR] + mCorner[CORNER_BR]) / 2;
	bottomCenterEdge = (mCorner[CORNER_BL] + mCorner[CORNER_BR]) / 2;
	centerEdge = (mCorner[CORNER_TL] + mCorner[CORNER_TR] + mCorner[CORNER_BL] + mCorner[CORNER_BR]) / 4;




	if (IsLeafNode()) return false;


	mChild[CORNER_TL] = AddChild(mCorner[CORNER_TL], topCenterEdge, leftCenterEdge, centerEdge);
	mChild[CORNER_TR] = AddChild(topCenterEdge,mCorner[CORNER_TR],centerEdge,rightCenterEdge);
	mChild[CORNER_BL] = AddChild(leftCenterEdge,centerEdge,mCorner[CORNER_BL],bottomCenterEdge);
	mChild[CORNER_BR] = AddChild(centerEdge,rightCenterEdge,bottomCenterEdge,mCorner[CORNER_BR]);

	return true;
}

QuadTree* QuadTree::AddChild(int CornerTL, int CornerTR, int CornerBL, int CornerBR)
{
	QuadTree* pChild = new QuadTree(this);
	pChild->SetCorners(CornerTL, CornerTR, CornerBL, CornerBR);

	return pChild;

}

void QuadTree::SetCorners(int CornerTL, int CornerTR, int CornerBL, int CornerBR)
{
	mCorner[CORNER_TL] = CornerTL;
	mCorner[CORNER_TR] = CornerTR;
	mCorner[CORNER_BL] = CornerBL;
	mCorner[CORNER_BR] = CornerBR;

	mCenter = (mCorner[0] + mCorner[1] + mCorner[2] + mCorner[3]) / 4.0;
}
void QuadTree::GenerateIndex(vector<int> &vIndices, vector<int>& indexLevel, vector<vector<int>>& subLevel,vector<int>& tileShape, vector<Vertex::PosNorTex0Tex1Tan>& vertices,
	Collider::Frustum& frustum, XMVECTOR camPos, float maxDist, int indexBufferCount)
{
	FrustumCulling(vertices, frustum);
	GenTriIndex(vIndices, indexLevel, subLevel, tileShape, vertices, camPos, maxDist, indexBufferCount);
}

void QuadTree::FrustumCulling(vector<Vertex::PosNorTex0Tex1Tan>& vertices, Collider::Frustum& frustum)
{
	
	if (!Collider::IsIntersectAABBWithFrustum(mAABB, frustum))
	{
		mCulling = true;
		return;
	}

	
	if (mChild[CORNER_TL]) mChild[CORNER_TL]->FrustumCulling(vertices, frustum);
	if (mChild[CORNER_TR]) mChild[CORNER_TR]->FrustumCulling(vertices, frustum);
	if (mChild[CORNER_BL]) mChild[CORNER_BL]->FrustumCulling(vertices, frustum);
	if (mChild[CORNER_BR]) mChild[CORNER_BR]->FrustumCulling(vertices, frustum);
}

void QuadTree::GenTriIndex(std::vector<int>& vIndices, vector<int>& indexLevel, vector<vector<int>>& subLevel,vector<int>& tileShape,
	vector<Vertex::PosNorTex0Tex1Tan>& vertices, XMVECTOR camPos, float maxDist, int indexBufferCount)
{


	if (mCulling) {
		mCulling = false;
		return;
	}

	if (IsLeafNode())
	{
		
		int TL = mCorner[CORNER_TL];
		int gap = mTileGap;
		int nTile = (mRowVertexCount - 1) / gap;
		int  i = nTile * (TL / (mRowVertexCount*gap)) + (TL%mRowVertexCount) / gap;

		
		mShapeLevel = CalcShapeLevel(vertices,camPos,maxDist,indexBufferCount);

		vIndices.push_back(i);
		indexLevel.push_back(mShapeLevel);

		bool b[4] = { false };

		vector<int> edgeLevel(4);
		for (int i = NEIGHBOR_LEFT; i < 4; i++)
		{
			edgeLevel[i] = -1;
			if (mNeighbor[i])
			{
				int level = mNeighbor[i]->CalcShapeLevel(vertices, camPos, maxDist, indexBufferCount);
				if (mShapeLevel < level) // level이 높을수룩 세부수준이 낮은거임
				{
					edgeLevel[i] = level - mShapeLevel - 1;
					b[i] = true;

				}
			
			}
		}
		subLevel.push_back(edgeLevel);
		if (b[NEIGHBOR_LEFT] && b[NEIGHBOR_TOP] && b[NEIGHBOR_RIGHT] && b[NEIGHBOR_BOTTOM])
			tileShape.push_back(SLOD_MARGIN_LEFT_TOP_RIGHT_BOTTOM);
		else if (b[NEIGHBOR_LEFT] && b[NEIGHBOR_TOP] && b[NEIGHBOR_RIGHT])
			tileShape.push_back(SLOD_MARGIN_LEFT_TOP_RIGHT);
		else if (b[NEIGHBOR_TOP] && b[NEIGHBOR_RIGHT] && b[NEIGHBOR_BOTTOM])
			tileShape.push_back(SLOD_MARGIN_TOP_RIGHT_BOTTOM);
		else if (b[NEIGHBOR_LEFT] && b[NEIGHBOR_RIGHT] && b[NEIGHBOR_BOTTOM])
			tileShape.push_back(SLOD_MARGIN_LEFT_RIGHT_BOTTOM);
		else if (b[NEIGHBOR_LEFT] && b[NEIGHBOR_TOP] && b[NEIGHBOR_BOTTOM])
			tileShape.push_back(SLOD_MARGIN_LEFT_TOP_BOTTOM);
		else if (b[NEIGHBOR_TOP] && b[NEIGHBOR_BOTTOM])
			tileShape.push_back(SLOD_MARGIN_TOP_BOTTOM);
		else if (b[NEIGHBOR_LEFT] && b[NEIGHBOR_RIGHT])
			tileShape.push_back(SLOD_MARGIN_LEFT_RIGHT);
		else if (b[NEIGHBOR_LEFT] && b[NEIGHBOR_TOP])
			tileShape.push_back(SLOD_MARGIN_LEFT_TOP);
		else if (b[NEIGHBOR_LEFT] && b[NEIGHBOR_BOTTOM])
			tileShape.push_back(SLOD_MARGIN_LEFT_BOTTOM);
		else if (b[NEIGHBOR_RIGHT] && b[NEIGHBOR_BOTTOM])
			tileShape.push_back(SLOD_MARGIN_RIGHT_BOTTOM);
		else if (b[NEIGHBOR_TOP] && b[NEIGHBOR_RIGHT])
			tileShape.push_back(SLOD_MARGIN_TOP_RIGHT);
		else if (b[NEIGHBOR_TOP])
			tileShape.push_back(SLOD_MARGIN_TOP);
		else if (b[NEIGHBOR_BOTTOM])
			tileShape.push_back(SLOD_MARGIN_BOTTOM);
		else if (b[NEIGHBOR_LEFT])
			tileShape.push_back(SLOD_MARGIN_LEFT);
		else if (b[NEIGHBOR_RIGHT])
			tileShape.push_back(SLOD_MARGIN_RIGHT);
		else
			tileShape.push_back(SLOD_MARGIN_NONE);

		return;
	}



	if (mChild[CORNER_TL]) mChild[CORNER_TL]->GenTriIndex(vIndices, indexLevel, subLevel, tileShape, vertices, camPos, maxDist, indexBufferCount);
	if (mChild[CORNER_TR]) mChild[CORNER_TR]->GenTriIndex(vIndices, indexLevel, subLevel, tileShape, vertices, camPos, maxDist, indexBufferCount);
	if (mChild[CORNER_BL]) mChild[CORNER_BL]->GenTriIndex(vIndices, indexLevel, subLevel, tileShape, vertices, camPos, maxDist, indexBufferCount);
	if (mChild[CORNER_BR]) mChild[CORNER_BR]->GenTriIndex(vIndices, indexLevel, subLevel, tileShape, vertices, camPos, maxDist, indexBufferCount);


}

bool QuadTree::IsLeafNode()
{
	return (mCorner[CORNER_TR] - mCorner[CORNER_TL] <= mTileGap);
}


int QuadTree::CalcShapeLevel(vector<Vertex::PosNorTex0Tex1Tan>& vertices, XMVECTOR camPos, float maxDist, int indexBufferCount) {


	float dist = XMVectorGetX(XMVector3Length(XMLoadFloat3(&vertices[mCenter].Position) - camPos));
	dist /= maxDist;
	int level = dist * indexBufferCount;
	level = MathHelper::Clamp(level, 0, indexBufferCount);
	return level;
}



void QuadTree::IsIntersectVertexWithRay(FXMVECTOR origin, FXMVECTOR dir, vector<Vertex::PosNorTex0Tex1Tan>& vertices, float& t, bool& bIntersect)
{

	Collider::Ray ray(origin, dir);

	if (!Collider::IsIntersectAABBWithRay(mAABB, ray)) {
		return;
	}



	if (IsLeafNode())
	{
		XMVECTOR v0, v1, v2;
		int cellCount = mCorner[CORNER_TR] - mCorner[CORNER_TL];

		for (int z = 0; z < cellCount; z++)
		{
			for (int x = 0; x < cellCount; x++)
			{
				float _t;
				if ((x < cellCount / 2 && z < cellCount / 2) || (x >= cellCount / 2 && z >= cellCount / 2)) {
					v0 = XMLoadFloat3(&vertices[mCorner[CORNER_TL] + z*mRowVertexCount + x].Position);
					v1 = XMLoadFloat3(&vertices[mCorner[CORNER_TL] + z*mRowVertexCount + x + 1].Position);
					v2 = XMLoadFloat3(&vertices[mCorner[CORNER_TL] + (z + 1)*mRowVertexCount + x + 1].Position);


					if (Collider::IsIntersectTriangleWithRay(ray, v0, v1, v2, _t)) {

						if (!bIntersect) t = _t;
						else t = fmin(t, _t);
						bIntersect = true;
					}

					v0 = XMLoadFloat3(&vertices[mCorner[CORNER_TL] + z*mRowVertexCount + x].Position);
					v1 = XMLoadFloat3(&vertices[mCorner[CORNER_TL] + (z + 1)*mRowVertexCount + x + 1].Position);
					v2 = XMLoadFloat3(&vertices[mCorner[CORNER_TL] + (z + 1)*mRowVertexCount + x].Position);

					if (Collider::IsIntersectTriangleWithRay(ray, v0, v1, v2, _t)) {
						if (!bIntersect) t = _t;
						else t = fmin(t, _t);
						bIntersect = true;

					}
				}

				else
				{
					v0 = XMLoadFloat3(&vertices[mCorner[CORNER_TL] + z*mRowVertexCount + x].Position);
					v1 = XMLoadFloat3(&vertices[mCorner[CORNER_TL] + z*mRowVertexCount + x + 1].Position);
					v2 = XMLoadFloat3(&vertices[mCorner[CORNER_TL] + (z + 1)*mRowVertexCount + x].Position);


					if (Collider::IsIntersectTriangleWithRay(ray, v0, v1, v2, _t)) {
						if (!bIntersect) t = _t;
						else t = fmin(t, _t);
						bIntersect = true;
					}

					v0 = XMLoadFloat3(&vertices[mCorner[CORNER_TL] + z*mRowVertexCount + x + 1].Position);
					v1 = XMLoadFloat3(&vertices[mCorner[CORNER_TL] + (z + 1)*mRowVertexCount + x + 1].Position);
					v2 = XMLoadFloat3(&vertices[mCorner[CORNER_TL] + (z + 1)*mRowVertexCount + x].Position);

					if (Collider::IsIntersectTriangleWithRay(ray, v0, v1, v2, _t)) {
						if (!bIntersect) t = _t;
						else t = fmin(t, _t);

						bIntersect = true;
					}
				}
			}
		}

	}

	else
	{
		mChild[CORNER_TL]->IsIntersectVertexWithRay(origin, dir, vertices, t, bIntersect);
		mChild[CORNER_TR]->IsIntersectVertexWithRay(origin, dir, vertices, t, bIntersect);
		mChild[CORNER_BL]->IsIntersectVertexWithRay(origin, dir, vertices, t, bIntersect);
		mChild[CORNER_BR]->IsIntersectVertexWithRay(origin, dir, vertices, t, bIntersect);
	}
}


void QuadTree::AddHeightInShape(Collider::Sphere s, vector<Vertex::PosNorTex0Tex1Tan>& vin, 
	float h, vector<int>& vbIndex, vector<bool>& bChange, bool bFlat)
{	

	
	static int m = 0;
	s.Center.y = 0.0f;
	if (mRoot == this)	m = mCorner[CORNER_TR] - mCorner[CORNER_TL]+1;

	if (!Collider::IsIntersectAABBWithSphere(mAABB, s)) return; 


	if (IsLeafNode())
	{

		int TL = mCorner[CORNER_TL];
		int gap = mTileGap;
		int nTile = (mRowVertexCount - 1) / gap;
		int  i = nTile * (TL / (mRowVertexCount*gap)) + (TL%mRowVertexCount) / gap;
		vbIndex.push_back(i);
		bool b[4] = { false, };

		for (int i = 0; i < 4; i++)
		{
			XMFLOAT3 vv = vin[mCorner[i]].Position;
			vv.y = 0.0f;
			XMVECTOR v = XMLoadFloat3(&vv);
			if (Collider::IsIntersectSphereWithPoint(s, v)) b[i] = true;
			else break;
		}
		
		if (b[0] && b[1] && b[2] && b[3])
		{
			int gap = mCorner[CORNER_TR] - mCorner[CORNER_TL];
			for (int i = 0; i < gap + 1; i++)
			{
				for (int j = 0; j < gap + 1; j++)
				{
					int k = (i*m + mCorner[CORNER_TL] + j);
					if (bChange[k] == false) {
						float y = vin[k].Position.y;
						
						if (bFlat)
						{
							if (y > 0)
							{
								if (y - h <= 0) vin[k].Position.y = 0;
								else vin[k].Position.y -= h;
								bChange[k] = true;
							}
							else if (y < 0)
							{

								if (y + h >= 0) vin[k].Position.y = 0;
								else vin[k].Position.y += h;
								bChange[k] = true;
							}
						}
						else {
							vin[k].Position.y += h;
							bChange[k] = true;
						}
						if(h > 0) vin[k].Position.y = min(vin[k].Position.y, mTerrainHeight/2.0f);
						if(h < 0)  vin[k].Position.y = max(vin[k].Position.y, -mTerrainHeight / 2.0f);
					}
				}
			}
			
			return;
		}

		gap = mCorner[CORNER_TR] - mCorner[CORNER_TL];
		for (int i = 0; i < gap + 1; i++)
		{
			for (int j = 0; j < gap + 1; j++)
			{
				XMFLOAT3 vv = vin[i*m + mCorner[CORNER_TL] + j].Position;
				vv.y = 0.0f;
				XMVECTOR v = XMLoadFloat3(&vv);
				
				if (Collider::IsIntersectSphereWithPoint(s, v)) {
					int k = (i*m + mCorner[CORNER_TL] + j);
					if (bChange[k] == false) {
						float y = vin[(i*m + mCorner[CORNER_TL] + j)].Position.y;

						if (bFlat)
						{
							if (y > 0)
							{
								if (y - h <= 0) vin[k].Position.y = 0;
								else vin[k].Position.y -= h;
								bChange[k] = true;
							}
							else if (y < 0)
							{

								if (y + h >= 0) vin[k].Position.y = 0;
								else vin[k].Position.y += h;
								bChange[k] = true;
							}
						}
						else {
							vin[k].Position.y += h;
							bChange[k] = true;


						}

						if (h > 0) vin[k].Position.y = min(vin[k].Position.y, mTerrainHeight / 2.0f);
						if (h < 0)  vin[k].Position.y = max(vin[k].Position.y, -mTerrainHeight / 2.0f);
					}


				}
			}
		}

		return;
	}

	else
	{

		mChild[CORNER_TL]->AddHeightInShape(s, vin, h, vbIndex, bChange,bFlat);
		mChild[CORNER_TR]->AddHeightInShape(s, vin, h, vbIndex, bChange,bFlat);
		mChild[CORNER_BL]->AddHeightInShape(s, vin, h, vbIndex, bChange,bFlat);
		mChild[CORNER_BR]->AddHeightInShape(s, vin, h, vbIndex, bChange,bFlat);
	}


}



void QuadTree::FindVertexIndexInShape(Collider::Sphere s, vector<Vertex::PosNorTex0Tex1Tan> &vin ,vector<int>& vIndices,
	vector<bool>& bChange)
{
	static int m = 0;
	if (mRoot == this)
	{
		m = mCorner[CORNER_TR] - mCorner[CORNER_TL] + 1;

	}

	s.Center.y = 0.0f;


	if (!Collider::IsIntersectAABBWithSphere(mAABB, s)) {
		return;
	}

	if (IsLeafNode())
	{

		bool b[4] = { false, };

		for (int i = 0; i < 4; i++)
		{
			XMFLOAT3 vv = vin[mCorner[i]].Position;
			vv.y = 0.0f;
			XMVECTOR v = XMLoadFloat3(&vv);
			if (Collider::IsIntersectSphereWithPoint(s, v))
				b[i] = true;
			else break;
		}

		if (b[0] && b[1] && b[2] && b[3])
		{
			int gap = mCorner[CORNER_TR] - mCorner[CORNER_TL];
			for (int i = 0; i < gap + 1; i++)
			{
				for (int j = 0; j < gap + 1; j++)
				{
					int k = i*m + mCorner[CORNER_TL] + j;
					if (!bChange[k]) {
						vIndices.push_back(k);
						bChange[k] = true;
					}

				}
			}

			return;
		}

		int gap = mCorner[CORNER_TR] - mCorner[CORNER_TL];
		for (int i = 0; i < gap + 1; i++)
		{
			for (int j = 0; j < gap + 1; j++)
			{
				int k = i*m + mCorner[CORNER_TL] + j;

				if (!bChange[k]) {
					XMFLOAT3 vv = vin[i*m + mCorner[CORNER_TL] + j].Position;
					vv.y = 0.0f;
					XMVECTOR v = XMLoadFloat3(&vv);


					if (Collider::IsIntersectSphereWithPoint(s, v)) {				
						vIndices.push_back(k);
						bChange[k] = true;

					}
				}
			
			}

		}

		return;
	}

	else
	{

		mChild[CORNER_TL]->FindVertexIndexInShape(s, vin, vIndices,bChange);
		mChild[CORNER_TR]->FindVertexIndexInShape(s, vin, vIndices,bChange);
		mChild[CORNER_BL]->FindVertexIndexInShape(s, vin, vIndices,bChange);
		mChild[CORNER_BR]->FindVertexIndexInShape(s, vin, vIndices,bChange);
	}
}

