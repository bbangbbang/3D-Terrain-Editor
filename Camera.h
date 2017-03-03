#pragma once
#include "d3dUtil.h"
class Camera
{
public:

	Camera();
	~Camera();

	void	SetLookAt(XMVECTOR pos, XMVECTOR target, XMVECTOR upVec);
	void	SetLens(float fovAngleY, float AspectRatio, float NearZ, float FarZ);
	void	SetPosition(float x, float y, float z);


	void	UpdateViewMatrix();

	void	Walk(float d);
	void	MoveUp(float d);
	void	MoveRight(float d);

	void	Yaw(float rad);
	void	Pitch(float rad);
	void	Roll(float rad);

	XMVECTOR	GetPosition();
	XMFLOAT3	GetPositionF();
	XMMATRIX	GetViewMatrix();
	XMMATRIX	GetProjMatrix();

	

private:

	XMFLOAT3 mPos;
	XMFLOAT3 mDir;
	XMFLOAT3 mRight;
	XMFLOAT3 mUp;

	XMFLOAT4X4 mProj;
	XMFLOAT4X4 mView;


};