#pragma once
#include "stdafx.h"
#include <fstream>
#include <vector>

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

class Mesh
{
	ID3D12Resource* vertexBuffer;
	ID3D12Resource* indexBuffer;
	ID3D12Resource* vBufferUploadHeap;
	ID3D12Resource* iBufferUploadHeap;
	UINT vBufferSize;
	UINT iBufferSize;
	ID3D12Device* device;
	D3D12_VERTEX_BUFFER_VIEW vBufferView;
	D3D12_INDEX_BUFFER_VIEW iBufferView;
	UINT indexCount;
public:
	Mesh(ID3D12Device* device);
	Mesh(std::string objFile, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, XMFLOAT3 _position);
	void Initialize(Vertex* vertices, UINT vertexCount, UINT* indices, UINT indexCount, ID3D12GraphicsCommandList* commandList);
	void CalculateTangents(Vertex* vertices, UINT vertexCount, UINT * indices, UINT indexCount);
	const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView();
	const D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView();
	const UINT& GetIndexCount();
	~Mesh();

	XMFLOAT4X4 mWorld;
	XMFLOAT3 position;
	XMFLOAT3 rotation;
	XMFLOAT3 scale;

	XMFLOAT3& GetPosition();
	void SetPosition(XMFLOAT3& _position);
	XMFLOAT4X4& GetWorld();


	void Initialize2(Vertex* vertices, UINT vertexCount, UINT* indices, UINT indexCount, ID3D12GraphicsCommandList* commandList);
};