#include "DXAssetsConstantBuffer.h"

DXAssetsConstantBuffer::DXAssetsConstantBuffer(std::string strAssetname) :
	DXAssets(strAssetname, eConstantBuffer),
	mpCBVHeap(nullptr),
	mpConstantBuffer(nullptr)
{
	CreateConstantBuffer();
}

void DXAssetsConstantBuffer::CreateConstantBuffer()
{
	UINT NumDescriptors = 1;
	UINT objectCount = 2;

	// Describe and create a constant buffer view (CBV) descriptor heap.
		// Flags indicate that this descriptor heap can be bound to the pipeline 
		// and that descriptors contained in it can be referenced by a root table.
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
	cbvHeapDesc.NumDescriptors = NumDescriptors;// 해당 디스크립힙에서 사용할 전체 상수버퍼 개수 꼭 다수가아니여도 되는듯??
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	ThrowIfFailed(DXPipelineManager::Get()->GetDevice()->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&mpCBVHeap)));

	// Create the constant buffer.
	{
		UINT mElementByteSize = sizeof(SceneConstantBuffer);
		// (sizeof(SceneConstantBuffer) + 255) & ~255;    // CB size is required to be 256-byte aligned.
		mElementByteSize = CalcConstantBufferByteSize(mElementByteSize);

		ThrowIfFailed(DXPipelineManager::Get()->GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(mElementByteSize * objectCount),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&mpConstantBuffer)));

		// Describe and create a constant buffer view.
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = mpConstantBuffer->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = CalcConstantBufferByteSize(sizeof(SceneConstantBuffer));    // CB size is required to be 256-byte aligned.
		DXPipelineManager::Get()->GetDevice()->CreateConstantBufferView(&cbvDesc, mpCBVHeap->GetCPUDescriptorHandleForHeapStart());

		//// Map and initialize the constant buffer. We don't unmap this until the
		//// app closes. Keeping things mapped for the lifetime of the resource is okay.
		//CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
		//ThrowIfFailed(mConstantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&mPCBVDataBegin)));
		//memcpy(mPCBVDataBegin, &mConstantBufferData, sizeof(mConstantBufferData));
	}
}

void DXAssetsConstantBuffer::CreateConstantBufferss(size_t iAddCount /*= 1*/)
{
	//mpConstantBuffer->Release();
	//mpCBVHeap->Release();

	//mpConstantBuffer.Detach();
	//mpCBVHeap.Detach();
	//mpConstantBuffer = nullptr;
	//mpCBVHeap = nullptr;

	UINT objectCount = static_cast<UINT>(iAddCount);

	// Describe and create a constant buffer view (CBV) descriptor heap.
		// Flags indicate that this descriptor heap can be bound to the pipeline 
		// and that descriptors contained in it can be referenced by a root table.
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
	cbvHeapDesc.NumDescriptors = objectCount;// 해당 디스크립힙에서 사용할 전체 상수버퍼 개수 꼭 다수가아니여도 되는듯??
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	ThrowIfFailed(DXPipelineManager::Get()->GetDevice()->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&mpCBVHeap)));

	// Create the constant buffer.
	{
		UINT mElementByteSize = sizeof(SceneConstantBuffer);
		// (sizeof(SceneConstantBuffer) + 255) & ~255;    // CB size is required to be 256-byte aligned.
		mElementByteSize = CalcConstantBufferByteSize(mElementByteSize);

		ThrowIfFailed(DXPipelineManager::Get()->GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(mElementByteSize * objectCount),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&mpConstantBuffer)));

		// Describe and create a constant buffer view.
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = mpConstantBuffer->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = CalcConstantBufferByteSize(sizeof(SceneConstantBuffer));    // CB size is required to be 256-byte aligned.
		DXPipelineManager::Get()->GetDevice()->CreateConstantBufferView(&cbvDesc, mpCBVHeap->GetCPUDescriptorHandleForHeapStart());

		//// Map and initialize the constant buffer. We don't unmap this until the
		//// app closes. Keeping things mapped for the lifetime of the resource is okay.
		//CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
		//ThrowIfFailed(mConstantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&mPCBVDataBegin)));
		//memcpy(mPCBVDataBegin, &mConstantBufferData, sizeof(mConstantBufferData));
	}
}

ComPtr<ID3D12DescriptorHeap> DXAssetsConstantBuffer::GetCBVHeap()
{
	return mpCBVHeap;
}

ComPtr<ID3D12Resource> DXAssetsConstantBuffer::GetCBVBuffer()
{
	return mpConstantBuffer;
}
