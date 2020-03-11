#pragma once
#include "../../../stdafx.h"

class DescriptorHeapHandle
{
};

class GPUResource
{
public:
	GPUResource(ID3D12Resource *resource, D3D12_RESOURCE_STATES usageState);
	virtual ~GPUResource();

	ID3D12Resource *GetResource() { return mResource; }
	D3D12_GPU_VIRTUAL_ADDRESS GetGpuAddress() { return mGPUAddress; }
	D3D12_RESOURCE_STATES GetUsageState() { return mUsageState; }
	void SetUsageState(D3D12_RESOURCE_STATES usageState) { mUsageState = usageState; }

	bool GetIsReady() { return mIsReady; }
	void SetIsReady(bool isReady) { mIsReady = isReady; }

protected:
	ID3D12Resource *mResource;
	D3D12_GPU_VIRTUAL_ADDRESS mGPUAddress;
	D3D12_RESOURCE_STATES mUsageState;
	bool mIsReady;
};


class TextureResource : public GPUResource
{
public:
	TextureResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES usageState, DescriptorHeapHandle shaderResourceViewHandle);
	~TextureResource() override;

	DescriptorHeapHandle GetShaderResourceViewHandle() { return mShaderResourceViewHandle; }

private:
	DescriptorHeapHandle mShaderResourceViewHandle;
};


class BackBufferTarget : public GPUResource
{
public:
	BackBufferTarget(ID3D12Resource* resource, D3D12_RESOURCE_STATES usageState, DescriptorHeapHandle renderTargetViewHandle);
	~BackBufferTarget() override;

	DescriptorHeapHandle GetRenderTargetViewHandle() { return mRenderTargetViewHandle; }

private:
	DescriptorHeapHandle mRenderTargetViewHandle;
};

class RenderTarget : public GPUResource
{
public:
	struct UAVHandle
	{
		UAVHandle()
		{
			HasUAV = false;
		}

		bool HasUAV;
		DescriptorHeapHandle BaseHandle;
		std::vector<DescriptorHeapHandle> Handles;
	};

	RenderTarget(ID3D12Resource* resource, D3D12_RESOURCE_STATES usageState, D3D12_RESOURCE_DESC renderTargetDesc, DescriptorHeapHandle renderTargetViewHandle,
		std::vector<DescriptorHeapHandle> &renderTargetViewArray, DescriptorHeapHandle shaderResourceViewHandle, const UAVHandle &unorderedAccessView);
	~RenderTarget() override;

	DescriptorHeapHandle GetRenderTargetViewHandle() { return mRenderTargetViewHandle; }
	DescriptorHeapHandle GetRenderTargetViewHandle(UINT index) { return mRenderTargetViewArray[index]; }
	const std::vector<DescriptorHeapHandle> &GetRenderTargetViewArray() { return mRenderTargetViewArray; }
	DescriptorHeapHandle GetShaderResourceViewHandle() { return mShaderResourceViewHandle; }
	DescriptorHeapHandle GetUnorderedAccessViewHandle(UINT mipIndex, UINT arrayIndex = 0);
	DescriptorHeapHandle GetUnorderedAccessViewHandle() { return mUnorderedAccessView.BaseHandle; }

	bool GetHasUAV() { return mUnorderedAccessView.HasUAV; }
	UINT GetArraySize() { return mRenderTargetDesc.DepthOrArraySize; }
	UINT GetMipCount() { return mRenderTargetDesc.MipLevels; }
	UINT GetWidth() { return (UINT)mRenderTargetDesc.Width; }
	UINT GetHeight() { return (UINT)mRenderTargetDesc.Height; }

private:
	D3D12_RESOURCE_DESC	 mRenderTargetDesc;
	DescriptorHeapHandle mRenderTargetViewHandle;
	DescriptorHeapHandle mShaderResourceViewHandle;
	UAVHandle mUnorderedAccessView;
	std::vector<DescriptorHeapHandle> mRenderTargetViewArray;
};

class DepthStencilTarget : public GPUResource
{
public:
	DepthStencilTarget(ID3D12Resource* resource, D3D12_RESOURCE_STATES usageState, D3D12_RESOURCE_DESC depthStencilDesc, DescriptorHeapHandle depthStencilViewHandle, DescriptorHeapHandle readOnlyDepthStencilViewHandle,
		std::vector<DescriptorHeapHandle> &depthStencilViewArray, std::vector<DescriptorHeapHandle> &readOnlyDepthStencilViewArray, DescriptorHeapHandle shaderResourceViewHandle);
	~DepthStencilTarget() override;

