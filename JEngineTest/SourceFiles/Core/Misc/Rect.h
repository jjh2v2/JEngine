#pragma once
#include "../../../stdafx.h"

class Rect2P
{
public:
	Rect2P()
	{
		TopLeft = XMFLOAT2(0,0);
		BottomRight = XMFLOAT2(0,0);
	}
	Rect2P(float x1, float y1, float x2, float y2)
	{
		TopLeft = XMFLOAT2(x1, y1);
		BottomRight = XMFLOAT2(x2, y2);
	}

	XMFLOAT2 TopLeft;
	XMFLOAT2 BottomRight;
};

template <class T>
class Rect
{
public:
	Rect()
	{
		X = 0;
		Y = 0;
		Width = 0;
		Height = 0;
	}
	Rect(T x, T y, T width, T height)
	{
		X = x;
		Y = y;
		Width = width;
		Height = height;
	}

	bool IsEqualTo(const Rect<T> &other)
	{
		return	(X == other.X) &&
				(Y == other.Y) &&
				(Width == other.Width) &&
				(Height == other.Height);
	}

	T X;
	T Y;
	T Width;
	T Height;
};