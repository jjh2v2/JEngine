#pragma once
#include "../../../stdafx.h"

struct Vertex
{
	Vertex() {}
	Vertex(float x, float y, float z, float u, float v) : pos(x, y, z), uv(u, v) {}
	XMFLOAT3 pos;
	XMFLOAT2 uv;
	XMFLOAT3 normal;
	XMFLOAT3 tangent;
	XMFLOAT4 color;
};


struct SceneConstantBuffer
{
	XMFLOAT4X4 worldViewProjection;
	XMFLOAT4X4 world;
	//XMFLOAT4X4 model;
	//XMFLOAT4X4 view;
	//XMFLOAT4X4 projection;
	//XMFLOAT4 ambientColor;
	//BOOL sampleShadowMap;
	//BOOL padding[3];        // Must be aligned to be made up of N float4s.
	//LightState lights[NumLights];
};