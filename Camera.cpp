#include "Camera.h"

Camera::Camera()
: mPos(0.0f, 0.0f, 0.0f), mDir(0.0f, 0.0f, 1.0f)
, mRight(1.0f, 0.0f, 0.0f), mUp(0.0f, 1.0f, 0.0f)
{
	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);
}

Camera::~Camera()
{

}

void Camera::SetLookAt(XMVECTOR pos, XMVECTOR target, XMVECTOR upVec)
{
	XMVECTOR dir = XMVector3Normalize(XMVectorSubtract(target,pos));
	XMVECTOR right = XMVector3Normalize(XMVector3Cross(upVec, dir));
	XMVECTOR up = XMVector3Cross(dir, right);


	XMStoreFloat3(&mPos, pos);
	XMStoreFloat3(&mDir, dir);
	XMStoreFloat3(&mRight, right);
	XMStoreFloat3(&mUp, up);
}

void Camera::SetLens(float fovAngleY, float AspectRatio, float NearZ, float FarZ)
{
	XMMATRIX P = XMMatrixPerspectiveFovLH(fovAngleY, AspectRatio, NearZ, FarZ);

	XMStoreFloat4x4(&mProj, P);
}

void Camera::SetPosition(float x, float y, float z)
{
	mPos = XMFLOAT3(x, y, z);
}

XMMATRIX Camera::GetViewMatrix()
{
	return XMLoadFloat4x4(&mView);
}

XMMATRIX Camera::GetProjMatrix()
{
	return XMLoadFloat4x4(&mProj);
}

void Camera::Walk(float d)
{

	XMVECTOR P = XMLoadFloat3(&mPos);
	XMVECTOR Dir = XMLoadFloat3(&mDir);

	XMStoreFloat3(&mPos, P + d * Dir);
}

void Camera::MoveUp(float d)
{

	XMVECTOR P = XMLoadFloat3(&mPos);
	XMVECTOR Dir = XMLoadFloat3(&mUp);

	XMStoreFloat3(&mPos, P + d * Dir);
}
void Camera::MoveRight(float d)
{

	XMVECTOR P = XMLoadFloat3(&mPos);
	XMVECTOR Dir = XMLoadFloat3(&mRight);

	XMStoreFloat3(&mPos, P + d * Dir);
}

void Camera::Yaw(float rad)
{
	
	XMMATRIX M = XMMatrixRotationY(rad);
	XMVECTOR up = XMVectorSet(0.0f,1.0f,0.0f,0.0f);
	XMStoreFloat3(&mRight, XMVector3TransformNormal(XMLoadFloat3(&mRight), M));
	XMStoreFloat3(&mUp, XMVector3TransformNormal(XMLoadFloat3(&mUp), M));
	XMStoreFloat3(&mDir, XMVector3TransformNormal(XMLoadFloat3(&mDir), M)); 
}

void Camera::Pitch(float rad)
{

	XMVECTOR R = XMLoadFloat3(&mRight);
	XMMATRIX M = XMMatrixRotationAxis(R, rad);


	XMStoreFloat3(&mUp, XMVector3TransformNormal(XMLoadFloat3(&mUp), M));
	XMStoreFloat3(&mDir, XMVector3TransformNormal(XMLoadFloat3(&mDir), M));


}


void Camera::Roll(float rad)
{

}

void Camera::UpdateViewMatrix()
{
	
	XMVECTOR P = XMLoadFloat3(&mPos);
	XMVECTOR R = XMLoadFloat3(&mRight);
	XMVECTOR U = XMLoadFloat3(&mUp);
	XMVECTOR D = XMLoadFloat3(&mDir);



	mView._11 = mRight.x;
	mView._21 = mRight.y;
	mView._31 = mRight.z;
	mView._41 = -XMVectorGetX(XMVector3Dot(P, R));

	mView._12 = mUp.x;
	mView._22 = mUp.y;
	mView._32 = mUp.z;
	mView._42 = -XMVectorGetX(XMVector3Dot(P, U));

	mView._13 = mDir.x;
	mView._23 = mDir.y;
	mView._33 = mDir.z;
	mView._43 = -XMVectorGetX(XMVector3Dot(P, D));

	mView._14 = 0.0f;
	mView._24 = 0.0f;
	mView._34 = 0.0f;
	mView._44 = 1.0f;

}


XMVECTOR Camera::GetPosition()
{
	return XMLoadFloat3(&mPos);
}

XMFLOAT3 Camera::GetPositionF()
{
	return mPos;
}