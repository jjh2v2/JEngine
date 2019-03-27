#include "stdafx.h"
#include "Mesh.h"


Mesh::Mesh(ID3D12Device * device)
{
	this->device = device;
}

Mesh::Mesh(std::string objFile, ID3D12Device * device, ID3D12GraphicsCommandList* commandList)
{

	this->device = device;
	std::vector<Vertex> vertices;
	std::vector<UINT> indexVals;
	if (objFile.compare("") == 0 || objFile.compare("triangle") == 0)
	{
		// Vertex(float x, float y, float z, float u, float v) : pos(x, y, z), uv(u, v) {}
		// XMFLOAT3 pos;
		// XMFLOAT2 uv;
		// XMFLOAT3 normal;
		// XMFLOAT3 tangent;
		// float padding[5];
		Vertex point1(-0.25f, -0.25f, 0.0f, 0.0f, 0.0f);
		vertices.push_back(point1);
		Vertex point2(0.0f, 0.25f, 0.0f, 0.5f, 1.0f);
		vertices.push_back(point2);
		Vertex point3(0.25f, -0.25f, 0.0f, 1.0f, 0.0f);
		vertices.push_back(point3);
		indexVals.push_back(0);
		indexVals.push_back(1);
		indexVals.push_back(2);
		Initialize2(vertices.data(), (UINT)vertices.size(), indexVals.data(), (UINT)indexVals.size(), commandList);
	}
	else if(objFile.compare("square") == 0){
		Vertex point1(-0.25f, -0.25f, 0.0f, 0.0f, 0.0f);
		vertices.push_back(point1);
		Vertex point2(-0.25f, 0.25f, 0.0f, 0.0f, 1.0f);
		vertices.push_back(point2);
		Vertex point3(0.25f, 0.25f, 0.0f, 1.0f, 1.0f);
		vertices.push_back(point3);
		Vertex point4(0.25f, -0.25f, 0.0f, 1.0f, 0.0f);
		vertices.push_back(point4);
		indexVals.push_back(0);
		indexVals.push_back(1);
		indexVals.push_back(2);
		indexVals.push_back(0);
		indexVals.push_back(2);
		indexVals.push_back(3);
		Initialize2(vertices.data(), (UINT)vertices.size(), indexVals.data(), (UINT)indexVals.size(), commandList);
	}
	else if (objFile.compare("cube") == 0) {
		Vertex point1(-0.25f, 0.25f, 0.25f, 0.0f, 0.0f);
		Vertex point2(0.25f, 0.25f, 0.25f, 0.0f, 0.0f);
		Vertex point3(0.25f, 0.25f, -0.25f, 0.0f, 0.0f);
		Vertex point4(-0.25f, 0.25f, -0.25f, 0.0f, 0.0f);

		Vertex point5(-0.25f, -0.25f, 0.25f, 0.0f, 0.0f);
		Vertex point6(0.25f, -0.25f, 0.25f, 0.0f, 0.0f);
		Vertex point7(0.25f, -0.25f, -0.25f, 0.0f, 0.0f);
		Vertex point8(-0.25f, -0.25f, -0.25f, 0.0f, 0.0f);
		vertices.push_back(point1);
		vertices.push_back(point2);
		vertices.push_back(point3);
		vertices.push_back(point4);
		vertices.push_back(point5);
		vertices.push_back(point6);
		vertices.push_back(point7);
		vertices.push_back(point8);
		UINT Indices[] =
		{

			0, 1, 2,
			0, 2, 3,
			4, 6, 5,
			4, 7, 6,
			0, 3, 7,
			0, 7, 4,
			1, 5, 6,
			1, 6, 2,
			3, 2, 6,
			3, 6, 7,
			0, 4, 5,
			0, 5, 1
		};
		int ii = sizeof(Indices)/ sizeof(UINT);
		for (int i = 0; i < ii; i++)
			indexVals.push_back(Indices[i]);
		Initialize2(vertices.data(), (UINT)vertices.size(), indexVals.data(), (UINT)indexVals.size(), commandList);
	}
	else {
		// 오브젝트 로드 해서 정점과 인덱스를 가져온다
		Initialize(vertices.data(), (UINT)vertices.size(), indexVals.data(), (UINT)indexVals.size(), commandList);
	}
}

