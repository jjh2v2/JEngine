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
	// ����� ���̾ Ȱ��ȭ�մϴ� (�׷��� ���� "������ ���"�ʿ�).
	// ���� : ��ġ ���� �� ����� ���̾ Ȱ��ȭ�ϸ� Ȱ�� ��ġ�� ��ȿȭ�˴ϴ�.
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();

			// ����� ���̾� �ɼ� �÷��� ����
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif

	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&mpFactory)));

	{
		// ����̽� �������� �ϵ���� ��������
		ComPtr<IDXGIAdapter1> hardwareAdapter;
		CreateHardwareAdapter(&hardwareAdapter);
		// ����̽� ����
		ThrowIfFailed(D3D12CreateDevice(
			hardwareAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&mpDevice)
		));
	}
}

void DXPipelineManager::CreateCommandQueue()
{
	// DESC ���ڰ� ��Ű� ��ü�� DX��ü�� �����Ҷ� �ʿ��� �����μ� ��Ʈ�÷��� ��������
	// ������ ��ü�� �����Ҷ� ������ ���� �ѱ�� ��ü�̴�
	// �Ѹ���� ��ü ������ �ʿ��� ������ ���� �ѱ�� �װ��� DESC��ü�̴�
	// Ŀ�ǵ�ť ��ũ����(��������) ���� 
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	// Ŀ�ŵ�ť ����
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
	// ����ü�� ��ũ����(��������) ����
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = iFrameCount;
	swapChainDesc.Width = gWndProp.width;
	swapChainDesc.Height = gWndProp.height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	// ����ü�� ����
	ComPtr<IDXGISwapChain1> swapChain;
	ThrowIfFailed(mpFactory->CreateSwapChainForHwnd(
		mpCommandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
		gWndProp.hwnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain
	));

	// ALT + Enter�� ��ü ȭ�� ���´�
	ThrowIfFailed(mpFactory->MakeWindowAssociation(gWndProp.hwnd, DXGI_MWA_NO_ALT_ENTER));

	// ����ü�ο� ���� ����� �ε��� �����´�
	ThrowIfFailed(swapChain.As(&mpSwapChain));
	rFrameIndex = mpSwapChain->GetCurrentBackBufferIndex();
}

void DXPipelineManager::CreateRenderTarget(UINT iFrameCount /*= 2*/)
{
	// ����Ÿ���� ��������(���ҽ���) ����
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = iFrameCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(mpDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&mpRtvHeap)));

	miRtvDescriptorSize = mpDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// ����Ÿ�Ϻ� ��ũ�������� ù��° ��ġ�� �ڵ�� �޴´�
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mpRtvHeap->GetCPUDescriptorHandleForHeapStart());

	// FrameCount��ŭ ����Ÿ��(���ҽ�)�� ���� �Ѵ�
	for (UINT n = 0; n < iFrameCount; n++)
	{
		ComPtr<ID3D12Resource> rtt;
		ThrowIfFailed(mpSwapChain->GetBuffer(n, IID_PPV_ARGS(&rtt)));
		mvRenderTargets.push_back(rtt);
		mpDevice->CreateRenderTargetView(mvRenderTargets[n].Get(), nullptr, rtvHandle);
		rtvHandle.Offset(1, miRtvDescriptorSize);
	}
}