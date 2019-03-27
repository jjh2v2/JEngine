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
	// RTV�� Render Target, CBV/SRV/UAV�� ��� ����, ���̴� �ڿ�, ���� ���� ����
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = NumRenderTargets + 1;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	D3D12_DESCRIPTOR_HEAP_DESC cbvSrvUavHeapDesc = {};
	cbvSrvUavHeapDesc.NumDescriptors = NumRenderTargets;
	CreateDescriptorHeaps(rtvHeapDesc, cbvSrvUavHeapDesc);// �ڿ��̶� �ۿ��� ���� �ؾ��ϳ�

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
	CreateSwapChain(this, swapChainDesc);// �ڿ��̶� �ۿ��� ���� �ؾ��ϳ�
	// �켱 ����ü���� ��Ƽ�ƴ��͸� �ϳ� �Ǳ� �ƴ��͸� �ؼ� ��Ƽ �����带 �ϳ� ����ü�� ��ü�� �ϳ��̴�.
	// ȭ�鿡 ���� ���۸� �۾��� ����ü�ΰ�ü �ϳ��� �ؾ��ϴѱ�?

	// Create frame resources.
	// CreateRenderTargetView ��.... �ƴ��� ���� �ִ°� ���� ���� ����GPU ��ǻ�ͽ��̴��� ������ �ϴ����� ����Ÿ���� �־�� �ؼ�??
	// ��ǻ�����̴��� ������ ����Ÿ���� �ʿ����� �˾ƺ���
}

DXDevice::~DXDevice()
{
	CloseHandle(mFenceEvent);
}

void DXDevice::CreateDescriptorHeaps(D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc, D3D12_DESCRIPTOR_HEAP_DESC cbvSrvUavHeapDesc)
{
	/*
	D3D12_DESCRIPTOR_HEAP_DESC ����ü�� 4���� ������ ����� �����ϴ�. 

	Type: D3D12_DESCRITOR_HEAP_TYPE �������� �޽��ϴ�. RTV�� Render Target, CBV_SRV_UAV�� ��� ����, ���̴� �ڿ�, ���� ���� ������ ���ǿ� ���� ������ View�� ������ �� �ִٴ� �ǹ��Դϴ�. �̴� ������ ũ�⸦ �����ϱ� ���� �ʿ��� ����Դϴ�.
	NumDescriptors: ����ü�� �����Դϴ�. ���� ���, �ĸ� ���� 3���� ���� ���� ����ü�� �Ҵ��ϱ� ���ؼ� ���⿡ 3�� �־�� �մϴ�.
	Flags: D3D12_DESCRIPTOR_HEAP_FLAGS�������� �޽��ϴ�. ��ǥ������ D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE ���� ������[AL1] , �� �״�� ���̴����� �� ����ü�� ����� �� �ִٴ� �ǹ̰� �˴ϴ�.
	NodeMask: ��𿡳� �ִ� �� NodeMask�� �½��ϴ�. ��κ��� _DESC ����ü���� �� ���� �����ϸ�, ��Ƽ ����͸� ���� ���Դϴ�. ������ ��Ƽ ����� ����� ����ص� �� ���� ���� ������� �ʽ��ϴ�. ���� Ư���� ��쿡 ���Դϴ�.
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