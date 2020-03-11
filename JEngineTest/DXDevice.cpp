#include "stdafx.h"
#include "DXDevice.h"

using Microsoft::WRL::ComPtr;

DXDevice::DXDevice(IDXGIAdapter * adapter, D3D_FEATURE_LEVEL MaxFeatureLevel)
{
	adapter->GetDesc(&mAdapterDesc);
	bool bIsAMD = mAdapterDesc.VendorId == 0x1002;
	bool bIsIntel = mAdapterDesc.VendorId == 0x8086;
	bool bIsNVIDIA = mAdapterDesc.VendorId == 0x10DE;
	bool bIsWARP = mAdapterDesc.VendorId == 0x1414;
	if (bIsAMD)
		mVendorId = eAMD;
	else if (bIsIntel)
		mVendorId = eINTEL;
	else if (bIsNVIDIA)
		mVendorId = eNVIDIA;
	else if (bIsWARP)
		mVendorId = eWARP;

	// Create Device
	HRESULT res;
	res = D3D12CreateDevice(adapter, MaxFeatureLevel, IID_PPV_ARGS(&mDevice));
	if (!SUCCEEDED(res))
	{
		// Use the default device
		res = D3D12CreateDevice(nullptr, MaxFeatureLevel, IID_PPV_ARGS(&mDevice));
	}

	// Create command queue
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	ThrowIfFailed(mDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCommandQueue)));

	// Create command allocator
	ThrowIfFailed(mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mCommandAllocator)));

	// Create a fence for the command queue 
	ThrowIfFailed(mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence)));
	// Fence handle and initial value
	mCurrentFenceValue = 1;
	mFenceEvent = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);

	// Describe descriptor heaps - RTV and CBV/SRV/UAV
	// RTV면 Render Target, CBV/SRV/UAV면 상수 버퍼, 셰이더 자원, 순서 없는 접근
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = NumRenderTargets + 1;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	D3D12_DESCRIPTOR_HEAP_DESC cbvSrvUavHeapDesc = {};
	cbvSrvUavHeapDesc.NumDescriptors = NumRenderTargets;
	CreateDescriptorHeaps(rtvHeapDesc, cbvSrvUavHeapDesc);// 자원이라 밖에서 생성 해야하나

	// Describe and create the swap chain.
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = FALSE;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapChainDesc.Flags = 0;
	CreateSwapChain(this, swapChainDesc);// 자원이라 밖에서 생성 해야하나
	// 우선 스왑체인은 멀티아답터를 하나 실글 아답터를 해서 멀티 스레드를 하나 스왑체인 객체는 하나이다.
	// 화면에 더블 버퍼링 작업은 스왑체인객체 하나로 해야하닌까?

	// Create frame resources.
	// CreateRenderTargetView 흠.... 아답터 마다 있는거 같다 만약 서브GPU 컴퓨터쉐이더를 돌린다 하더래도 랜더타켓이 있어야 해서??
	// 컴퓨텉쉐이더를 돌릴때 랜더타켓이 필요한지 알아보자
}

DXDevice::~DXDevice()
{
	CloseHandle(mFenceEvent);
}

void DXDevice::CreateDescriptorHeaps(D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc, D3D12_DESCRIPTOR_HEAP_DESC cbvSrvUavHeapDesc)
{
	/*
	D3D12_DESCRIPTOR_HEAP_DESC 구조체는 4개의 간단한 멤버를 가집니다. 

	Type: D3D12_DESCRITOR_HEAP_TYPE 열거형을 받습니다. RTV면 Render Target, CBV_SRV_UAV면 상수 버퍼, 셰이더 자원, 순서 없는 접근의 정의에 대한 각각의 View를 생성할 수 있다는 의미입니다. 이는 공간의 크기를 결정하기 위해 필요한 요소입니다.
	NumDescriptors: 정보체의 개수입니다. 예를 들어, 후면 버퍼 3개에 대해 각각 정보체를 할당하기 위해서 여기에 3을 주어야 합니다.
	Flags: D3D12_DESCRIPTOR_HEAP_FLAGS열거형을 받습니다. 대표적으론 D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE 등이 있으며[AL1] , 말 그대로 셰이더에서 이 정보체를 사용할 수 있다는 의미가 됩니다.
	NodeMask: 어디에나 있는 그 NodeMask가 맞습니다. 대부분의 _DESC 구조체에서 이 값이 등장하며, 멀티 어댑터를 위한 값입니다. 하지만 멀티 어댑터 기능을 사용해도 이 값은 별로 사용하지 않습니다. 아주 특수한 경우에 쓰입니다.
	*/

	// Create the RTV descriptor heap
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(mDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&mRtvHeap)));

	// Set RTV descriptor size
	mRtvDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// Create the CBV / SRV / UAV descriptor heap
	cbvSrvUavHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvSrvUavHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	ThrowIfFailed(mDevice->CreateDescriptorHeap(&cbvSrvUavHeapDesc, IID_PPV_ARGS(&mCbvSrvUavHeap)));

	// Set CBV / SRV / UAV descriptor size
	mCbvSrvUavDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void DXDevice::CreateSwapChain(DXDevice* dxDevice, DXGI_SWAP_CHAIN_DESC1 swapChainDesc)
{
	// Invariant SwapChain properties
	swapChainDesc.Width = gWndProp.width;
	swapChainDesc.Height = gWndProp.height;
	swapChainDesc.BufferCount = NumRenderTargets; // Required because of FLIP SwapEffect

	// Create SwapChain
	IDXGISwapChain1* tempSwapChain1 = nullptr; // Call to CreateSwapChainForHwnd() requires a SwapChain1 object
	ThrowIfFailed(mDXGIFactory->CreateSwapChainForHwnd(dxDevice->mCommandQueue.Get(), gWndProp.hwnd, &swapChainDesc, nullptr, nullptr, &tempSwapChain1));
	ThrowIfFailed(tempSwapChain1->QueryInterface(IID_PPV_ARGS(&(mSwapChain)))); // Copy the temp SwapChain1 into our desired SwapChain3
	tempSwapChain1->Release();

	// Get a handle to the memory location (CPU) of the descriptor heap
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(dxDevice->mRtvHeap->GetCPUDescriptorHandleForHeapStart());
	dxDevice->mRtvDescriptorSize = dxDevice->mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// Create render target and view for each swap chain buffer
	for (UINT s = 0; s < swapChainDesc.BufferCount; s++) {
		// Get buffer render target
		ThrowIfFailed(mSwapChain->GetBuffer(s, IID_PPV_ARGS(&dxDevice->mRenderTargets[s])));
		// Create render target view
		dxDevice->mDevice->CreateRenderTargetView(dxDevice->mRenderTargets[s].Get(), nullptr, rtvHandle);
		// Increment the RTV heap handle
		rtvHandle.Offset(1, dxDevice->mRtvDescriptorSize);
	}
}