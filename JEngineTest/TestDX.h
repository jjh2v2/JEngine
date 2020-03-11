#pragma once
#include "stdafx.h"
#include "DXDevice.h"
#include "Mesh.h"
#include "Camera.h"
#include "timerclass.h"
#include "JInput.h"
#include "SourceFiles/Render/DXPipelineManager.h"
#include "SourceFiles/Render/DXAssetsManager.h"
#include "SourceFiles/Render/Buffer/ConstantBuffer.h"
#include "SourceFiles/DirectXTK12/WICTextureLoader.h"
#include "SourceFiles/DirectXTK12/DDSTextureLoader.h"
#include "SourceFiles/DirectXTK12/ResourceUploadBatch.h"
#include "SourceFiles/Common/d3dUtil.h"

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
	//ComPtr<ID3D12Resource>				mRenderTargets[FrameCount];
	std::vector<ComPtr<ID3D12Resource>>	mRenderTargets;
	ComPtr<ID3D12CommandAllocator>		mCommandAllocator;
	ComPtr<ID3D12CommandQueue>			mCommandQueue;
	ComPtr<ID3D12RootSignature>			mRootSignature;
	ComPtr<ID3D12DescriptorHeap>		mRtvHeap;
	UINT								mRtvDescriptorSize;
	ComPtr<ID3D12PipelineState>			mPipelineState;
	ComPtr<ID3D12GraphicsCommandList>	mCommandList;

	//
	ComPtr<ID3D12DescriptorHeap>		mCBVHeap;
	ComPtr<ID3D12Resource>				mConstantBuffer;
	BYTE*								mPCBVDataBegin;
	SceneConstantBuffer					mConstantBufferData;

	// App resources.
	ComPtr<ID3D12Resource>				mVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW			mVertexBufferView;
	ComPtr<ID3D12Resource>				mIndexBuffer;
	D3D12_INDEX_BUFFER_VIEW				mIndexBufferView;
	UINT								indexCount;
	std::vector<Mesh*>					mObj;

	// Synchronization objects.
	UINT								mFrameIndex;
	HANDLE								mFenceEvent;
	ComPtr<ID3D12Fence>					mFence;
	UINT64								mFenceValue;

	// Shader
	ComPtr<ID3DBlob>					mVertexShader;
	ComPtr<ID3DBlob>					mPixelShader;

	// 
	std::unordered_map<std::string, std::unique_ptr<Texture>> mTextures;

	// Object
	float mAspectRatio = 1.0f;

	// Camera Timer
	
	Camera* mCamera;
public:
	TestDX();
	~TestDX();

public:
	void Init();
	void OnUpdate(float deltaTime);
	void OnRender();

private:
	void WaitForPreviousFrame();
	void Test();

public:
	void HandleWindowsCallback(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

