#include "Terrain.h"
#include "Effects.h"
#include "InputLayouts.h"
#include "MathHelper.h"
#include "SLOD.h"
Terrain::Terrain()
: mVBs(0), mIBs(0), mDiffuseMapSRV(0),mTriCount(0),
  mIndexBufferLevelCount(0), mMaxDist(10.0f),mDC(0), mCellCount(0),mLineVertexCount(0),
  mDevice(0),mEdgeIBs(0), mEdgeTriCount(0),mNormalTex(0)
  
{
}

Terrain::~Terrain()
{
	for (UINT i = 0; i < mVBs.size(); i ++)
		ReleaseCOM(mVBs[i]);

	for (UINT i = 0; i < mIBs.size(); i++)
	{
		for (UINT j = 0; j < mIBs[i].size(); j++)
				ReleaseCOM(mIBs[i][j]);
	}
	ReleaseCOM(mDiffuseMapSRV); 
	
	for (UINT i = 0; i < mEdgeIBs.size(); i++)
	{
		for (UINT j = 0; j < mEdgeIBs[i].size(); j++)
		{
			for (UINT k = 0; k < mEdgeIBs[i][j].size(); k++) {
				ReleaseCOM(mEdgeIBs[i][j][j]);
			}
			mEdgeIBs[i][j].clear();
			
		}
		mEdgeIBs[i].clear();
	}
	mEdgeIBs.clear();
}

void Terrain::SetDiffuseMapSRV(ID3D11ShaderResourceView* diffuseMapSRV)
{
	mDiffuseMapSRV = diffuseMapSRV;
	
}
/**
@brief 지형을 구성할 정정버퍼와 인덱스 버퍼를 생성.
@param width : 지형의 넓이(x축)
@param depth : 지형의 깊이(z축)
@param m : 한 열당 정점 개수
@param n : 한 행당 정점 개수
@param nCell : 한 타일당 셀 개수
*/
void Terrain::CreateBuffer(float width, float depth, int m, int n, 
	int nCell, ID3D11Device *device)
{

	for (UINT i = 0; i < mVBs.size(); i++) {
		ReleaseCOM(mVBs[i]);
		mVBs.clear();
	}
	for (UINT i = 0; i < mIBs.size(); i++)
	{
		for (UINT j = 0; j < mIBs[i].size(); j++) {
				ReleaseCOM(mIBs[i][j]);
				
		}

		mIBs[i].clear();
	}
	mIBs.clear();

	for (UINT i = 0; i < mEdgeIBs.size(); i++)
	{
		for (UINT j = 0; j < mEdgeIBs[i].size(); j++)
		{
			for (UINT k = 0; k < mEdgeIBs[i][j].size(); k++) {
				ReleaseCOM(mEdgeIBs[i][j][k]);
			}
			mEdgeIBs[i][j].clear();
		}
		mEdgeIBs[i].clear();
	}
	mEdgeIBs.clear();

	mDevice = device;	
	mCellCount = nCell;

	int vertexCountOfTile = (nCell + 1)*(nCell + 1); // 타일당 정점 개수
	
	mVertices.clear();
	mVertices.resize(m * n); // m * n : 지형 전체의 정점 개수

	float halfWidth = width / 2.0f;
	float halfDepth = depth / 2.0f;
	
	float dx = width / (m - 1); // 정점간 간격(x축)
	float dz = depth / (n - 1); // 정점간 간격(z축)

	float du = 1.0f / (m - 1); // 텍스쳐 간격(u축)
	float dv = 1.0f / (n - 1); // 텍스쳐 간격(v축)


	int k = 0;
	
	int tileCnt = (m - 1) / mCellCount; // 한 열당 타일 개수
	
	for (int i = 0; i < n; i++)
	{
		float z = halfDepth - dz * i;

		for (int j = 0; j < m; j++)
		{
			float x = dx * j - halfWidth;

			mVertices[k].Position = XMFLOAT3(x, 0.0f, z);
			mVertices[k].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
			mVertices[k].Tex0 = XMFLOAT2(j*du, i*dv);
			mVertices[k].Tex1 = XMFLOAT2(j*du*tileCnt, i*dv*tileCnt);
			mVertices[k].Tan = XMFLOAT3(1.0f, 0.0f, 0.0f);

			k++;
		}
	}


	int nTileX = (m - 1) / nCell; // 한 열당 타일 개수
	int nTileZ = (n - 1) / nCell; // 한 행당 타일 개수

	// 타일마다 정점 버퍼를 생성
	for (int i = 0; i < nTileZ; i++)
	{
		for (int j = 0; j < nTileX; j++)
		{
			D3D11_BUFFER_DESC vbd;
			vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			vbd.ByteWidth = sizeof(Vertex::PosNorTex0Tex1Tan)* vertexCountOfTile;
			vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			vbd.MiscFlags = 0;
			vbd.StructureByteStride = 0;
			vbd.Usage = D3D11_USAGE_DYNAMIC;

			vector<Vertex::PosNorTex0Tex1Tan> vertices;	
			// 타일의 정점 인덱스들을 계산해서 저장
			for (int z = 0; z < nCell + 1; z++)
			{
				for (int x = 0; x < nCell + 1; x++)
				{
					int off = (z*m + x) + i*(m*nCell) + j * nCell; // i*(m*nCell) + j * nCell : 타일의 시작 정점 인덱스
					vertices.push_back(mVertices[off]);
				}
			}

			D3D11_SUBRESOURCE_DATA vdata;
			vdata.pSysMem = &vertices[0];

			ID3D11Buffer* buf;
			device->CreateBuffer(&vbd, &vdata, &buf);
			mVBs.push_back(buf);
		}
	}


	CreateIndexBuffer(device, nCell);

	
}

