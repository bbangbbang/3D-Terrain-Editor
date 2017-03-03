#pragma once
#include "d3dUtil.h"

class Collider
{
public:


	struct Frustum{
		// left, right, bottom, top, near, far
		XMFLOAT4 Planes[6];
	};

	struct Sphere {
		float Radius;
		XMFLOAT3 Center;

		Sphere() {}
		Sphere(XMVECTOR center, float rad) 
		{

			XMStoreFloat3(&Center, center);
			Radius = rad;
		}
	};

	struct Ray {
		XMFLOAT3 Origin;
		XMFLOAT3 Dir;

		Ray() {}
		Ray(XMVECTOR ori, XMVECTOR dir) {
			XMStoreFloat3(&Origin, ori);
			XMStoreFloat3(&Dir, dir);

		}
	};

	struct AABB {
		XMFLOAT3 Min;
		XMFLOAT3 Max;
	};

public:

	// return Frustum in View space
	static Frustum		ComputeFrustumFromViewProj(CXMMATRIX viewproj);
	static bool			IsIntersectSphereWithFrustum(Sphere sphere, Frustum frustum);
	static bool			IsIntersectPointWithFrustum(FXMVECTOR pos, Frustum frustm);
	static Ray			ComputePickRayFromScreen(int x, int y, int width, int height, CXMMATRIX proj);
	static bool			IsIntersectSphereWithRay(Ray ray, Sphere sphere);
	static bool			IsIntersectTriangleWithRay(Ray ray, XMVECTOR v0, XMVECTOR v1, XMVECTOR v2, float& t);
	static bool			IsIntersectSphereWithSphere(Sphere s1, Sphere s2);
	static bool			IsIntersectSphereWithPoint(Sphere s, XMVECTOR p);
	static Sphere		ComputeSphere(FXMVECTOR v1, FXMVECTOR v2, FXMVECTOR v3, CXMVECTOR v4);
	static bool			IsIntersectAABBWithFrustum(AABB aabb, Frustum frustum);
	static bool			IsIntersectAABBWithRay(AABB aabb, Ray ray);
	static bool			IsIntersectAABBWithSphere(AABB aabb, Sphere s);


private:


	static float EPSILON;

};