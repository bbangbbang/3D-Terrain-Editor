
#include "Collider.h"

float Collider::EPSILON = 0.0f;
/**
@brief View ��İ� Projection ����� ���� World ������ ī�޶� ����ü ���
@param M : View Matrix * Projection Matrix
@return ī�޶� ����ü ������ ���� ����ü ��ȯ
*/
Collider::Frustum Collider::ComputeFrustumFromViewProj(CXMMATRIX M)
{
	/*
	Frustum frustum; // ����ü �� 6���� ��� ����ü

	// NDC ���������� ī�޶� ����ü ���� 8�� ��ǥ
	XMVECTOR vertices[8] = 
	{
		{-1.0f, +1.0f, +0.0f, +1.0f},
		{+1.0f, +1.0f, +0.0f, +1.0f},
		{+1.0f, -1.0f, +0.0f, +1.0f},
		{-1.0f, -1.0f, +0.0f, +1.0f},
		{-1.0f, +1.0f, +1.0f, +1.0f},
		{+1.0f, +1.0f, +1.0f, +1.0f},
		{+1.0f, -1.0f, +1.0f, +1.0f},
		{-1.0f, -1.0f, +1.0f, +1.0f},
	};
	
	XMVECTOR det;
	XMMATRIX invM = XMMatrixInverse(&det, M); // View * Proj�� �����

	// NDC ���������� ī�޶� ����ü ���� 8���� ���� ������ ��ǥ�� ��ȯ
	for (int i = 0; i < 8; i++)
	{

		vertices[i] = XMVector3Transform(vertices[i], invM);
		vertices[i] = vertices[i] * XMVectorReciprocal(XMVectorSplatW(vertices[i]));
	}

	// ���� ������ ���� 8���� �̿��Ͽ� ����ü�� �̷�� ��� 6���� ��� �Ͽ� ����
	//ax + by + cz + D ���� ���� ����(a,b,c) �� D ����
	XMStoreFloat4(&frustum.Planes[0], XMPlaneFromPoints(vertices[0], vertices[4], vertices[7]));
	XMStoreFloat4(&frustum.Planes[1], XMPlaneFromPoints(vertices[0], vertices[1], vertices[5]));
	XMStoreFloat4(&frustum.Planes[2], XMPlaneFromPoints(vertices[1], vertices[6], vertices[5]));
	XMStoreFloat4(&frustum.Planes[3], XMPlaneFromPoints(vertices[2], vertices[3], vertices[7]));
	XMStoreFloat4(&frustum.Planes[4], XMPlaneFromPoints(vertices[0], vertices[3], vertices[2]));
	XMStoreFloat4(&frustum.Planes[5], XMPlaneFromPoints(vertices[4], vertices[5], vertices[6]));

	
	return frustum; 
	*/
	
	Frustum frustum;
	//
	// Left
	//
	frustum.Planes[0].x = M(0, 3) + M(0, 0);
	frustum.Planes[0].y = M(1, 3) + M(1, 0);
	frustum.Planes[0].z = M(2, 3) + M(2, 0);
	frustum.Planes[0].w = M(3, 3) + M(3, 0);

	//
	// Right
	//
	frustum.Planes[1].x = M(0, 3) - M(0, 0);
	frustum.Planes[1].y = M(1, 3) - M(1, 0);
	frustum.Planes[1].z = M(2, 3) - M(2, 0);
	frustum.Planes[1].w = M(3, 3) - M(3, 0);

	//
	// Bottom
	//
	frustum.Planes[2].x = M(0, 3) + M(0, 1);
	frustum.Planes[2].y = M(1, 3) + M(1, 1);
	frustum.Planes[2].z = M(2, 3) + M(2, 1);
	frustum.Planes[2].w = M(3, 3) + M(3, 1);

	//
	// Top
	//
	frustum.Planes[3].x = M(0, 3) - M(0, 1);
	frustum.Planes[3].y = M(1, 3) - M(1, 1);
	frustum.Planes[3].z = M(2, 3) - M(2, 1);
	frustum.Planes[3].w = M(3, 3) - M(3, 1);

	//
	// Near
	//
	frustum.Planes[4].x = M(0, 2);
	frustum.Planes[4].y = M(1, 2);
	frustum.Planes[4].z = M(2, 2);
	frustum.Planes[4].w = M(3, 2);

	//
	// Far
	//
	frustum.Planes[5].x = M(0, 3) - M(0, 2);
	frustum.Planes[5].y = M(1, 3) - M(1, 2);
	frustum.Planes[5].z = M(2, 3) - M(2, 2);
	frustum.Planes[5].w = M(3, 3) - M(3, 2);

	// Normalize the plane equations.
	for (int i = 0; i < 6; ++i)
	{
		XMVECTOR v = XMPlaneNormalize(XMLoadFloat4(&frustum.Planes[i]));
		XMStoreFloat4(&frustum.Planes[i], v);
	}



	return frustum;
	
}