void Terrain::Init(float width, float height, float depth, int m, int n, int nCell, ID3D11Device *device, ID3D11DeviceContext* dc)
{

	mDC = dc;
	mWidth = width;
	mDepth = depth;
	mHeight = height;
	mLineVertexCount = m;
	mColCount = m;
	mRowCount = n;
	CreateBuffer(width, depth, m, n, nCell, device);
	mMaxDist = sqrt(width*width + depth*depth);
	mEdgeTriCount = nCell*5;
	mCellSpaceX = mWidth / (mColCount - 1);
	mCellSpaceZ = mDepth / (mRowCount - 1);


	mQuadTree.Init(m, n, height, nCell);
	mQuadTree.Build(mVertices);
	InitNormalMap();
}
void Terrain::InitNormalMap()
{

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = mColCount;
	desc.Height = mRowCount;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	mDevice->CreateTexture2D(&desc, 0, &mNormalTex);



}

/**
@brief 지형 Draw.
@param tech : 지형에 적용할 셰이더
@param frustum : 시야 절두체
@param camPos : 현재 카메라 위치(월드기준)
*/
void Terrain::Draw(ID3D11DeviceContext* dc, ID3DX11EffectTechnique* tech, Collider::Frustum frustum, XMVECTOR camPos, bool bSLOD)
{
	
	dc->IASetInputLayout(InputLayouts::PosNormalTex0Tex1Tan);
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	UINT stride = sizeof(Vertex::PosNorTex0Tex1Tan);
	UINT offset = 0;

	
	vector<int> vbIndices; // Draw할 정점 버퍼 리스트
	vector<int> indexLevel; // SLOD 레벨 리스트
	vector<int> tileShape; // 타일 모양 리스트 
	vector<vector<int> > subLevel;

	mQuadTree.GenerateIndex(vbIndices, indexLevel, subLevel,tileShape, mVertices, frustum, camPos, mMaxDist, mIndexBufferLevelCount);
	

	

	for (int i = 0; i < vbIndices.size(); i++) {
		dc->IASetVertexBuffers(0, 1, &mVBs[vbIndices[i]], &stride, &offset);
		if(!bSLOD) dc->IASetIndexBuffer(mIBs[0][0], DXGI_FORMAT_R32_UINT, 0);
		else dc->IASetIndexBuffer(mIBs[indexLevel[i]][tileShape[i]], DXGI_FORMAT_R32_UINT, 0);
		int nTriCnt = mTriCount * 3 / pow(2, indexLevel[i]);

		D3DX11_TECHNIQUE_DESC techDesc;
		tech->GetDesc(&techDesc);


		for (int p = 0; p < techDesc.Passes; p++) {

			tech->GetPassByIndex(p)->Apply(0, dc);
			if(bSLOD)dc->DrawIndexed(nTriCnt, 0, 0);
			else dc->DrawIndexed(mTriCount*3, 0, 0);

		}
		if (bSLOD) {
			if (tileShape[i] != 0) {
				for (int k = QuadTree::NEIGHBOR_LEFT; k < 4; k++) {
					if (subLevel[i][k] != -1) {
						dc->IASetIndexBuffer(mEdgeIBs[k][indexLevel[i]][subLevel[i][k]], DXGI_FORMAT_R32_UINT, 0);
						for (int p = 0; p < techDesc.Passes; p++) {
							tech->GetPassByIndex(p)->Apply(0, dc);
							dc->DrawIndexed(mEdgeTriCount, 0, 0);
						}
					}
				}

			}
		}
		
		
	}
}


/**
@brief 지형을 구성하는 정점과 반직선 교차여부.
@param origin : 반직선 시작 위치
@param dir : 반직선 방향 벡터
@param v : 교차할 경우 정점 위치가 저장됨
*/
bool Terrain::IsIntersectVertexWithRay(FXMVECTOR origin, FXMVECTOR dir, XMVECTOR& v)
{

	bool b = false;
	float t;
	mQuadTree.IsIntersectVertexWithRay(origin, dir, mVertices, t, b);
	if (b)	v = origin + t * dir;
	
	return b;
}



