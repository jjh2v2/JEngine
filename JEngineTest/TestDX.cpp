#include "stdafx.h"
#include "TestDX.h"

/*******************************************
������ �Ҹ���
*******************************************/
TestDX::TestDX()
{
}

TestDX::~TestDX()
{
}

/*******************************************
public
*******************************************/
void TestDX::Init()
{
	mViewport.Width = static_cast<float>(gWndProp.width);
	mViewport.Height = static_cast<float>(gWndProp.height);
	mViewport.MaxDepth = 1.0f;
	mScissorRect.right = static_cast<LONG>(gWndProp.width);
	mScissorRect.bottom = static_cast<LONG>(gWndProp.height);

	mAspectRatio = static_cast<float>(gWndProp.width) / static_cast<float>(gWndProp.height);

	CreateDevice();
	CreateCommandQueue();
	SwapChain();
	CreateRenderTarget();
	CreateDepthStencil();
	CreateShaderResource();
	CreateRootSignature();
	CreateShader();
	CreatePipelineState();
	CreateCommandAllocator();
	CreateCommandList();
	CreateCommittedResource();
	CreateFence();
	//��ɱ���� �������� �˸���
	ThrowIfFailed(mCommandList->Close());
}

void TestDX::OnUpdate()
{

}

void TestDX::OnRender()
{
	// ����� �������ϴ� �� �ʿ��� ��� ����� ��� ��Ͽ� ����մϴ�.
	//PopulateCommandList();
	// ��ɸ���Ʈ �Ҵ� �ڵ��� ������
	// ��� ����� GPU���� ������ ���ƽ��ϴ�. �ۿ��� ����ؾ��ϴ�
	// GPU ���� ������ �����ϴ� ��Ÿ��.
	ThrowIfFailed(mCommandAllocator->Reset());

	// �׷��� Ư�� ��ɿ��� ExecuteCommandList()�� ȣ��Ǹ�
	// ��Ͽ��� �������� ��� ����� �缳�� �� �� �ֽ��ϴ�.
	// �ٽ� �����Ͻʽÿ�.
	ThrowIfFailed(mCommandList->Reset(mCommandAllocator.Get(), mPipelineState.Get()));

	// Set necessary state.
	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());
	mCommandList->RSSetViewports(1, &mViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	// Indicate that the back buffer will be used as a render target.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mRenderTargets[mFrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), mFrameIndex, mRtvDescriptorSize);
	mCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	// Record commands.
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	mCommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//mCommandList->IASetVertexBuffers(0, 1, &mVertexBufferView);
	//mCommandList->DrawInstanced(3, 1, 0, 0);
	//mCommandList->IASetVertexBuffers(0, 1, &mVertexBufferView);
	//mCommandList->IASetIndexBuffer(&mIndexBufferView);
	//mCommandList->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);
	mCommandList->IASetVertexBuffers(0, 1, &ttttttt->GetVertexBufferView());
	mCommandList->IASetIndexBuffer(&ttttttt->GetIndexBufferView());
	mCommandList->DrawIndexedInstanced(ttttttt->GetIndexCount(), 1, 0, 0, 0);

	// Indicate that the back buffer will now be used to present.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mRenderTargets[mFrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	//��ɱ���� �������� �˸���
	ThrowIfFailed(mCommandList->Close());

	// Execute the command list.
	ID3D12CommandList* ppCommandLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// ���۹� ����Ʈ ���� ��ü
	ThrowIfFailed(mSwapChain->Present(1, 0));

	WaitForPreviousFrame();
}

/*******************************************
private
*******************************************/

void TestDX::CreateHardwareAdapter( IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter )
{
	ComPtr<IDXGIAdapter1> adapter;
	*ppAdapter = nullptr;

	for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			continue;
		}

		ComPtr<ID3D12Device> _device;
		if ( SUCCEEDED( D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&_device))) )
		{
			break;
		}
	}

	*ppAdapter = adapter.Detach();
}

void TestDX::CreateDevice()
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

	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&mFactory)));

	{
		// ����̽� �������� �ϵ���� ��������
		ComPtr<IDXGIAdapter1> hardwareAdapter;
		CreateHardwareAdapter(mFactory.Get(), &hardwareAdapter);
		// ����̽� ����
		ThrowIfFailed(D3D12CreateDevice(
			hardwareAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&mDevice)
		));
	}
}

void TestDX::CreateCommandQueue()
{
	// DESC ���ڰ� ��Ű� ��ü�� DX��ü�� �����Ҷ� �ʿ��� �����μ� ��Ʈ�÷��� ��������
	// ������ ��ü�� �����Ҷ� ������ ���� �ѱ�� ��ü�̴�
	// �Ѹ���� ��ü ������ �ʿ��� ������ ���� �ѱ�� �װ��� DESC��ü�̴�
	// Ŀ�ǵ�ť ��ũ����(��������) ���� 
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	// Ŀ�ŵ�ť ����
	ThrowIfFailed(mDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCommandQueue)));
}

