#pragma once
#include "../../../stdafx.h"
#include "Ray.h"

class Box
{
public:
	Box()
	{
		X = Y = Z = 0.0f;
		Width = Height = Depth = 1.0f;
	}

	Box(float x, float y, float z, float width, float height, float depth)
	{
		X = x;
		Y = y;
		Z = z;
		Width = width;
		Height = height;
		Depth = depth;
	}

	XMFLOAT3 GetMin()
	{
		return XMFLOAT3(X,Y,Z);
	}

	XMFLOAT3 GetMax()
	{
		return XMFLOAT3(X+Width, Y+Height, Z+Depth);
	}

	XMFLOAT3 GetXYZ()
	{
		return XMFLOAT3(X, Y, Z);
	}

	void Set(float x, float y, float z, float width, float height, float depth)
	{
		X = x;
		Y = y;
		Z = z;
		Width = width;
		Height = height;
		Depth = depth;
	}

	void SetXYZ(XMFLOAT3 point)
	{
		SetXYZ(point.x, point.y, point.z);
	}

	void SetXYZ(float x, float y, float z)
	{
		X = x;
		Y = y;
		Z = z;
	}

	XMFLOAT3 GetBoundCenter()
	{
		XMFLOAT3 center(X + Width/2.0f, Y + Height/2.0f, Z + Depth/2.0f);
		return center;
	}

	XMFLOAT3 GetSize()
	{
		return XMFLOAT3(Width, Height, Depth);
	}

	bool IsCollidingWith(const XMFLOAT3 &point)
	{
		return IsCollidingWith(point.x, point.y, point.z);
	}

	bool IsCollidingWith(float x, float y, float z)
	{
		if(x >= X && x <= (X + Width) && y >= Y && y <= (Y + Height) && z >= Z && z <= (Z + Depth))
		{
			return true;
		}
		return false;
	}

	bool IsCollidingWith(const Ray &ray)
	{
		float tmin, tmax, tymin, tymax, tzmin, tzmax;
		if (ray.Direction.x >= 0) 
		{
			tmin = (X - ray.Origin.x) / ray.Direction.x;
			tmax = ((X + Width) - ray.Origin.x) / ray.Direction.x;
		}
		else 
		{
			tmin = ((X + Width) - ray.Origin.x) / ray.Direction.x;
			tmax = (X - ray.Origin.x) / ray.Direction.x;
		}
		if (ray.Direction.y >= 0) 
		{
			tymin = (Y - ray.Origin.y) / ray.Direction.y;
			tymax = ((Y + Height) - ray.Origin.y) / ray.Direction.y;
		}
		else 
		{
			tymin = ((Y + Height) - ray.Origin.y) / ray.Direction.y;
			tymax = (Y - ray.Origin.y) / ray.Direction.y;
		}
		if ( (tmin > tymax) || (tymin > tmax) )
		{
			return false;
		}
		if (tymin > tmin)
		{
			tmin = tymin;
		}
		if (tymax < tmax)
		{
			tmax = tymax;
		}
		if (ray.Direction.z >= 0) 
		{
			tzmin = (Z - ray.Origin.z) / ray.Direction.z;
			tzmax = ((Z + Depth) - ray.Origin.z) / ray.Direction.z;
		}
		else 
		{
			tzmin = ((Z + Depth) - ray.Origin.z) / ray.Direction.z;
			tzmax = (Z - ray.Origin.z) / ray.Direction.z;
		}
		if ( (tmin > tzmax) || (tzmin > tmax) )
		{
			return false;
		}
		if (tzmin > tmin)
		{
			tmin = tzmin;
		}
		if (tzmax < tmax)
		{
			tmax = tzmax;
		}
		return true;//( (tmin < t1) && (tmax > t0) );
	}

	float X;
	float Y;
	float Z;
	float Width;
	float Height;
	float Depth;
private:
};