void Terrain::CreateIndexBuffer(ID3D11Device *device,int nCell)
{


	mTriCount = nCell * nCell * 2;
	mIndexBufferLevelCount = log2(nCell);

	vector<vector<ID3D11Buffer*>> topBufss, botBufss, rightBufss, leftBufss;

	for (int k = 0; k < mIndexBufferLevelCount + 1; k++)
	{
		int gap = pow(2, k);

		int m = nCell + 1;
		int n = nCell + 1;
		vector<ID3D11Buffer*> bufs;
		for (int l = 0; l < 16; l++) {

			vector<UINT> indices;
			int leftMarginColIdx = 0;
			int rightMarginColIdx = 0;
			int topMarginRowIdx = 0;
			int botMarginRowIdx = 0;

			if (k != mIndexBufferLevelCount)
			{
				switch (l)
				{
				case SLOD_MARGIN_NONE:
					leftMarginColIdx = 0;
					rightMarginColIdx = 0;
					topMarginRowIdx = 0;
					botMarginRowIdx = 0;
					break;

				case SLOD_MARGIN_LEFT_TOP_RIGHT_BOTTOM:
					if (k == mIndexBufferLevelCount - 1) {

						leftMarginColIdx = 0;
						rightMarginColIdx = 0;
						topMarginRowIdx = 0;
						botMarginRowIdx = 0;
					}
					else {
						leftMarginColIdx = gap;
						rightMarginColIdx = gap;
						topMarginRowIdx = gap;
						botMarginRowIdx = gap;
					}
					break;

				case SLOD_MARGIN_TOP:
					leftMarginColIdx = 0;
					rightMarginColIdx = 0;
					topMarginRowIdx = gap;
					botMarginRowIdx = 0;

					indices.push_back(0);
					indices.push_back(m *gap + gap);
					indices.push_back(m * gap);

					indices.push_back(nCell);
					indices.push_back(m*gap + nCell);
					indices.push_back(m*gap + nCell - gap);

					break;
				case SLOD_MARGIN_BOTTOM:
					leftMarginColIdx = 0;
					rightMarginColIdx = 0;
					topMarginRowIdx = 0;
					botMarginRowIdx = gap;

					indices.push_back(m*nCell);
					indices.push_back(m*(nCell - gap));
					indices.push_back(m*(nCell - gap) + gap);

					indices.push_back(m*nCell + nCell);
					indices.push_back(m*(nCell - gap) + nCell - gap);
					indices.push_back(m*(nCell - gap) + nCell);
					break;

				case SLOD_MARGIN_LEFT:
					leftMarginColIdx = gap;
					rightMarginColIdx = 0;
					topMarginRowIdx = 0;
					botMarginRowIdx = 0;

					indices.push_back(0);
					indices.push_back(gap);
					indices.push_back(m *gap + gap);

					indices.push_back(m*nCell);
					indices.push_back(m*(nCell - gap) + gap);
					indices.push_back(m*nCell + gap);
					break;

				case SLOD_MARGIN_RIGHT:
					leftMarginColIdx = 0;
					rightMarginColIdx = gap;
					topMarginRowIdx = 0;
					botMarginRowIdx = 0;

					indices.push_back(nCell - gap);
					indices.push_back(nCell);
					indices.push_back(m * gap + nCell - gap);

					indices.push_back(m*(nCell - gap) + nCell - gap);
					indices.push_back(m*nCell + nCell);
					indices.push_back(m*nCell + nCell - gap);
					break;

				case SLOD_MARGIN_TOP_BOTTOM:
					leftMarginColIdx = 0;
					rightMarginColIdx = 0;
					topMarginRowIdx = gap;
					botMarginRowIdx = gap;

					indices.push_back(0);
					indices.push_back(m *gap + gap);
					indices.push_back(m * gap);

					indices.push_back(nCell);
					indices.push_back(m*gap + nCell);
					indices.push_back(m*gap + nCell - gap);

					indices.push_back(m*nCell);
					indices.push_back(m*(nCell - gap));
					indices.push_back(m*(nCell - gap) + gap);

					indices.push_back(m*nCell + nCell);
					indices.push_back(m*(nCell - gap) + nCell - gap);
					indices.push_back(m*(nCell - gap) + nCell);
					break;
				case SLOD_MARGIN_LEFT_RIGHT:
					leftMarginColIdx = gap;
					rightMarginColIdx = gap;
					topMarginRowIdx = 0;
					botMarginRowIdx = 0;

					indices.push_back(0);
					indices.push_back(gap);
					indices.push_back(m *gap + gap);

					indices.push_back(m*nCell);
					indices.push_back(m*(nCell - gap) + gap);
					indices.push_back(m*nCell + gap);

					indices.push_back(nCell - gap);
					indices.push_back(nCell);
					indices.push_back(m * gap + nCell - gap);

					indices.push_back(m*(nCell - gap) + nCell - gap);
					indices.push_back(m*nCell + nCell);
					indices.push_back(m*nCell + nCell - gap);
					break;

				case SLOD_MARGIN_LEFT_TOP:
					leftMarginColIdx = gap;
					rightMarginColIdx = 0;
					topMarginRowIdx = gap;
					botMarginRowIdx = 0;

					indices.push_back(nCell);
					indices.push_back(m*gap + nCell);
					indices.push_back(m*gap + nCell - gap);


					indices.push_back(m*nCell);
					indices.push_back(m*(nCell - gap) + gap);
					indices.push_back(m*nCell + gap);
					break;

				case SLOD_MARGIN_LEFT_BOTTOM:
					leftMarginColIdx = gap;
					rightMarginColIdx = 0;
					topMarginRowIdx = 0;
					botMarginRowIdx = gap;

					indices.push_back(0);
					indices.push_back(gap);
					indices.push_back(m *gap + gap);

					indices.push_back(m*nCell + nCell);
					indices.push_back(m*(nCell - gap) + nCell - gap);
					indices.push_back(m*(nCell - gap) + nCell);
					break;

				case SLOD_MARGIN_RIGHT_BOTTOM:
					leftMarginColIdx = 0;
					rightMarginColIdx = gap;
					topMarginRowIdx = 0;
					botMarginRowIdx = gap;

					indices.push_back(nCell - gap);
					indices.push_back(nCell);
					indices.push_back(m * gap + nCell - gap);

					indices.push_back(m*nCell);
					indices.push_back(m*(nCell - gap));
					indices.push_back(m*(nCell - gap) + gap);
					break;

				case SLOD_MARGIN_TOP_RIGHT:
					leftMarginColIdx = 0;
					rightMarginColIdx = gap;
					topMarginRowIdx = gap;
					botMarginRowIdx = 0;

					indices.push_back(0);
					indices.push_back(m *gap + gap);
					indices.push_back(m * gap);

					indices.push_back(m*(nCell - gap) + nCell - gap);
					indices.push_back(m*nCell + nCell);
					indices.push_back(m*nCell + nCell - gap);
					break;

				case SLOD_MARGIN_LEFT_TOP_RIGHT:
					leftMarginColIdx = gap;
					rightMarginColIdx = gap;
					topMarginRowIdx = gap;
					botMarginRowIdx = 0;

					indices.push_back(m*nCell);
					indices.push_back(m*(nCell - gap) + gap);
					indices.push_back(m*nCell + gap);

					indices.push_back(m*(nCell - gap) + nCell - gap);
					indices.push_back(m*nCell + nCell);
					indices.push_back(m*nCell + nCell - gap);
					break;

				case SLOD_MARGIN_LEFT_RIGHT_BOTTOM:
					leftMarginColIdx = gap;
					rightMarginColIdx = gap;
					topMarginRowIdx = 0;
					botMarginRowIdx = gap;

					indices.push_back(0);
					indices.push_back(gap);
					indices.push_back(m *gap + gap);

					indices.push_back(nCell - gap);
					indices.push_back(nCell);
					indices.push_back(m * gap + nCell - gap);
					break;

				case SLOD_MARGIN_TOP_RIGHT_BOTTOM:
					leftMarginColIdx = 0;
					rightMarginColIdx = gap;
					topMarginRowIdx = gap;
					botMarginRowIdx = gap;

					indices.push_back(0);
					indices.push_back(m *gap + gap);
					indices.push_back(m * gap);

					indices.push_back(m*nCell);
					indices.push_back(m*(nCell - gap));
					indices.push_back(m*(nCell - gap) + gap);

					break;

				case SLOD_MARGIN_LEFT_TOP_BOTTOM:
					leftMarginColIdx = gap;
					rightMarginColIdx = 0;
					topMarginRowIdx = gap;
					botMarginRowIdx = gap;

					indices.push_back(nCell);
					indices.push_back(m*gap + nCell);
					indices.push_back(m*gap + nCell - gap);

					indices.push_back(m*nCell + nCell);
					indices.push_back(m*(nCell - gap) + nCell - gap);
					indices.push_back(m*(nCell - gap) + nCell);
					break;


				}
			}
			for (int i = topMarginRowIdx; i < nCell - botMarginRowIdx; i += gap)
			{
				for (int j = leftMarginColIdx; j < nCell - rightMarginColIdx; j += gap)
				{
					if (i < nCell / 2) {
						if (j < nCell / 2) {
							indices.push_back(i*(nCell + 1) + j);
							indices.push_back((i + gap)*(nCell + 1) + j + gap);
							indices.push_back((i + gap)*(nCell + 1) + j);

							indices.push_back(i*(nCell + 1) + j);
							indices.push_back(i*(nCell + 1) + j + gap);
							indices.push_back((i + gap)*(nCell + 1) + j + gap);
						}

						else
						{
							indices.push_back(i*(nCell + 1) + j);
							indices.push_back(i*(nCell + 1) + j + gap);
							indices.push_back((i + gap)*(nCell + 1) + j);

							indices.push_back(i*(nCell + 1) + j + gap);
							indices.push_back((i + gap)*(nCell + 1) + j + gap);
							indices.push_back((i + gap)*(nCell + 1) + j);
						}
					}
					else
					{
						if (j < nCell / 2) {
							indices.push_back(i*(nCell + 1) + j);
							indices.push_back(i*(nCell + 1) + j + gap);
							indices.push_back((i + gap)*(nCell + 1) + j);

							indices.push_back(i*(nCell + 1) + j + gap);
							indices.push_back((i + gap)*(nCell + 1) + j + gap);
							indices.push_back((i + gap)*(nCell + 1) + j);
						}

						else
						{
							indices.push_back(i*(nCell + 1) + j);
							indices.push_back((i + gap)*(nCell + 1) + j + gap);
							indices.push_back((i + gap)*(nCell + 1) + j);

							indices.push_back(i*(nCell + 1) + j);
							indices.push_back(i*(nCell + 1) + j + gap);
							indices.push_back((i + gap)*(nCell + 1) + j + gap);
						}

					}
				}
			}
			

			D3D11_BUFFER_DESC ibd;
			ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
			ibd.ByteWidth = sizeof(UINT)* indices.size();
			ibd.CPUAccessFlags = 0;
			ibd.MiscFlags = 0;
			ibd.StructureByteStride = 0;
			ibd.Usage = D3D11_USAGE_IMMUTABLE;

			D3D11_SUBRESOURCE_DATA idata;
			idata.pSysMem = &indices[0];

			ID3D11Buffer* buf;
			HR(device->CreateBuffer(&ibd, &idata, &buf));

			bufs.push_back(buf);
		}
		mIBs.push_back(bufs);
				
		vector<ID3D11Buffer*> topBufs, botBufs, rightBufs, leftBufs;
		for (int subLevel = 0; subLevel < mIndexBufferLevelCount - k + 1; subLevel++) {

			ID3D11Buffer *topBuf, *rightBuf, *botBuf, *leftBuf;
			vector<UINT> edgeIndics;
			int subLevelGap = pow(2, subLevel + 1);

			for (int j = 0; j < nCell; j += gap * subLevelGap)
			{
				edgeIndics.push_back(j);
				edgeIndics.push_back(j + gap * subLevelGap);
				edgeIndics.push_back(gap*m + j + gap * subLevelGap / 2);

				for (int x = 0; x < gap*subLevelGap / 2; x += gap)
				{
					if (x == 0 && j == 0) continue;
					edgeIndics.push_back(j);
					edgeIndics.push_back(gap*m + j + gap + x);
					edgeIndics.push_back(gap*m + j + x);
				}
				for (int x = 0; x < gap*subLevelGap / 2; x += gap)
				{

					if (x + gap >= gap*subLevelGap / 2 && j + gap*subLevelGap >= nCell) continue;
					edgeIndics.push_back(gap*m + j + gap * subLevelGap / 2 + x);
					edgeIndics.push_back(j + gap * subLevelGap);
					edgeIndics.push_back(gap*m + j + gap * subLevelGap / 2 + gap + x);
				}
			}

			D3D11_BUFFER_DESC ibd;
			ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
			ibd.ByteWidth = sizeof(UINT)* edgeIndics.size();
			ibd.CPUAccessFlags = 0;
			ibd.MiscFlags = 0;
			ibd.StructureByteStride = 0;
			ibd.Usage = D3D11_USAGE_IMMUTABLE;

			D3D11_SUBRESOURCE_DATA idata;
			idata.pSysMem = &edgeIndics[0];

			HR(device->CreateBuffer(&ibd, &idata, &topBuf));

			topBufs.push_back(topBuf);
			edgeIndics.clear();

			for (int j = 0; j < nCell; j += gap * subLevelGap)
			{
				edgeIndics.push_back(j + m*nCell);
				edgeIndics.push_back(j + gap * subLevelGap / 2 + m*(nCell - gap));
				edgeIndics.push_back(nCell*m + j + gap * subLevelGap);

				for (int x = 0; x < gap*subLevelGap / 2; x += gap)
				{

					if (x == 0 && j == 0) continue;
					edgeIndics.push_back(j + m*(nCell - gap) + x);
					edgeIndics.push_back(j + m*(nCell - gap) + gap + x);
					edgeIndics.push_back(j + m*nCell);
				}
				for (int x = 0; x < gap*subLevelGap / 2; x += gap)
				{

					if (x + gap >= gap*subLevelGap / 2 && j + gap*subLevelGap >= nCell) continue;
					edgeIndics.push_back(j + gap * subLevelGap / 2 + m*(nCell - gap) + x);
					edgeIndics.push_back(j + gap * subLevelGap / 2 + m*(nCell - gap) + gap + x);
					edgeIndics.push_back(nCell*m + j + gap * subLevelGap);
				}
			}

			idata.pSysMem = &edgeIndics[0];

			HR(device->CreateBuffer(&ibd, &idata, &botBuf));

			botBufs.push_back(botBuf);
			edgeIndics.clear();


			for (int j = 0; j < nCell; j += gap * subLevelGap)
			{
				edgeIndics.push_back(j*m);
				edgeIndics.push_back(j*m + gap + m*gap* subLevelGap / 2);
				edgeIndics.push_back(j*m + m*gap * subLevelGap);

				for (int x = 0; x < gap*subLevelGap / 2; x += gap)
				{

					if (x == 0 && j == 0) continue;
					edgeIndics.push_back(j*m);
					edgeIndics.push_back((j + x)*m + gap);
					edgeIndics.push_back((j + x + gap)*m + gap);
				}
				for (int x = 0; x < gap*subLevelGap / 2; x += gap)
				{

					if (x + gap >= gap*subLevelGap / 2 && j + gap*subLevelGap >= nCell) continue;
					edgeIndics.push_back((j + x)*m + gap + m*gap* subLevelGap / 2);
					edgeIndics.push_back((j + gap + x)*m + gap + m*gap* subLevelGap / 2);
					edgeIndics.push_back(j*m + m*gap * subLevelGap);
				}
			}

			idata.pSysMem = &edgeIndics[0];

			HR(device->CreateBuffer(&ibd, &idata, &leftBuf));

			leftBufs.push_back(leftBuf);
			edgeIndics.clear();

			for (int j = 0; j < nCell; j += gap * subLevelGap)
			{
				edgeIndics.push_back(nCell + (j + gap * subLevelGap / 2)*m - gap);
				edgeIndics.push_back(nCell + j*m);
				edgeIndics.push_back(nCell + (j + gap * subLevelGap)*m);
				for (int x = 0; x < gap*subLevelGap / 2; x += gap)
				{

					if (x == 0 && j == 0) continue;
					edgeIndics.push_back(nCell + (j+x)*m - gap);
					edgeIndics.push_back(nCell + j*m);
					edgeIndics.push_back(nCell + (j + x + gap)*m - gap);
				}
				for (int x = 0; x < gap*subLevelGap / 2; x += gap)
				{

					if (x + gap>= gap*subLevelGap / 2 && j + gap*subLevelGap >= nCell) continue;

					edgeIndics.push_back(nCell + (j + gap * subLevelGap / 2 + x)*m - gap);
					edgeIndics.push_back(nCell + (j + gap * subLevelGap)*m);
					edgeIndics.push_back(nCell + (j + gap * subLevelGap / 2 + x + gap)*m - gap);
				}
			}
			idata.pSysMem = &edgeIndics[0];

			HR(device->CreateBuffer(&ibd, &idata, &rightBuf));

			rightBufs.push_back(rightBuf);
			edgeIndics.clear();

		}
		topBufss.push_back(topBufs);
		botBufss.push_back(botBufs);
		rightBufss.push_back(rightBufs);
		leftBufss.push_back(leftBufs);

	}
	
	mEdgeIBs.resize(4);
	mEdgeIBs[QuadTree::NEIGHBOR_LEFT] = leftBufss;
	mEdgeIBs[QuadTree::NEIGHBOR_TOP] = topBufss;
	mEdgeIBs[QuadTree::NEIGHBOR_RIGHT] = rightBufss;
	mEdgeIBs[QuadTree::NEIGHBOR_BOTTOM] = botBufss;


}
/**
@brief 지형에 스무딩 적용
@param vIndices : 스무딩을 적용할 정점 리스트
*/
void Terrain::Smooth(vector<int>& vIndices)
{

	float guassianFilter[5] = { 0.0545f, 0.2442f, 0.4026f, 0.2442f, 0.0545f };
	
	vector<float> temp;
	vector<int> off;
	
	for (int k = 0; k < vIndices.size(); k++)
	{
		float sum = 0;
		for (int i = -2; i <= 2; i++)
		{
			if (vIndices[k] + i >= 0 && vIndices[k] + i < mVertices.size()) {
				sum += mVertices[vIndices[k] + i].Position.y * guassianFilter[i+2];
			}		
		}

		temp.push_back(sum);
		off.push_back(vIndices[k]);
	}

	for (int i = 0; i < off.size(); i++)
	{
		mVertices[off[i]].Position.y = temp[i];
	}

	temp.clear();
	off.clear();


	for (int k = 0; k < vIndices.size(); k++)
	{
		float sum = 0;
	
		for (int i = -2; i <= 2; i++)
		{
			int l = mColCount * i;
			if (vIndices[k] + l >= 0 && vIndices[k] + l < mVertices.size()) {
				sum += mVertices[vIndices[k] + l].Position.y * guassianFilter[i + 2];
			}
		}
		temp.push_back(sum);
		off.push_back(vIndices[k]);
	}
	for (int i = 0; i < off.size(); i++)
	{
		mVertices[off[i]].Position.y = temp[i];
	}
	
	

	/*
	for (int k = 0; k < vIndices.size(); k++) {
		float sum = 0, n = 0;
		for (int i = -1; i <= 1; i++)
		{
			for (int j = -1; j <= 1; j++)
			{
				int l = mColCount * i + j;
				if (vIndices[k] + l >= 0 && vIndices[k] + l < mVertices.size() && l != 0) {
					sum += mVertices[vIndices[k] + l].Position.y;
					n++;
				}
			}
		}

		if (n != 0) {
			sum /= n;
			sum = (mVertices[vIndices[k]].Position.y + sum) / 2.0f;
			temp.push_back(sum);
			off.push_back(vIndices[k]);
		}
	}
	
	for (int i = 0; i < off.size(); i++)
	{
		mVertices[off[i]].Position.y = temp[i];
	}
	*/

}
/**
@brief sphere에 속하는 정점의 높이를 변경.
@param s : 선택 구
@param f : 기존 정점 높이에 더할 값(음수일 수도있음)
@param bFlat : 지형을 평지로 만들 것인지에 대한 판별 변수
@remark bFlat이 True일 경우 f만큼의 크기를 사용해서 점차적으로 정점의 높이를 0으로 변경시킴
*/
void Terrain::AddHeightInShape(Collider::Sphere s, float f, bool bFlat, bool bSmooth)
{
	int st, et;
	vector<int> vbIndex; // 높이값이 수정된 정점 버퍼 인덱스 리스트
	vector<bool> bChange(mVertices.size()); // 타일의 경계에 위치한 정점들은 여러번 수정될 수 있으므로, 중복 적용 여부 체크

	mQuadTree.AddHeightInShape(s, mVertices, f, vbIndex, bChange,bFlat);



	bChange.assign(bChange.size(), false);
	

	vector<int> vIndices; // 수정된 정점 리스트

	mQuadTree.FindVertexIndexInShape(s, mVertices, vIndices, bChange); // 수정된 정점들의 리스트 반환

	if(bSmooth) Smooth(vIndices); // 스무딩 적용
	UpdateNormal(vIndices); // 정점 노멀 업데이트
	
	// 정점 버퍼 갱신
	for (int i = 0; i < vbIndex.size(); i++) {
		D3D11_MAPPED_SUBRESOURCE data;
		int vbOff = vbIndex[i];
		mDC->Map(mVBs[vbOff], 0, D3D11_MAP_WRITE_DISCARD, 0,&data);

		Vertex::PosNorTex0Tex1Tan* v = (Vertex::PosNorTex0Tex1Tan*)data.pData;

		
		for (int z = 0; z < mCellCount + 1; z++)
		{
			for (int x = 0; x < mCellCount + 1; x++)
			{
				int tileCountOfLine = (mLineVertexCount - 1) / mCellCount;
				int line = vbOff / tileCountOfLine;
				int TL = line * mLineVertexCount * mCellCount + (vbOff % tileCountOfLine) * mCellCount; // 타일의 시작 정점 인덱스
				int off = TL + z * mLineVertexCount + x;
				*v = mVertices[off];
				v++;
			}
		}
		mDC->Unmap(mVBs[i], 0);
	}


}