/**
@brief ��豸�� ī�޶� ����ü�� �����ϴ��� �Ǵ�
@param sphere : ��豸 �Ӽ�(�߽� ��ǥ�� ������)
@param Frustum : ����ü �Ӽ�(��� 6�� ������ ����� ������)
@return true : ����, false : �������� ����
*/

bool Collider::IsIntersectSphereWithFrustum(Sphere sphere, Frustum frustum)
{
	for (int i = 0; i < 6; i++)
	{

		XMVECTOR p = XMLoadFloat4(&frustum.Planes[i]);
		XMVECTOR v = XMLoadFloat3(&sphere.Center);
		
		// ����� ������(ax + by + cz + D)�� (x,y,z) �κп� ��豸�� �߽� ��ǥ�� �־� ��������ν� ����ü �� ���� ��豸�� �Ÿ��� ���
		float dist = XMVectorGetX(XMPlaneDotCoord(p, v));
		
		// ��豸�� ��鰣�� �Ÿ����� �������� ���̰� ������ �������� ����
		if (-dist > sphere.Radius + EPSILON) return false;

	}

	return true;
}


/**
@brief ������ ����ü�� �����ϴ��� �Ǵ�
@param point : ������ ��ǥ
@param frustum : ����ü �Ӽ�(��� 6�� ������ ����� ������)
@return true : ����, false : �������� ����
*/
bool Collider::IsIntersectPointWithFrustum(FXMVECTOR point, Frustum frustum)
{

	for (int i = 0; i < 6; i++){

		XMVECTOR p = XMLoadFloat4(&frustum.Planes[i]);
		

		// ����� ������(ax + by + cz + D)�� (x,y,z) �κп� ������ ��ǥ�� �־� ��������ν� ����ü �� ���� ������ �Ÿ��� ���
		float dist = XMVectorGetX(XMPlaneDotCoord(p, point));

		// ��� ���� -��� ����� ���ʿ� ������ �����ϹǷ� ����ü�� ������ �� ����
		if (dist< 0.0f) return false;
	}


	return true;
}



Collider::Ray Collider::ComputePickRayFromScreen(int x, int y, int width, int height, CXMMATRIX P)
{
	float vx = (2.0*x / width - 1.0) / P(0, 0);
	float vy = (-2.0*y / height + 1.0) / P(1, 1);

	Ray ray;
	ray.Origin = XMFLOAT3(0.0f, 0.0f, 0.0f);
	ray.Dir = XMFLOAT3(vx, vy, 1.0f);

	return ray;
}


bool Collider::IsIntersectSphereWithRay(Ray ray, Sphere sphere)
{

	XMVECTOR dir = XMLoadFloat3(&ray.Dir);
	XMVECTOR origin = XMLoadFloat3(&ray.Origin);

	float rad = sphere.Radius;
	XMVECTOR center = XMLoadFloat3(&sphere.Center);

	XMVECTOR m = origin - center;
	float b = XMVectorGetX(XMVector3Dot(dir, m));
	float c = XMVectorGetX(XMVector3Dot(m,m)) - rad*rad;

	float D = b*b - c;

	if (D < 0) return false;

	float t1 = -b + sqrt(b*b - c);
	float t2 = -b - sqrt(b*b - c);

	if (t1 > 0) return true;
	if (t2 > 0) return true;


	return false;
}

