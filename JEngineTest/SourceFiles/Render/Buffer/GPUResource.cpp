#include "GPUResource.h"


GPUResource::GPUResource(ID3D12Resource *resource, D3D12_RESOURCE_STATES usageState)
{
	mResource = resource;
	mUsageState = usageState;
	mGPUAddress = 0;
	mIsReady = false;
}

GPUResource::~GPUResource()
{
	mResource->Release();
	mResource = NULL;
}

TextureResource::TextureResource(ID3D12Resource *resource, D3D12_RESOURCE_STATES usageState, DescriptorHeapHandle shaderResourceViewHandle)
	:GPUResource(resource, usageState)
{
	mShaderResourceViewHandle = shaderResourceViewHandle;
}

TextureResource::~TextureResource()
{

}

BackBufferTarget::BackBufferTarget(ID3D12Resource *resource, D3D12_RESOURCE_STATES usageState, DescriptorHeapHandle renderTargetViewHandle)
	:GPUResource(resource, usageState)
{
	mRenderTargetViewHandle = renderTargetViewHandle;
}

BackBufferTarget::~BackBufferTarget()
{

}

RenderTarget::RenderTarget(ID3D12Resource *resource, D3D12_RESOURCE_STATES usageState, D3D12_RESOURCE_DESC renderTargetDesc, DescriptorHeapHandle renderTargetViewHandle,
	std::vector<DescriptorHeapHandle> &renderTargetViewArray, DescriptorHeapHandle shaderResourceViewHandle, const UAVHandle &unorderedAccessView)
	:GPUResource(resource, usageState)
{
	mRenderTargetDesc = renderTargetDesc;
	mRenderTargetViewHandle = renderTargetViewHandle;
	mShaderResourceViewHandle = shaderResourceViewHandle;
	mUnorderedAccessView.HasUAV = unorderedAccessView.HasUAV;
	mUnorderedAccessView.BaseHandle = unorderedAccessView.BaseHandle;

	for (UINT i = 0; i < unorderedAccessView.Handles.size(); i++)
	{
		mUnorderedAccessView.Handles.push_back(unorderedAccessView.Handles[i]);
	}

	for (UINT i = 0; i < renderTargetViewArray.size(); i++)
	{
		mRenderTargetViewArray.push_back(renderTargetViewArray[i]);
	}

	mIsReady = true;
}

RenderTarget::~RenderTarget()
{

}

DescriptorHeapHandle RenderTarget::GetUnorderedAccessViewHandle(UINT mipIndex, UINT arrayIndex)
{
	//Application::Assert(mUnorderedAccessView.HasUAV);
	//Application::Assert(mRenderTargetDesc.MipLevels > mipIndex);
	//Application::Assert(mRenderTargetDesc.DepthOrArraySize > arrayIndex);

	return mUnorderedAccessView.Handles[arrayIndex * mRenderTargetDesc.MipLevels + mipIndex];
}


DepthStencilTarget::DepthStencilTarget(ID3D12Resource* resource, D3D12_RESOURCE_STATES usageState, D3D12_RESOURCE_DESC depthStencilDesc, DescriptorHeapHandle depthStencilViewHandle, DescriptorHeapHandle readOnlyDepthStencilViewHandle,
	std::vector<DescriptorHeapHandle> &depthStencilViewArray, std::vector<DescriptorHeapHandle> &readOnlyDepthStencilViewArray, DescriptorHeapHandle shaderResourceViewHandle)
	:GPUResource(resource, usageState)
{
	mDepthStencilDesc = depthStencilDesc;
	mDepthStencilViewHandle = depthStencilViewHandle;
	mReadOnlyDepthStencilViewHandle = readOnlyDepthStencilViewHandle;
	mShaderResourceViewHandle = shaderResourceViewHandle;
	mIsReady = true;

	for (UINT i = 0; i < depthStencilViewArray.size(); i++)
	{
		mDepthStencilViewArray.push_back(depthStencilViewArray[i]);
	}

	for (UINT i = 0; i < readOnlyDepthStencilViewArray.size(); i++)
	{
		mReadOnlyDepthStencilViewArray.push_back(readOnlyDepthStencilViewArray[i]);
	}
}

DepthStencilTarget::~DepthStencilTarget()
{

}

VertexBuffer::VertexBuffer(ID3D12Resource *resource, D3D12_RESOURCE_STATES usageState, UINT vertexStride, UINT bufferSize)
	:GPUResource(resource, usageState)
{
	mGPUAddress = resource->GetGPUVirtualAddress();
	mVertexBufferView.StrideInBytes = vertexStride;
	mVertexBufferView.SizeInBytes = bufferSize;
	mVertexBufferView.BufferLocation = mGPUAddress;
}

VertexBuffer::~VertexBuffer()
{

}

IndexBuffer::IndexBuffer(ID3D12Resource *resource, D3D12_RESOURCE_STATES usageState, UINT bufferSize)
	:GPUResource(resource, usageState)
{
	mGPUAddress = resource->GetGPUVirtualAddress();
	mIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	mIndexBufferView.SizeInBytes = bufferSize;
	mIndexBufferView.BufferLocation = mGPUAddress;
}

IndexBuffer::~IndexBuffer()
{

}

