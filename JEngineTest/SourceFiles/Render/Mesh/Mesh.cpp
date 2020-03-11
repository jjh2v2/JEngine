#include "Mesh.h"

Mesh::Mesh(ComPtr<ID3D12Device>& mDevice, UINT vertexCount, UINT indexCount, MeshVertex *meshData, std::vector<UINT> &splits, UINT *indices)
{
	//Application::Assert(vertexCount > 0 && indexCount > 0);

	mVertexCount = vertexCount;
	mIndexCount = indexCount;
	mMeshVertices = meshData;
	mMeshIndices = indices;

	for (UINT i = 0; i < splits.size(); i++)
	{
		mIndexSplits.push_back(splits[i]);
	}
	mVertexBuffer = mDevice->CreateVertexBuffer(meshData, sizeof(MeshVertex), vertexCount * sizeof(MeshVertex));
	mIndexBuffer = mDevice->CreateIndexBuffer(meshData, indexCount * sizeof(UINT));

	RecalculateBounds();
}

Mesh::~Mesh()
{
	if (mVertexBuffer)
	{
		delete mVertexBuffer;
		mVertexBuffer = NULL;
	}

	if (mIndexBuffer)
	{
		delete mIndexBuffer;
		mIndexBuffer = NULL;
	}

	if (mMeshVertices)
	{
		delete[] mMeshVertices;
		mMeshVertices = NULL;
	}

	if (mMeshIndices)
	{
		delete[] mMeshIndices;
		mMeshIndices = NULL;
	}
}

void Mesh::RecalculateBounds()
{
	if(mVertexCount <= 0)
	{
		return;
	}

	XMFLOAT3 boundMin = XMFLOAT3(mMeshVertices[0].Position.x, mMeshVertices[0].Position.y, mMeshVertices[0].Position.z);
	XMFLOAT3 boundMax = XMFLOAT3(mMeshVertices[0].Position.x, mMeshVertices[0].Position.y, mMeshVertices[0].Position.z);

	for(UINT i = 1; i < mVertexCount; i++)
	{
		boundMin.x = MathHelper::Min(mMeshVertices[i].Position.x, boundMin.x);
		boundMin.y = MathHelper::Min(mMeshVertices[i].Position.y, boundMin.y);
		boundMin.z = MathHelper::Min(mMeshVertices[i].Position.z, boundMin.z);

		boundMax.x = MathHelper::Max(mMeshVertices[i].Position.x, boundMax.x);
		boundMax.y = MathHelper::Max(mMeshVertices[i].Position.y, boundMax.y);
		boundMax.z = MathHelper::Max(mMeshVertices[i].Position.z, boundMax.z);
	}

	mMeshBounds.Set(boundMin.x, boundMin.y, boundMin.z, boundMax.x - boundMin.x, boundMax.y - boundMin.y, boundMax.z - boundMin.z);
}