	DescriptorHeapHandle GetDepthStencilViewHandle() { return mDepthStencilViewHandle; }
	DescriptorHeapHandle GetReadOnlyDepthStencilViewHandle() { return mReadOnlyDepthStencilViewHandle; }
	DescriptorHeapHandle GetDepthStencilViewHandle(UINT index) { return mDepthStencilViewArray[index]; }
	DescriptorHeapHandle GetReadOnlyDepthStencilViewHandle(UINT index) { return mReadOnlyDepthStencilViewArray[index]; }
	const std::vector<DescriptorHeapHandle> &GetDepthStencilViewArray() { return mDepthStencilViewArray; }
	DescriptorHeapHandle GetShaderResourceViewHandle() { return mShaderResourceViewHandle; }
	UINT GetArraySize() { return mDepthStencilDesc.DepthOrArraySize; }

private:
	D3D12_RESOURCE_DESC	 mDepthStencilDesc;
	DescriptorHeapHandle mDepthStencilViewHandle;
	DescriptorHeapHandle mReadOnlyDepthStencilViewHandle;
	DescriptorHeapHandle mShaderResourceViewHandle;
	std::vector<DescriptorHeapHandle> mDepthStencilViewArray;
	std::vector<DescriptorHeapHandle> mReadOnlyDepthStencilViewArray;
};

class VertexBuffer : public GPUResource
{
public:
	VertexBuffer(ID3D12Resource *resource, D3D12_RESOURCE_STATES usageState, UINT vertexStride, UINT bufferSize);
	~VertexBuffer() override;

	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() { return mVertexBufferView; }

private:
	D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;
};


class IndexBuffer : public GPUResource
{
public:
	IndexBuffer(ID3D12Resource* resource, D3D12_RESOURCE_STATES usageState, UINT bufferSize);
	~IndexBuffer() override;

	D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() { return mIndexBufferView; }

private:
	D3D12_INDEX_BUFFER_VIEW mIndexBufferView;
};


class ConstantBuffer : public GPUResource
{
public:
	ConstantBuffer(ID3D12Resource *resource, D3D12_RESOURCE_STATES usageState, UINT bufferSize, DescriptorHeapHandle constantBufferViewHandle);
	~ConstantBuffer() override;

	void SetConstantBufferData(const void *bufferData, UINT bufferSize);
	DescriptorHeapHandle GetConstantBufferViewHandle() { return mConstantBufferViewHandle; }

private:
	void *mMappedBuffer;
	UINT mBufferSize;
	DescriptorHeapHandle mConstantBufferViewHandle;
};


enum StructuredBufferAccess
{
	GPU_READ_WRITE = 0,
	CPU_WRITE_GPU_READ = 1,
	CPU_WRITE_GPU_READ_WRITE = 2
};

class StructuredBuffer : public GPUResource
{
public:
	StructuredBuffer(ID3D12Resource* resource, ID3D12Resource *uploadResource, D3D12_RESOURCE_STATES usageState, bool isRaw, StructuredBufferAccess accessType, D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc, DescriptorHeapHandle uavHandle, DescriptorHeapHandle srvHandle);
	~StructuredBuffer() override;

	bool CopyToBuffer(const void* bufferData, UINT bufferSize); //returns true if a GPU copy needs to be scheduled
	DescriptorHeapHandle GetUnorderedAccessViewHandle() { return mUnorderedAccessViewHandle; }
	DescriptorHeapHandle GetShaderResourceViewHandle() { return mShaderResourceViewHandle; }

private:
	void *mMappedBuffer;
	ID3D12Resource *mUploadResource;

	bool mIsRaw;
	StructuredBufferAccess mAccessType;
	D3D12_UNORDERED_ACCESS_VIEW_DESC mUAVDesc;
	DescriptorHeapHandle mUnorderedAccessViewHandle;
	DescriptorHeapHandle mShaderResourceViewHandle;
};

class FilteredCubeMapRenderTexture : public GPUResource
{
public:
	FilteredCubeMapRenderTexture(ID3D12Resource* resource, D3D12_RESOURCE_STATES usageState, UINT numMipLevels, const std::vector<DescriptorHeapHandle> &uavHandles, DescriptorHeapHandle srvHandle);
	~FilteredCubeMapRenderTexture() override;

	DescriptorHeapHandle GetShaderResourceViewHandle() { return mSRVHandle; }
	DescriptorHeapHandle GetUnorderedAccessViewHandle(UINT mipIndex, UINT arrayIndex = 0);
	UINT GetMipCount() { return mNumMips; }

	void SetComputeFence(UINT64 computeFence) { mComputeFence = computeFence; }
	UINT64 GetComputeFence() { return mComputeFence; }

private:
	UINT64 mComputeFence;
	UINT mNumMips;
	std::vector<DescriptorHeapHandle> mUAVHandles;
	DescriptorHeapHandle mSRVHandle;
};

class RayTraceBuffer : public GPUResource
{
public:
	enum RayTraceBufferType
	{
		RayTraceBufferType_Acceleration_Structure = 0,
		RayTraceBufferType_Instancing,
		RayTraceBufferType_Shader_Binding_Table_Storage,
		RayTraceBufferType_Transform
	};

	RayTraceBuffer(ID3D12Resource* resource, D3D12_RESOURCE_STATES usageState, RayTraceBufferType bufferType, DescriptorHeapHandle srvHandle);
	~RayTraceBuffer() override;
	void MapInstanceDescData(const void *instanceDescData, UINT numInstanceDescs);
	void MapTransform(const void *transform, UINT sizeOfTransform);
	DescriptorHeapHandle GetShaderResourceViewHandle() { return mSRVHandle; }

private:
	RayTraceBufferType mBufferType;
	DescriptorHeapHandle mSRVHandle;
};