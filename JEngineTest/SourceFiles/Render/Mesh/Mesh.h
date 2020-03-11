#pragma once
#include "../../../stdafx.h"
#include "../Vertex/MeshVertex.h"
#include "../Buffer/GPUResource.h"
#include "../../Core/Misc/Box.h"


//class Direct3DManager;

class Mesh
{
public:
	Mesh(ComPtr<ID3D12Device>& mDevice, UINT vertexCount, UINT indexCount, MeshVertex *meshData, std::vector<UINT> &splits, UINT *indices);
	virtual ~Mesh();

	UINT GetIndexCount() { return mIndexCount; }
	UINT GetVertexCount() { return mVertexCount; }
	UINT GetMeshSplitCount() { return mIndexSplits.size(); }
	UINT GetMeshIndexSplitByIndex(UINT index) { return mIndexSplits[index]; }
	MeshVertex *GetMeshData() { return mMeshVertices; }
	UINT *GetIndices() { return mMeshIndices; }

	void RecalculateBounds();
	Box GetBounds(){return mMeshBounds;}

	VertexBuffer *GetVertexBuffer() { return mVertexBuffer; }
	IndexBuffer *GetIndexBuffer() { return mIndexBuffer; }

    bool IsReady() { return mVertexBuffer->GetIsReady() && mIndexBuffer->GetIsReady(); }

protected:

	VertexBuffer *mVertexBuffer;
	IndexBuffer *mIndexBuffer;
	MeshVertex *mMeshVertices;
	UINT *mMeshIndices;
	std::vector<UINT> mIndexSplits;
	UINT mVertexCount;
	UINT mIndexCount;
	Box mMeshBounds;
};