/**
@brief �������� �ﰢ���� �����ϴ��� �Ǻ�
@param ray : �������� �Ӽ�(�������� ����ϴ� ù ������ �������� ���ϴ� ���� ����)
@param v0,v1,v2 : �ﰢ���� ���� ��ǥ 3��
@pratm t : �������� �󸶳� ���� �ﰢ���� �����ϴ����� ���� �Ű�����
@return true : ����, false : �������� ����
*/
bool Collider::IsIntersectTriangleWithRay(Ray ray, XMVECTOR v0, XMVECTOR v1, XMVECTOR v2, float& t)
{
	/*
	�Ʒ��� �ﰢ���� ������ ������ ����ϱ� ���� �������� �̷��̴�.

	�ﰢ�� t(u,v) = (1-u-v)*v0 + u*v1 + v*v2
	������ r(t) = o + t*d;
	
	�ﰢ�� ���� ������ ���� ������ ������ ���� ���� ����� ������
	o + t*d = (1-u-v)v0 + u*v1 + v*v2 = v0 + u*(v1-v0) + v*(v2-v0)
	�����ϸ� �Ʒ��� ��������.
	-t*d + u*(v1-v0) + v*(v2-v0) = o - v0 �� ���� ��Ľ����� ǥ���ϸ�

					|t|
	[-d v1-v0 v2-v0]	|u| = o - v0 �̴�.
					|v|

	�� �� ����� ������� ���Ͽ� ����ϸ�

	|t|	        1         |(s*e1) dot e2|
	|u| = ------------- * |(-d*s) dot e2| �̴�. ���⼭ e1 = v1 - v0, e2 = v2 - v0, s = o - v0 �̴�. dot�� �����̴�
	|v|	 (-d*e1) dot e2   |(-d*e1) dot s|

	*/


	XMVECTOR dir = XMLoadFloat3(&ray.Dir); // �������� ���� ����
	XMVECTOR origin = XMLoadFloat3(&ray.Origin); // �������� �ʱ� ����
	

	// �ﰢ�� �� ����
	XMVECTOR e1 = v1 - v0; 
	XMVECTOR e2 = v2 - v0;


	XMVECTOR m = origin - v0;

	XMVECTOR x1 = XMVector3Cross(m, e1);
	XMVECTOR x2 = XMVector3Cross(dir, e2);

	float _t = XMVectorGetX(XMVector3Dot(e2, x1) / XMVector3Dot(e1, x2));
	float u = XMVectorGetX(XMVector3Dot(m, x2) / XMVector3Dot(e1, x2));
	float v = XMVectorGetX(XMVector3Dot(dir, x1) / XMVector3Dot(e1, x2));

	// T(u,v) = (1-u-v)v0 + uv1 + vv2, u+v <= 1 && u >= 0 && v >= 0 �̾�߸� �ﰢ���� ���� ���� 
	if (u >= 0 && v >= 0 && 1 >= u + v)
	{
		t = _t;
		return true;
	}

	return false;
}


/**
@brief ��豸�� ��豸�� �����ϴ��� �Ǻ�
@param s1,s2 : ���� ��豸�� �Ӽ��� ���ϰ� ����(�߽� ��ġ�� ������)
@return true : ����, false : �������� ����
*/
bool Collider::IsIntersectSphereWithSphere(Sphere s1, Sphere s2)
{


	XMVECTOR c1 = XMLoadFloat3(&s1.Center); // ��豸 s1�� �߽� ��ġ
	XMVECTOR c2 = XMLoadFloat3(&s2.Center); // ��豸 s2�� �߽���ġ

	// s1�� �߽� ��ġ�� s2�� �߽� ��ġ�� �Ÿ��� ��� dist = sqrt((c2.x - c1.x)^2 + (c2.y - c1.y)^2 + (c2.z - c1.z)^2)
	float dist = XMVectorGetX(XMVector3Length(c1 - c2));

	// �� ��豸�� �߽���ġ�� �Ÿ��� �� ��豸�� �������� �պ��� �۰ų� ���ƾ� �����Ѵ�.
	if (dist <= s1.Radius + s2.Radius)
	{
		return true;
	}


	return false;
}

/**
@brief ���� ��豸�� �����ϴ��� �Ǵ�
@param s : ��豸�� �Ӽ�(�߽���ġ�� ������)
@param p : ������ ��ǥ
@return true : ����, false : �������� ����
*/
bool Collider::IsIntersectSphereWithPoint(Sphere s, XMVECTOR p)
{

	XMVECTOR c = XMLoadFloat3(&s.Center); // ��豸�� �߽� ��ǥ


	// ������ ��豸 �߽���ǥ���� �Ÿ�
	float dist = XMVectorGetX(XMVector3Length(p - c));


	// ������ ��豸 �߽��� �Ÿ����� ��豸�� �������� Ŀ�� �����Ѵ�.
	if (s.Radius >= dist)
		return true;

	return false;
}


Collider::Sphere Collider::ComputeSphere(FXMVECTOR v1, FXMVECTOR v2, FXMVECTOR v3, CXMVECTOR v4)
{

	Sphere s;
	s.Radius = XMVectorGetX(XMVector3Length(v4 - v1)) / 2.0f;
	XMStoreFloat3(&s.Center, (v1 + v2 + v3 + v4) / 4.0f);


	return s;
}


