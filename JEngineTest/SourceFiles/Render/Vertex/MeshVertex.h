#pragma once
#include "../../../stdafx.h"

struct MeshVertex
{
public:
	XMFLOAT4 Position;
	XMFLOAT2 TexCoord;
	XMFLOAT3 Normal;
	XMFLOAT3 Tangent;
	XMFLOAT3 Binormal;
	XMFLOAT4 Color;

	static void GetTangentAndBinormal(MeshVertex data1, MeshVertex data2, MeshVertex data3, XMFLOAT3 &tangent, XMFLOAT3 &binormal)
	{
		// 생성자 함수로 연산해서 생성 하지 말고 XMVECTOR로계산 하는걸로 나중에 바꿔보자
		XMFLOAT3 v1, v2;
		XMFLOAT3 t1, t2;

		v1 = XMFLOAT3(data2.Position.x - data1.Position.x, data2.Position.y - data1.Position.y, data2.Position.z - data1.Position.z);
		v2 = XMFLOAT3(data3.Position.x - data1.Position.x, data3.Position.y - data1.Position.y, data3.Position.z - data1.Position.z);

		t1 = XMFLOAT3(data2.TexCoord.x - data1.TexCoord.x, data2.TexCoord.y - data1.TexCoord.y, 0.0f);
		t2 = XMFLOAT3(data3.TexCoord.x - data1.TexCoord.x, data3.TexCoord.y - data1.TexCoord.y, 0.0f);

		float div = 1.0f / (t1.x * t2.y - t2.x * t1.y);

		tangent.x = (t2.y * v1.x - t1.y * v2.x) * div;
		tangent.y = (t2.y * v1.y - t1.y * v2.y) * div;
		tangent.z = (t2.y * v1.z - t1.y * v2.z) * div;

		binormal.x = (t1.x * v2.x - t2.x * v1.x) * div;
		binormal.y = (t1.x * v2.y - t2.x * v1.y) * div;
		binormal.z = (t1.x * v2.z - t2.x * v1.z) * div;

		XMVECTOR tangentNor = XMVectorSet(tangent.x, tangent.y, tangent.z, 1.0f);
		tangentNor = XMVector3Normalize(tangentNor);
		XMStoreFloat3(&tangent, tangentNor);

		XMVECTOR binormalNor = XMVectorSet(binormal.x, binormal.y, binormal.z, 1.0f);
		binormalNor = XMVector3Normalize(binormalNor);
		XMStoreFloat3(&binormal, binormalNor);
	}
};

//extern 