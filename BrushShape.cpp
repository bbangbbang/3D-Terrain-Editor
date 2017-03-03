#include "BrushShape.h"
#include "MathHelper.h"

BrushShape::BrushShape()
	: mCircleVB(0), mCenter(0.0f, 0.0f, 0.0f), mTerrain(0),
	mRadius(0.0f), mTheta(0.0f), mCircleVertexCount(0), mCircleVertices(0)
{

}

void BrushShape::UpdateCircleVertices()
{
	mCircleVertices.clear();


	for (float d = 0.0f; d <= MathHelper::PI * 2.0; d += mTheta)
	{
		XMFLOAT3 pos;
		pos.x = mCenter.x + mRadius * cosf(d);
		pos.z = mCenter.z + mRadius * sinf(d);
		if (mTerrain)
			pos.y = mTerrain->GetHeight(pos.x, pos.z);
		else
			pos.y = mCenter.y;


		XMFLOAT4 color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

		Vertex::PosColor v(pos, color);

		mCircleVertices.push_back(v);
	}

	mCircleVertexCount = mCircleVertices.size();

	
}

void BrushShape::SetCenter(XMVECTOR center)
{
	XMStoreFloat3(&mCenter,center);
}

void BrushShape::SetRadius(float radius)
{
	mRadius = radius;
}

void BrushShape::SetTheta(float theta)
{
	mTheta = theta;
}
BrushShape::~BrushShape()
{
	ReleaseCOM(mCircleVB);
}


void BrushShape::InitCircle(ID3D11Device* device, XMVECTOR c, float r, float t)
{

	XMStoreFloat3(&mCenter, c);
	mRadius = r;
	mTheta = t;


	UpdateCircleVertices();
	D3D11_BUFFER_DESC vdesc;
	vdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vdesc.ByteWidth = mCircleVertexCount * sizeof(Vertex::PosColor);
	vdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vdesc.MiscFlags = 0;
	vdesc.StructureByteStride = 0;
	vdesc.Usage = D3D11_USAGE_DYNAMIC;

	HR(device->CreateBuffer(&vdesc, 0, &mCircleVB));

	

}



void BrushShape::DrawCircle(ID3D11DeviceContext* dc, ID3DX11EffectTechnique* tech)
{
	
	UpdateCircleVertices();
	D3D11_MAPPED_SUBRESOURCE data;
	dc->Map(mCircleVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &data);
	Vertex::PosColor *v = (Vertex::PosColor*)data.pData;
	for (UINT i = 0; i < mCircleVertexCount; i++, v++){
		*v = mCircleVertices[i];
		v->Position.y += 0.5f;
		
	}
	
	dc->Unmap(mCircleVB, 0);
	

	dc->IASetInputLayout(InputLayouts::PosColor);
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
	UINT stride = sizeof(Vertex::PosColor);
	UINT offset = 0;
	
	dc->IASetVertexBuffers(0, 1, &mCircleVB, &stride, &offset);

	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	
	for (UINT p = 0; p < techDesc.Passes; p++) {

		tech->GetPassByIndex(p)->Apply(0, dc);
		dc->Draw(mCircleVertexCount, 0);
	}
	

}

XMVECTOR BrushShape::GetCenter()
{
	return XMLoadFloat3(&mCenter);
}

float BrushShape::GetRadius()
{
	return mRadius;
}

float BrushShape::GetTheta()
{
	return mTheta;
}

void BrushShape::EnableTerrainHeight(Terrain *t)
{
	mTerrain = t;
}

void BrushShape::UnableTerrainHeight()
{
	mTerrain = 0;
}