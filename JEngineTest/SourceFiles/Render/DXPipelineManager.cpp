#include "DXPipelineManager.h"

void DXPipelineManager::Init(UINT& rFrameIndex, UINT iFrameCount /*= 2*/)
{
	CreateDevice();
	CreateCommandQueue();
	CreateCommandAllocator();
	CreateCommandList();

	CreateSwapChain(rFrameIndex, iFrameCount);
	CreateRenderTarget(iFrameCount);
}

void DXPipelineManager::CreateHardwareAdapter(IDXGIAdapter1** ppAdapter)
{
	ComPtr<IDXGIAdapter1> adapter;
	*ppAdapter = nullptr;

	for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != mpFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			continue;
		}

		ComPtr<ID3D12Device> _device;
		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&_device))))
		{
			break;
		}
	}

	*ppAdapter = adapter.Detach();
}

void DXPipelineManager::CreateDevice()
{
	UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
	// 디버그 레이어를 활성화합니다 (그래픽 도구 "선택적 기능"필요).
	// 참고 : 장치 생성 후 디버그 레이어를 활성화하면 활성 장치가 무효화됩니다.
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();

			// 디버그 레이어 옵셥 플래그 설정
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif

	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&mpFactory)));

	{
		// 디바이스 생성전에 하드웨어 가져오기
		ComPtr<IDXGIAdapter1> hardwareAdapter;
		CreateHardwareAdapter(&hardwareAdapter);
		// 디바이스 생성
		ThrowIfFailed(D3D12CreateDevice(
			hardwareAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&mpDevice)
		));
	}
}

void DXPipelineManager::CreateCommandQueue()
{
	// DESC 약자가 들거간 객체는 DX객체를 생성할때 필요한 정보로서 비트플래그 형식으로
	// 각각의 객체를 생성할때 정보를 만들어서 넘기는 객체이다
	// 한마디로 객체 생성에 필요한 정보를 만들어서 넘긴다 그것이 DESC객체이다
	// 커맨드큐 디스크립션(생성정보) 설정 
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	// 커매드큐 생서
	ThrowIfFailed(mpDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mpCommandQueue)));
}

void DXPipelineManager::CreateCommandAllocator()
{
	ThrowIfFailed(mpDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mpCommandAllocator)));
}
void DXPipelineManager::CreateCommandList()
{
	//ThrowIfFailed(mpDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mpCommandAllocator.Get(), mPipelineState.Get(), IID_PPV_ARGS(&mpCommandList)));

	ThrowIfFailed(mpDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mpCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&mpCommandList)));
	// Command lists are created in the recording state, but there is nothing
	// to record yet. The main loop expects it to be closed, so close it now.
	//ThrowIfFailed(mCommandList->Close());
}

void DXPipelineManager::CreateSwapChain(UINT& rFrameIndex, UINT iFrameCount /*= 2*/)
{
	// 스왑체인 디스크립션(생성정보) 설정
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = iFrameCount;
	swapChainDesc.Width = gWndProp.width;
	swapChainDesc.Height = gWndProp.height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	// 스왑체인 생성
	ComPtr<IDXGISwapChain1> swapChain;
	ThrowIfFailed(mpFactory->CreateSwapChainForHwnd(
		mpCommandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
		gWndProp.hwnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain
	));

	// ALT + Enter로 전체 화면 막는다
	ThrowIfFailed(mpFactory->MakeWindowAssociation(gWndProp.hwnd, DXGI_MWA_NO_ALT_ENTER));

	// 스왑체인에 현제 백버퍼 인덱스 가져온다
	ThrowIfFailed(swapChain.As(&mpSwapChain));
	rFrameIndex = mpSwapChain->GetCurrentBackBufferIndex();
}

void DXPipelineManager::CreateRenderTarget(UINT iFrameCount /*= 2*/)
{
	// 랜더타켓의 서술자힙(리소스뷰) 생성
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = iFrameCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(mpDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&mpRtvHeap)));

	miRtvDescriptorSize = mpDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// 랜더타켓뷰 디스크립션힙의 첫번째 위치를 핸들로 받는다
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mpRtvHeap->GetCPUDescriptorHandleForHeapStart());

	// FrameCount만큼 랜더타켓(리소스)을 생성 한다
	for (UINT n = 0; n < iFrameCount; n++)
	{
		ComPtr<ID3D12Resource> rtt;
		ThrowIfFailed(mpSwapChain->GetBuffer(n, IID_PPV_ARGS(&rtt)));
		mvRenderTargets.push_back(rtt);
		mpDevice->CreateRenderTargetView(mvRenderTargets[n].Get(), nullptr, rtvHandle);
		rtvHandle.Offset(1, miRtvDescriptorSize);
	}
}