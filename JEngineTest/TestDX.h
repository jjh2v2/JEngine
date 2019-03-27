#pragma once
#include "stdafx.h"
#include "DXDevice.h"
#include "Mesh.h"

class TestDX
{
public:
	static const UINT FrameCount = 2;

	struct TestVertex
	{
	public:
		XMFLOAT3 position;
		XMFLOAT4 color;
	};

	// Pipeline objects.
	CD3DX12_VIEWPORT					mViewport;
	CD3DX12_RECT						mScissorRect;
	ComPtr<IDXGIFactory4>				mFactory;
	ComPtr<IDXGISwapChain3>				mSwapChain;
	ComPtr<ID3D12Device>				mDevice;
	ComPtr<ID3D12Resource>				mRenderTargets[FrameCount];
	ComPtr<ID3D12CommandAllocator>		mCommandAllocator;
	ComPtr<ID3D12CommandQueue>			mCommandQueue;
	ComPtr<ID3D12RootSignature>			mRootSignature;
	ComPtr<ID3D12DescriptorHeap>		mRtvHeap;
	UINT								mRtvDescriptorSize;
	ComPtr<ID3D12PipelineState>			mPipelineState;
	ComPtr<ID3D12GraphicsCommandList>	mCommandList;

	// App resources.
	ComPtr<ID3D12Resource>				mVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW			mVertexBufferView;
	ComPtr<ID3D12Resource>				mIndexBuffer;
	D3D12_INDEX_BUFFER_VIEW				mIndexBufferView;
	UINT								indexCount;
	Mesh*								ttttttt;

	// Synchronization objects.
	UINT								mFrameIndex;
	HANDLE								mFenceEvent;
	ComPtr<ID3D12Fence>					mFence;
	UINT64								mFenceValue;

	// Shader
	ComPtr<ID3DBlob>					mVertexShader;
	ComPtr<ID3DBlob>					mPixelShader;

	// Object
	float mAspectRatio = 1.0f;

public:
	TestDX();
	~TestDX();

public:
	void Init();
	void OnUpdate();
	void OnRender();

private:
	void CreateHardwareAdapter( IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter );
	void CreateDevice();
	void CreateCommandQueue();
	void SwapChain();
	void CreateRenderTarget();
	void CreateDepthStencil();
	void CreateShaderResource();
	void CreateRootSignature();
	void CreateShader();
	void CreatePipelineState();
	void CreateCommandAllocator();
	void CreateCommandList();
	// 버텍스 버퍼 생성
	void CreateCommittedResource();
	void CreateFence();
	void WaitForPreviousFrame();
};