/**
@brief x,z에 위치한 정점의 높이를 반환
@param x : x 좌표
@param z : z 좌표
*/
float Terrain::GetHeight(float x, float z)
{

	if (std::fabsf(x) >= mWidth / 2.0f || std::fabsf(z) >= mDepth / 2.0f) // 지형 밖일 경우 리턴
		return 0.0f;
	float y, c, d;
	float TL, TR, BL, BR;
	y = 0.0f;


	c = (x + 0.5* mWidth) / mCellSpaceX; // -width/2 ~ width/2에 위치한 x좌표를 0~width로 변경하고 정점 간격을 1로 바꿈  
	d = (z - 0.5* mDepth) / -mCellSpaceZ; // -depth/2 ~ depth/2에 위치한 z좌표를 0~depth로 변경하고 정점 간격을 1로 바꿈 


	int row = (int)floorf(d); // 정점의 행
	int col = (int)floorf(c); // 정점의 열


	TL = mVertices[row*mLineVertexCount + col].Position.y; // 정점에 가장 가까운 좌상단 정점 높이
	TR = mVertices[row*mLineVertexCount + col + 1].Position.y; // 정점에 가장 가까운 우상단 정점 높이
	BL = mVertices[(row + 1)*mLineVertexCount + col].Position.y; // 정점에 가장 가까운 좌하단 정점 높이
	BR = mVertices[(row + 1)*mLineVertexCount + col + 1].Position.y; // 정점에 가장 가까운 우하단 정점 높이


	float s = c - (float)col; // TL 에서 얼마나 오른쪽으로 갔는지 
	float t = d - (float)row; // TL 에서 얼마나 밑으로 갔는지


	// 위쪽 삼각형에 포함되어 있는지 아래쪽 삼각형에 포함되어 있는지 판별
	if ((row % mCellCount) < mCellCount / 2) {

		if ((col % mCellCount) < mCellCount /2)
		{

			if ((1 - t + s) <= 1.0f)
			{
				float uy = BR - BL;
				float vy = TL - BL;

				y = BL + s * uy + (1.0f - t) * vy;
			}
			else
			{

				float uy = TL - TR;
				float vy = BR - TR;

				y = TR + (1.0f-s) * uy + t * vy;
			}

		}
		else
		{
			if (s + t <= 1.0f) {
				float uy = TR - TL;
				float vy = BL - TL;

				y = TL + s *uy + t*vy;

			}
			else
			{
				float uy = BL - BR;
				float vy = TR - BR;

				y = BR + (1.0f - s)*uy + (1.0f - t)*vy;

			}
		}
	}
	else {

		if ((col % mCellCount) > mCellCount / 2)
		{
			if ((1 - t + s) <= 1.0f)
			{
				float uy = BR - BL;
				float vy = TL - BL;

				y = BL + s * uy + (1.0f - t) * vy;
			}
			else
			{

				float uy = TL - TR;
				float vy = BR - TR;

				y = TR + (1.0f - s) * uy + t * vy;
			}

		}
		else
		{
			if (s + t <= 1.0f) {
				float uy = TR - TL;
				float vy = BL - TL;

				y = TL + s *uy + t*vy;

			}
			else
			{
				float uy = BL - BR;
				float vy = TR - BR;

				y = BR + (1.0f - s)*uy + (1.0f - t)*vy;

			}
		}

	}




	return y;
}