void TestDX::SwapChain()
{
	// ����ü�� ��ũ����(��������) ����
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = FrameCount;
	swapChainDesc.Width = gWndProp.width;
	swapChainDesc.Height = gWndProp.height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	// ����ü�� ����
	ComPtr<IDXGISwapChain1> swapChain;
	ThrowIfFailed(mFactory->CreateSwapChainForHwnd(
		mCommandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
		gWndProp.hwnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain
	));

	// ALT + Enter�� ��ü ȭ�� ���´�
	ThrowIfFailed(mFactory->MakeWindowAssociation(gWndProp.hwnd, DXGI_MWA_NO_ALT_ENTER));

	// ����ü�ο� ���� ����� �ε��� �����´�
	ThrowIfFailed(swapChain.As(&mSwapChain));
	mFrameIndex = mSwapChain->GetCurrentBackBufferIndex();
}

void TestDX::CreateRenderTarget()
{
	// ����Ÿ���� ��������(���ҽ���) ����
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = FrameCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(mDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&mRtvHeap)));

	mRtvDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// ����Ÿ�Ϻ� ��ũ�������� ù��° ��ġ�� �ڵ�� �޴´�
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());

	// FrameCount��ŭ ����Ÿ��(���ҽ�)�� ���� �Ѵ�
	for (UINT n = 0; n < FrameCount; n++)
	{
		ThrowIfFailed(mSwapChain->GetBuffer(n, IID_PPV_ARGS(&mRenderTargets[n])));
		mDevice->CreateRenderTargetView(mRenderTargets[n].Get(), nullptr, rtvHandle);
		rtvHandle.Offset(1, mRtvDescriptorSize);
	}
}

void TestDX::CreateDepthStencil()
{

}

void TestDX::CreateShaderResource()
{
	
}

void TestDX::CreateRootSignature()
{
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
	ThrowIfFailed(mDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&mRootSignature)));
}

void TestDX::CreateShader()
{
#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	ThrowIfFailed(D3DCompileFromFile(L"shaders_0.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &mVertexShader, nullptr));
	ThrowIfFailed(D3DCompileFromFile(L"shaders_0.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &mPixelShader, nullptr));

}

void TestDX::CreatePipelineState()
{
	// Define the vertex input layout.
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	// Describe and create the graphics pipeline state object (PSO).
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
	psoDesc.pRootSignature = mRootSignature.Get();
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(mVertexShader.Get());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(mPixelShader.Get());
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = FALSE;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;
	ThrowIfFailed(mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPipelineState)));
}

void TestDX::CreateCommandAllocator()
{
	ThrowIfFailed(mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mCommandAllocator)));
}

void TestDX::CreateCommandList()
{
	ThrowIfFailed(mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mCommandAllocator.Get(), mPipelineState.Get(), IID_PPV_ARGS(&mCommandList)));

	// Command lists are created in the recording state, but there is nothing
	// to record yet. The main loop expects it to be closed, so close it now.
	//ThrowIfFailed(mCommandList->Close());
}

void TestDX::CreateCommittedResource()
{
	ttttttt = new Mesh("square",mDevice.Get(), mCommandList.Get());
}

void TestDX::CreateFence()
{
	ThrowIfFailed(mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence)));
	mFenceValue = 1;

	// Create an event handle to use for frame synchronization.
	mFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (mFenceEvent == nullptr)
	{
		ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
	}

	// Wait for the command list to execute; we are reusing the same command 
	// list in our main loop but for now, we just want to wait for setup to 
	// complete before continuing.
	WaitForPreviousFrame();
}

void TestDX::WaitForPreviousFrame()
{

	// ����ϱ� ���� �������� ��ٷȴٰ� ����ϴ� ���� ���� �ٶ������� �ʽ��ϴ�.
	// �̰��� �ܼ�ȭ�� ���� ���� �� �ڵ��Դϴ�. D3D12HelloFrameBuffering
	// ȿ������ �ڿ� ����� ���� �潺�� ����ϴ� ����� �����ִ� �����Դϴ�.
	// GPU Ȱ�뵵�� �ش�ȭ�մϴ�.

	// ��Ÿ�� ���� ��ȣ�ϰ� ������ŵ�ϴ�.
	const UINT64 fence = mFenceValue;
	ThrowIfFailed(mCommandQueue->Signal(mFence.Get(), fence));
	mFenceValue++;

	// ���� �������� ���� ������ ��ٸ��ϴ�.
	if (mFence->GetCompletedValue() < fence)
	{
		ThrowIfFailed(mFence->SetEventOnCompletion(fence, mFenceEvent));
		WaitForSingleObject(mFenceEvent, INFINITE);
	}

	mFrameIndex = mSwapChain->GetCurrentBackBufferIndex();
}