void Mesh::Initialize2(Vertex* vertices, UINT vertexCount, UINT * indices, UINT indexCount, ID3D12GraphicsCommandList* commandList)
{
	/*-------------------------- Vertex --------------------------*/
	vBufferSize = sizeof(Vertex) * vertexCount;

	// Note: using upload heaps to transfer static data like vert buffers is not 
	// recommended. Every time the GPU needs it, the upload heap will be marshalled 
	// over. Please read up on Default Heap usage. An upload heap is used here for 
	// code simplicity and because there are very few verts to actually transfer.
	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertexBuffer)));

	// Copy the triangle data to the vertex buffer.
	UINT8* pVertexDataBegin;
	CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
	ThrowIfFailed(vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
	memcpy(pVertexDataBegin, vertices, vBufferSize);
	vertexBuffer->Unmap(0, nullptr);

	// Initialize the vertex buffer view.
	vBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vBufferView.StrideInBytes = sizeof(Vertex);
	vBufferView.SizeInBytes = vBufferSize;

	/*-------------------------- Index --------------------------*/
	this->indexCount = indexCount;
	//UINT indexs[] = { 0,1,2 };
	//const UINT indexBufferSize = sizeof(indexs);
	//indexCount = indexBufferSize / sizeof(UINT);
	iBufferSize = sizeof(UINT) * indexCount;

	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(iBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&indexBuffer)));

	// Copy the triangle data to the vertex buffer.
	UINT8* pIndexDataBegin;
	CD3DX12_RANGE IndexreadRange(0, 0);        // We do not intend to read from this resource on the CPU.
	ThrowIfFailed(indexBuffer->Map(0, &IndexreadRange, reinterpret_cast<void**>(&pIndexDataBegin)));
	memcpy(pIndexDataBegin, indices, iBufferSize);
	indexBuffer->Unmap(0, nullptr);

	// Initialize the vertex buffer view.
	iBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	iBufferView.Format = DXGI_FORMAT_R32_UINT;
	iBufferView.SizeInBytes = iBufferSize;
}

void Mesh::Initialize(Vertex* vertices, UINT vertexCount, UINT * indices, UINT indexCount, ID3D12GraphicsCommandList* commandList)
{
	this->indexCount = indexCount;
	CalculateTangents(vertices, vertexCount, indices, indexCount);
	vBufferSize = sizeof(Vertex) * vertexCount;

	// create default heap
	device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(vBufferSize), // resource description for a buffer
		D3D12_RESOURCE_STATE_COPY_DEST, // we will start this heap in the copy destination state since we will copy data
										// from the upload heap to this heap
		nullptr, // optimized clear value must be null for this type of resource. used for render targets and depth/stencil buffers
		IID_PPV_ARGS(&vertexBuffer));

	// we can give resource heaps a name so when we debug with the graphics debugger we know what resource we are looking at
	vertexBuffer->SetName(L"Vertex Buffer Resource Heap");

	// create upload heap

	device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(vBufferSize), // resource description for a buffer
		D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
		nullptr,
		IID_PPV_ARGS(&vBufferUploadHeap));
	vBufferUploadHeap->SetName(L"Vertex Buffer Upload Resource Heap");

	// store vertex buffer in upload heap
	D3D12_SUBRESOURCE_DATA vertexData = {};
	vertexData.pData = reinterpret_cast<BYTE*>(vertices); // pointer to our vertex array
	vertexData.RowPitch = vBufferSize; // size of all our triangle vertex data
	vertexData.SlicePitch = vBufferSize; // also the size of our triangle vertex data

	//UpdateSubresources(commandList, vertexBuffer, vBufferUploadHeap, 0, 0, 1, &vertexData);

	// transition the vertex buffer data from copy destination state to vertex buffer state
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
	iBufferSize = sizeof(UINT) * indexCount;

	// create default heap to hold index buffer
	device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(iBufferSize), // resource description for a buffer
		D3D12_RESOURCE_STATE_COPY_DEST, // start in the copy destination state
		nullptr, // optimized clear value must be null for this type of resource
		IID_PPV_ARGS(&indexBuffer));

	// we can give resource heaps a name so when we debug with the graphics debugger we know what resource we are looking at
	indexBuffer->SetName(L"Index Buffer Resource Heap");

	// create upload heap to upload index buffer
	device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(vBufferSize), // resource description for a buffer
		D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
		nullptr,
		IID_PPV_ARGS(&iBufferUploadHeap));
	iBufferUploadHeap->SetName(L"Index Buffer Upload Resource Heap");

	// store vertex buffer in upload heap
	D3D12_SUBRESOURCE_DATA indexData = {};
	indexData.pData = reinterpret_cast<BYTE*>(indices); // pointer to our index array
	indexData.RowPitch = iBufferSize; // size of all our index buffer
	indexData.SlicePitch = iBufferSize; // also the size of our index buffer

										// we are now creating a command with the command list to copy the data from
										// the upload heap to the default heap
	//UpdateSubresources(commandList, indexBuffer, iBufferUploadHeap, 0, 0, 1, &indexData);

	// transition the vertex buffer data from copy destination state to vertex buffer state
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(indexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

	vBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vBufferView.StrideInBytes = sizeof(Vertex);
	vBufferView.SizeInBytes = vBufferSize;

	iBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	iBufferView.Format = DXGI_FORMAT_R32_UINT;
	iBufferView.SizeInBytes = iBufferSize;
}