void Terrain::FindVertexIndexInShape(Collider::Sphere s, vector<int>& vIndices)
{

	vector<bool> bChange(mVertices.size());
	mQuadTree.FindVertexIndexInShape(s,mVertices,vIndices, bChange);
}


void Terrain::UpdateNormal(vector<int>& vIndices)
{

	float left, right, top, bot;
	XMVECTOR normal, tangent, bitan;
	for (int k = 0; k < vIndices.size(); k++)
	{
		left = right = top = bot = 1.0f;
		int vIndex = vIndices[k];

		if(vIndex -1 >= 0 && vIndex % mColCount != 0) left = mVertices[vIndex - 1].Position.y;
		if(vIndex + 1 < mColCount* mRowCount && (vIndex+1)%mColCount != 0) right = mVertices[vIndex + 1].Position.y;
		if (vIndex >= mColCount) top = mVertices[vIndex - mColCount].Position.y;
		if (vIndex < mColCount * (mRowCount - 1)) bot = mVertices[vIndex + mColCount].Position.y;
		
		tangent = XMVector3Normalize(XMVectorSet(2.0f*mCellSpaceX, right - left, 0.0f,0.0f));
		bitan = XMVector3Normalize(XMVectorSet(0.0f, top - bot, 2.0*mCellSpaceZ, 0.0f));
		normal = XMVector3Cross(bitan, tangent);
		XMStoreFloat3(&mVertices[vIndex].Normal, normal);
	
	}

}

