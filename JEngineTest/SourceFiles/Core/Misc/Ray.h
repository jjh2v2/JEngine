#pragma once
#include "../../../stdafx.h"

class Ray
{
public:
	Ray(){};
	Ray(XMFLOAT3 origin, XMFLOAT3 direction)
	{
		Origin = origin;
		Direction = direction;
	}
	~Ray(){};

	XMFLOAT3 Origin;
	XMFLOAT3 Direction;
};