ConstantBuffer::ConstantBuffer(ID3D12Resource *resource, D3D12_RESOURCE_STATES usageState, UINT bufferSize, DescriptorHeapHandle constantBufferViewHandle)
	:GPUResource(resource, usageState)
{
	mGPUAddress = resource->GetGPUVirtualAddress();
	mBufferSize = bufferSize;
	mConstantBufferViewHandle = constantBufferViewHandle;

	mMappedBuffer = NULL;
	mResource->Map(0, NULL, reinterpret_cast<void**>(&mMappedBuffer));
}

ConstantBuffer::~ConstantBuffer()
{
	mResource->Unmap(0, NULL);
}

void ConstantBuffer::SetConstantBufferData(const void *bufferData, UINT bufferSize)
{
	//Application::Assert(bufferSize <= mBufferSize);
	memcpy(mMappedBuffer, bufferData, bufferSize);
}

StructuredBuffer::StructuredBuffer(ID3D12Resource *resource, ID3D12Resource *uploadResource, D3D12_RESOURCE_STATES usageState, bool isRaw, StructuredBufferAccess accessType, D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc, DescriptorHeapHandle uavHandle, DescriptorHeapHandle srvHandle)
	:GPUResource(resource, usageState)
{
	mMappedBuffer = NULL;
	mUploadResource = uploadResource;
	mIsRaw = isRaw;
	mAccessType = accessType;
	mUAVDesc = uavDesc;
	mUnorderedAccessViewHandle = uavHandle;
	mShaderResourceViewHandle = srvHandle;

	if (mAccessType == CPU_WRITE_GPU_READ)
	{
		ThrowIfFailed(mResource->Map(0, NULL, reinterpret_cast<void**>(&mMappedBuffer)));
	}
}

StructuredBuffer::~StructuredBuffer()
{
	if (mAccessType == CPU_WRITE_GPU_READ)
	{
		mResource->Unmap(0, NULL);
	}

	if (mUploadResource)
	{
		mUploadResource->Release();
		mUploadResource = NULL;
	}
}

bool StructuredBuffer::CopyToBuffer(const void* bufferData, UINT bufferSize)
{
	bool needsGPUCopy = false;

	switch (mAccessType)
	{
	case GPU_READ_WRITE:
		//Application::Assert(false); //Trying to copy to a buffer that was designated as never written to by the CPU
		break;
	case CPU_WRITE_GPU_READ:
		//copy to the always-mapped resource
		//Application::Assert(mMappedBuffer != NULL);
		//Application::Assert(bufferSize <= (mUAVDesc.Buffer.NumElements * mUAVDesc.Buffer.StructureByteStride));
		memcpy(mMappedBuffer, bufferData, bufferSize);
		break;
	case CPU_WRITE_GPU_READ_WRITE:
		//Application::Assert(bufferSize <= (mUAVDesc.Buffer.NumElements * mUAVDesc.Buffer.StructureByteStride));

		{
			//copy data to the upload buffer
			void *mappedUpload = NULL;
			ThrowIfFailed(mUploadResource->Map(0, NULL, reinterpret_cast<void**>(&mappedUpload)));
			memcpy(mappedUpload, bufferData, bufferSize);
			mUploadResource->Unmap(0, NULL);
		}

		//then return that we need to schedule a GPU copy, but leave it up to the managing system to schedule when exactly
		needsGPUCopy = true;
		break;
	default:
		//Application::Assert(false); //not implemented
		break;
	}

	return needsGPUCopy;
}

FilteredCubeMapRenderTexture::FilteredCubeMapRenderTexture(ID3D12Resource *resource, D3D12_RESOURCE_STATES usageState, UINT numMipLevels, const std::vector<DescriptorHeapHandle> &uavHandles, DescriptorHeapHandle srvHandle)
	:GPUResource(resource, usageState)
{
	mSRVHandle = srvHandle;
	mNumMips = numMipLevels;
	mComputeFence = 0;

	for (UINT i = 0; i < uavHandles.size(); i++)
	{
		mUAVHandles.push_back(uavHandles[i]);
	}
}

FilteredCubeMapRenderTexture::~FilteredCubeMapRenderTexture()
{

}

DescriptorHeapHandle FilteredCubeMapRenderTexture::GetUnorderedAccessViewHandle(UINT mipIndex, UINT arrayIndex /* = 0 */)
{
	return mUAVHandles[arrayIndex * mNumMips + mipIndex];
}

RayTraceBuffer::RayTraceBuffer(ID3D12Resource* resource, D3D12_RESOURCE_STATES usageState, RayTraceBufferType bufferType, DescriptorHeapHandle srvHandle)
	:GPUResource(resource, usageState)
{
	mGPUAddress = resource->GetGPUVirtualAddress();
	mBufferType = bufferType;
	mSRVHandle = srvHandle;
}

RayTraceBuffer::~RayTraceBuffer()
{

}

void RayTraceBuffer::MapInstanceDescData(const void *instanceDescData, UINT numInstanceDescs)
{
	//Application::Assert(mBufferType == RayTraceBufferType_Instancing);

	void *mappedData;
	mResource->Map(0, NULL, reinterpret_cast<void**>(&mappedData));
	memcpy(mappedData, instanceDescData, numInstanceDescs * sizeof(D3D12_RAYTRACING_INSTANCE_DESC));
	mResource->Unmap(0, NULL);
}

void RayTraceBuffer::MapTransform(const void *transform, UINT sizeOfTransform)
{
	//Application::Assert(mBufferType == RayTraceBufferType_Transform);

	void *mappedData;
	mResource->Map(0, NULL, reinterpret_cast<void**>(&mappedData));
	memcpy(mappedData, transform, sizeOfTransform);
	mResource->Unmap(0, NULL);
}