void Terrain::LoadHeightMap(TCHAR* filePath, int width, int depth, int height, int m, int format, bool bFlipVertical, bool bSignAbility)
{

	std::ifstream in(filePath, std::ios_base::binary);

	if (in)
	{

		vector<int> vIndices;
		// 16bit
		if (format == 16) {
			in.seekg(0, std::ios_base::end);
			int size = in.tellg();
			in.seekg(0, std::ios_base::beg);

			std::vector<unsigned char> heightMap(size);

			in.read((char*)&heightMap[0], size);
			in.close();

			Init(width, height, depth, m, m, 32, mDevice, mDC);
			for (int i = 0; i < size; i += 2)
			{
				if (bSignAbility) {
					int j;
					int sign = 1;
					if (heightMap.size() <= i) break;
					if (mVertices.size() <= i / 2) break;
					if (heightMap[i + 1] & 0x80) sign = -1;

					mVertices[i / 2].Position.y = sign *(((heightMap[i + 1] & 0x7f) << 0x08) | heightMap[i]) / 65535.0f*height * 2.0f;
				}
				else
				{

					mVertices[i / 2].Position.y = ((heightMap[i + 1] << 0x08) | heightMap[i]) / 65535.0f*height;
				}
			}

			vIndices.resize(size / 2);
			for (int i = 0; i < size / 2; i++) vIndices[i] = i;
			
		}
		// 8bit
		else {
			in.seekg(0, std::ios_base::end);
			int size = in.tellg();
			in.seekg(0, std::ios_base::beg);

			std::vector<unsigned char> heightMap(size);

			in.read((char*)&heightMap[0], size);
			in.close();




			Init(width, height,depth, m, m, 32, mDevice, mDC);



			for (int i = 0; i < mVertices.size(); i++)
			{

				if (bSignAbility) {
					int j;
					int sign = 1;

					if (heightMap.size() <= i) break;
					if (mVertices.size() <= i) break;
					if (heightMap[i] & 0x80) sign = -1;

					mVertices[i].Position.y = sign *(heightMap[i] & 0x7f) / 255.0f * height *2.0f;
				}
				else {
					mVertices[i].Position.y = heightMap[i] / 255.0f * height;
				}
			}

			vIndices.resize(mVertices.size());
			for (int i = 0; i < mVertices.size(); i++) vIndices[i] = i;
			
		}

		if (bFlipVertical)
		{
			vector<Vertex::PosNorTex0Tex1Tan> temp(mVertices);
			for (int z = 0; z < m; z++)
			{
				for (int x = 0; x < m; x++)
				{
					mVertices[z*m + x].Position.y = temp[z*m + m - x - 1].Position.y;
				}
			}
		}
		UpdateNormal(vIndices);
		for (int k = 0; k < mVBs.size(); k++) {
			D3D11_MAPPED_SUBRESOURCE data;
			mDC->Map(mVBs[k], 0, D3D11_MAP_WRITE_DISCARD, 0, &data);

			Vertex::PosNorTex0Tex1Tan* v = (Vertex::PosNorTex0Tex1Tan*)data.pData;


			for (int z = 0; z < mCellCount + 1; z++)
			{
				for (int x = 0; x < mCellCount + 1; x++)
				{

					int tileCountOfLine = (mLineVertexCount - 1) / mCellCount;
					int line = k / tileCountOfLine;
					int TL = line * mLineVertexCount * mCellCount + (k % tileCountOfLine) * mCellCount; // 타일의 시작 정점 인덱스
					int off = TL + z * mLineVertexCount + x;
					*v = mVertices[off];
					v++;
				}
			}
			mDC->Unmap(mVBs[k], 0);
		}

	}


}

