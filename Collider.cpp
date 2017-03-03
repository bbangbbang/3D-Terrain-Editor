
#include "Collider.h"

float Collider::EPSILON = 0.0f;
/**
@brief View 행렬과 Projection 행렬을 통해 World 공간의 카메라 절두체 계산
@param M : View Matrix * Projection Matrix
@return 카메라 절두체 정보를 담은 구조체 반환
*/
Collider::Frustum Collider::ComputeFrustumFromViewProj(CXMMATRIX M)
{
	/*
	Frustum frustum; // 절두체 면 6개를 담는 구조체

	// NDC 공간에서의 카메라 절두체 정점 8개 좌표
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
	XMMATRIX invM = XMMatrixInverse(&det, M); // View * Proj의 역행렬

	// NDC 공간에서의 카메라 절두체 정점 8개를 월드 공간의 좌표로 변환
	for (int i = 0; i < 8; i++)
	{

		vertices[i] = XMVector3Transform(vertices[i], invM);
		vertices[i] = vertices[i] * XMVectorReciprocal(XMVectorSplatW(vertices[i]));
	}

	// 월드 공간의 정점 8개를 이용하여 절두체를 이루는 평면 6개를 계산 하여 저장
	//ax + by + cz + D 에서 법선 벡터(a,b,c) 와 D 저장
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
@brief 경계구와 카메라 절두체가 교차하는지 판단
@param sphere : 경계구 속성(중심 좌표와 반지름)
@param Frustum : 절두체 속성(평면 6개 각각의 평면의 방정식)
@return true : 교차, false : 교차하지 않음
*/

bool Collider::IsIntersectSphereWithFrustum(Sphere sphere, Frustum frustum)
{
	for (int i = 0; i < 6; i++)
	{

		XMVECTOR p = XMLoadFloat4(&frustum.Planes[i]);
		XMVECTOR v = XMLoadFloat3(&sphere.Center);
		
		// 평면의 방정식(ax + by + cz + D)의 (x,y,z) 부분에 경계구의 중심 좌표를 넣어 계산함으로써 절두체 각 평면과 경계구의 거리를 계산
		float dist = XMVectorGetX(XMPlaneDotCoord(p, v));
		
		// 경계구와 평면간의 거리보다 반지름의 길이가 작으면 교차하지 않음
		if (-dist > sphere.Radius + EPSILON) return false;

	}

	return true;
}


/**
@brief 정점과 절두체가 교차하는지 판단
@param point : 정점의 좌표
@param frustum : 절두체 속성(평면 6개 각각의 평면의 방정식)
@return true : 교차, false : 교차하지 않음
*/
bool Collider::IsIntersectPointWithFrustum(FXMVECTOR point, Frustum frustum)
{

	for (int i = 0; i < 6; i++){

		XMVECTOR p = XMLoadFloat4(&frustum.Planes[i]);
		

		// 평면의 방정식(ax + by + cz + D)의 (x,y,z) 부분에 정점의 좌표를 넣어 계산함으로써 절두체 각 평면과 정점의 거리를 계산
		float dist = XMVectorGetX(XMPlaneDotCoord(p, point));

		// 계산 값이 -라면 평면의 뒤쪽에 정점이 존재하므로 절두체와 교차할 수 없음
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
@brief 반직선과 삼각형이 교차하는지 판별
@param ray : 반직선의 속성(반직선이 출발하는 첫 지점과 반직선이 향하는 방향 벡터)
@param v0,v1,v2 : 삼각형의 정점 좌표 3개
@pratm t : 반직선이 얼마나 가야 삼각형과 교차하는지에 대한 매개변수
@return true : 교차, false : 교차하지 않음
*/
bool Collider::IsIntersectTriangleWithRay(Ray ray, XMVECTOR v0, XMVECTOR v1, XMVECTOR v2, float& t)
{
	/*
	아래는 삼각형과 반직선 교차를 계산하기 위한 수학적인 이론이다.

	삼각형 t(u,v) = (1-u-v)*v0 + u*v1 + v*v2
	반직선 r(t) = o + t*d;
	
	삼각형 안의 임의의 점과 광선의 임의의 점이 같을 경우의 방정식
	o + t*d = (1-u-v)v0 + u*v1 + v*v2 = v0 + u*(v1-v0) + v*(v2-v0)
	정리하면 아래와 같아진다.
	-t*d + u*(v1-v0) + v*(v2-v0) = o - v0 이 식을 행렬식으로 표현하면

					|t|
	[-d v1-v0 v2-v0]	|u| = o - v0 이다.
					|v|

	앞 쪽 행렬의 역행렬을 구하여 계산하면

	|t|	        1         |(s*e1) dot e2|
	|u| = ------------- * |(-d*s) dot e2| 이다. 여기서 e1 = v1 - v0, e2 = v2 - v0, s = o - v0 이다. dot은 내적이다
	|v|	 (-d*e1) dot e2   |(-d*e1) dot s|

	*/


	XMVECTOR dir = XMLoadFloat3(&ray.Dir); // 반직선의 방향 벡터
	XMVECTOR origin = XMLoadFloat3(&ray.Origin); // 반직선의 초기 지점
	

	// 삼각형 두 벡터
	XMVECTOR e1 = v1 - v0; 
	XMVECTOR e2 = v2 - v0;


	XMVECTOR m = origin - v0;

	XMVECTOR x1 = XMVector3Cross(m, e1);
	XMVECTOR x2 = XMVector3Cross(dir, e2);

	float _t = XMVectorGetX(XMVector3Dot(e2, x1) / XMVector3Dot(e1, x2));
	float u = XMVectorGetX(XMVector3Dot(m, x2) / XMVector3Dot(e1, x2));
	float v = XMVectorGetX(XMVector3Dot(dir, x1) / XMVector3Dot(e1, x2));

	// T(u,v) = (1-u-v)v0 + uv1 + vv2, u+v <= 1 && u >= 0 && v >= 0 이어야만 삼각형의 성질 만족 
	if (u >= 0 && v >= 0 && 1 >= u + v)
	{
		t = _t;
		return true;
	}

	return false;
}


/**
@brief 경계구와 경계구가 교차하는지 판별
@param s1,s2 : 각각 경계구의 속성을 지니고 있음(중심 위치와 반지름)
@return true : 교차, false : 교차하지 않음
*/
bool Collider::IsIntersectSphereWithSphere(Sphere s1, Sphere s2)
{


	XMVECTOR c1 = XMLoadFloat3(&s1.Center); // 경계구 s1의 중심 위치
	XMVECTOR c2 = XMLoadFloat3(&s2.Center); // 경계구 s2의 중심위치

	// s1의 중심 위치와 s2의 중심 위치의 거리를 계산 dist = sqrt((c2.x - c1.x)^2 + (c2.y - c1.y)^2 + (c2.z - c1.z)^2)
	float dist = XMVectorGetX(XMVector3Length(c1 - c2));

	// 각 경계구의 중심위치의 거리가 두 경계구의 반지름의 합보다 작거나 같아야 교차한다.
	if (dist <= s1.Radius + s2.Radius)
	{
		return true;
	}


	return false;
}

/**
@brief 점과 경계구가 교차하는지 판단
@param s : 경계구의 속성(중심위치와 반지름)
@param p : 정점의 좌표
@return true : 교차, false : 교차하지 않음
*/
bool Collider::IsIntersectSphereWithPoint(Sphere s, XMVECTOR p)
{

	XMVECTOR c = XMLoadFloat3(&s.Center); // 경계구의 중심 좌표


	// 정점과 경계구 중심좌표간의 거리
	float dist = XMVectorGetX(XMVector3Length(p - c));


	// 정점과 경계구 중심의 거리보다 경계구의 반지름이 커야 교차한다.
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
@brief AABB와 절두체가 교차하는지 판단
@param aabb : AABB의 정보를 담은 구조체
@param frustum : 절두체의 정보를 담은 구조체
@return true : 교차, false : 교차하지 않음
*/
bool Collider::IsIntersectAABBWithFrustum(AABB aabb, Frustum frustum)
{

	XMFLOAT3 PP;

	for (int i = 0; i < 6; i++)
	{
		PP = aabb.Min; // aabb에서 가장 값이 작은 좌표

		// 평면과 가장 가까운 점의 좌표를 계산한다.
		if (frustum.Planes[i].x >= 0)
			PP.x = aabb.Max.x;
		if (frustum.Planes[i].y >= 0)
			PP.y = aabb.Max.y;
		if (frustum.Planes[i].z >= 0)
			PP.z = aabb.Max.z;
		
		XMVECTOR V = XMLoadFloat3(&PP);
		XMVECTOR P = XMLoadFloat4(&frustum.Planes[i]);
		
		// 평면과 가장 가까운 점을 평면의 방정식에 대입하여 음수가 나오면 평면의 뒤에 있으므로 절두체와 교차하지 않는다.
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