void Mesh::CalculateTangents(Vertex * vertices, UINT vertexCount, UINT * indices, UINT indexCount)
{
	XMFLOAT3 *tan1 = new XMFLOAT3[vertexCount * 2];
	XMFLOAT3 *tan2 = tan1 + vertexCount;
	ZeroMemory(tan1, vertexCount * sizeof(XMFLOAT3) * 2);
	int triangleCount = indexCount / 3;
	for (UINT i = 0; i < indexCount; i += 3)
	{
		int i1 = indices[i];
		int i2 = indices[i + 2];
		int i3 = indices[i + 1];
		XMFLOAT3 v1 = vertices[i1].pos;
		XMFLOAT3 v2 = vertices[i2].pos;
		XMFLOAT3 v3 = vertices[i3].pos;

		XMFLOAT2 w1 = vertices[i1].uv;
		XMFLOAT2 w2 = vertices[i2].uv;
		XMFLOAT2 w3 = vertices[i3].uv;

		float x1 = v2.x - v1.x;
		float x2 = v3.x - v1.x;
		float y1 = v2.y - v1.y;
		float y2 = v3.y - v1.y;
		float z1 = v2.z - v1.z;
		float z2 = v3.z - v1.z;

		float s1 = w2.x - w1.x;
		float s2 = w3.x - w1.x;
		float t1 = w2.y - w1.y;
		float t2 = w3.y - w1.y;
		float r = 1.0F / (s1 * t2 - s2 * t1);

		XMFLOAT3 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r,
			(t2 * z1 - t1 * z2) * r);

		XMFLOAT3 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r,
			(s1 * z2 - s2 * z1) * r);

		XMStoreFloat3(&tan1[i1], XMLoadFloat3(&tan1[i1]) + XMLoadFloat3(&sdir));
		XMStoreFloat3(&tan1[i2], XMLoadFloat3(&tan1[i2]) + XMLoadFloat3(&sdir));
		XMStoreFloat3(&tan1[i3], XMLoadFloat3(&tan1[i3]) + XMLoadFloat3(&sdir));

		XMStoreFloat3(&tan2[i1], XMLoadFloat3(&tan2[i1]) + XMLoadFloat3(&tdir));
		XMStoreFloat3(&tan2[i2], XMLoadFloat3(&tan2[i2]) + XMLoadFloat3(&tdir));
		XMStoreFloat3(&tan2[i3], XMLoadFloat3(&tan2[i3]) + XMLoadFloat3(&tdir));
	}

	for (UINT a = 0; a < vertexCount; a++)
	{
		auto n = vertices[a].normal;
		auto t = tan1[a];

		// Gram-Schmidt orthogonalize
		auto dot = XMVector3Dot(XMLoadFloat3(&n), XMLoadFloat3(&t));
		XMStoreFloat3(&vertices[a].tangent, XMVector3Normalize(XMLoadFloat3(&t) - XMLoadFloat3(&n)* dot));

		// Calculate handedness
		/*tangent[a].w = (Dot(Cross(n, t), tan2[a]) < 0.0F) ? -1.0F : 1.0F;*/
	}

	delete[] tan1;
}

const D3D12_VERTEX_BUFFER_VIEW& Mesh::GetVertexBufferView()
{
	return vBufferView;
}

const D3D12_INDEX_BUFFER_VIEW& Mesh::GetIndexBufferView()
{
	return iBufferView;
}

const UINT& Mesh::GetIndexCount()
{
	return indexCount;
}

Mesh::~Mesh()
{
	vertexBuffer->Release();
	indexBuffer->Release();

	iBufferUploadHeap->Release(); //Most probably should have one upload heap for all meshes
	vBufferUploadHeap->Release();
}