void Terrain::SaveHeightMapTo8bitRaw(TCHAR* filePath, int format)
{
	std::ofstream out(filePath, std::ios_base::binary);


	if (out)
	{

		// 16bit
		if (format == 16) {


			std::vector<unsigned char> heightMap(mVertices.size() * 2);

			unsigned char one, two;
			for (int i = 0; i < mVertices.size(); i++)
			{

				float y = fabsf(mVertices[i].Position.y);
				unsigned short data = y / mHeight * 65535.0f/2;
				data &= 0x7fff;
				int sign = 1;
				if (mVertices[i].Position.y < 0) sign = -1;

				one = (data & 0xff00) >> 0x08;
				two = (data & 0xff);
				if (sign == -1) 
					one |= 0x80;
				heightMap[i * 2] = two;
				heightMap[i * 2 + 1] = one;

			}

			out.write((char*)&heightMap[0], heightMap.size());
		}	
		else if (format == 8) {


			std::vector<unsigned char> heightMap(mVertices.size());

			for (int i = 0; i < mVertices.size(); i++)
			{

				float y = fabsf(mVertices[i].Position.y);
				unsigned char data = y / mHeight * 255.0f / 2;
				data &= 0x7fff;
				int sign = 1;
				if (mVertices[i].Position.y < 0) sign = -1;

				if (sign == -1)	data |= 0x80;
				heightMap[i] = data;

			}

			out.write((char*)&heightMap[0], heightMap.size());
		}
	}
		
}

ID3D11Texture2D* Terrain::GetNormalMap()
{
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));

	mNormalTex->GetDesc(&desc);
	D3D11_MAPPED_SUBRESOURCE data;
	mDC->Map(mNormalTex, 0, D3D11_MAP_WRITE_DISCARD, 0, &data);

	UCHAR* pTexel = (UCHAR*)data.pData;


	for (UINT z = 0; z < desc.Height; z++)
	{
		for (UINT x = 0; x < desc.Width; x++)
		{
			pTexel[z*data.RowPitch + x * 4 + 0] = (mVertices[z*mRowCount+x].Normal.x * 0.5 + 0.5) * 255; // R
			pTexel[z*data.RowPitch + x * 4 + 1] = (mVertices[z*mRowCount + x].Normal.y* 0.5 + 0.5) * 255; // G
			pTexel[z*data.RowPitch + x * 4 + 2] = (mVertices[z*mRowCount + x].Normal.z* 0.5 + 0.5) * 255; // B
			pTexel[z*data.RowPitch + x * 4 + 3] = 0.0f; // A
		
		}
	}

	mDC->Unmap(mNormalTex, 0);

	return mNormalTex;
}