/**
@brief AABB�� ����ü�� �����ϴ��� �Ǵ�
@param aabb : AABB�� ������ ���� ����ü
@param frustum : ����ü�� ������ ���� ����ü
@return true : ����, false : �������� ����
*/
bool Collider::IsIntersectAABBWithFrustum(AABB aabb, Frustum frustum)
{

	XMFLOAT3 PP;

	for (int i = 0; i < 6; i++)
	{
		PP = aabb.Min; // aabb���� ���� ���� ���� ��ǥ

		// ���� ���� ����� ���� ��ǥ�� ����Ѵ�.
		if (frustum.Planes[i].x >= 0)
			PP.x = aabb.Max.x;
		if (frustum.Planes[i].y >= 0)
			PP.y = aabb.Max.y;
		if (frustum.Planes[i].z >= 0)
			PP.z = aabb.Max.z;
		
		XMVECTOR V = XMLoadFloat3(&PP);
		XMVECTOR P = XMLoadFloat4(&frustum.Planes[i]);
		
		// ���� ���� ����� ���� ����� �����Ŀ� �����Ͽ� ������ ������ ����� �ڿ� �����Ƿ� ����ü�� �������� �ʴ´�.
		float d = XMVectorGetX(XMPlaneDotCoord(P, V));
		if (d + EPSILON< 0) return false;
	}

	return true;
}

bool Collider::IsIntersectAABBWithRay(AABB aabb, Ray ray)
{

	float tStart = 0.0f;
	float tEnd = 9999999999.0f;

	if (ray.Dir.x == 0.0) {
		if (ray.Origin.x < aabb.Min.x || ray.Origin.x > aabb.Max.x)
			return false;
	}

	else
	{
		float tStartX = (aabb.Min.x - ray.Origin.x) / ray.Dir.x;
		float tEndX = (aabb.Max.x - ray.Origin.x) / ray.Dir.x;
		if (tStartX > tEndX)
			std::swap(tStartX, tEndX);

		if (tStart < tStartX) tStart = tStartX;
		tEnd = tEndX;

		if (tStart > tEnd) return false;
	}

	if (ray.Dir.y == 0.0) {
		if (ray.Origin.y < aabb.Min.y || ray.Origin.y > aabb.Max.y)
			return false;
	}

	else
	{
		float tStartY = (aabb.Min.y - ray.Origin.y) / ray.Dir.y;
		float tEndY = (aabb.Max.y - ray.Origin.y) / ray.Dir.y;
		if (tStartY > tEndY)
			std::swap(tStartY, tEndY);

		if (tStart < tStartY) tStart = tStartY;
		if (tEnd > tEndY) tEnd = tEndY;

		if (tStart > tEnd) return false;
	}

	if (ray.Dir.z == 0.0) {
		if (ray.Origin.z < aabb.Min.z || ray.Origin.z > aabb.Max.z)
			return false;
	}

	else
	{
		float tStartZ = (aabb.Min.z - ray.Origin.z) / ray.Dir.z;
		float tEndZ = (aabb.Max.z - ray.Origin.z) / ray.Dir.z;
		if (tStartZ > tEndZ)
			std::swap(tStartZ, tEndZ);

		if (tStart < tStartZ) tStart = tStartZ;
		if (tEnd > tEndZ) tEnd = tEndZ;

		if (tStart > tEnd) return false;
	}


	return true;
}

bool Collider::IsIntersectAABBWithSphere(AABB aabb, Sphere s)
{

	float d = 0.0f;


	
	if (s.Center.x < aabb.Min.x)
	{
		d += (aabb.Min.x - s.Center.x) * (aabb.Min.x - s.Center.x);
	}
	else if (s.Center.x > aabb.Max.x)
	{
		d += (aabb.Max.x - s.Center.x) * (aabb.Max.x - s.Center.x);
	}
		

	if (s.Center.y < aabb.Min.y)
	{
		d += (aabb.Min.y - s.Center.y) * (aabb.Min.y - s.Center.y);
	}
	else if (s.Center.y > aabb.Max.y)
	{
		d += (aabb.Max.y - s.Center.y) * (aabb.Max.y - s.Center.y);
	}

	if (s.Center.z < aabb.Min.z)
	{
		d += (aabb.Min.z - s.Center.z) * (aabb.Min.z - s.Center.z);
	}
	else if (s.Center.z > aabb.Max.z)
	{
		d += (aabb.Max.z - s.Center.z) * (aabb.Max.z - s.Center.z);
	}

	if (d > s.Radius * s.Radius)
		return false;


	return true;
}