//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "stdafx.h"
#include "DX12SampleBox.h"

void DX12SampleBox::OnDestroy()
{
	// Ensure that the GPU is no longer referencing resources that are about to be
	// cleaned up by the destructor.
	WaitForPreviousFrame();

	CloseHandle(m_fenceEvent);
}

DX12SampleBox::DX12SampleBox(UINT width, UINT height, std::wstring name) :
    DXSample(width, height, name),
    m_frameIndex(0),
    m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
    m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height)),
    m_rtvDescriptorSize(0)
{
}

void DX12SampleBox::OnInit()
{
    LoadPipeline();
    LoadAssets();
}

// Load the rendering pipeline dependencies.
void DX12SampleBox::LoadPipeline()
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

    ComPtr<IDXGIFactory4> factory;
    ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

    {
		// ����̽� �������� �ϵ���� ��������
        ComPtr<IDXGIAdapter1> hardwareAdapter;
        GetHardwareAdapter(factory.Get(), &hardwareAdapter);
		// ����̽� ����
        ThrowIfFailed(D3D12CreateDevice(
            hardwareAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&m_device)
            ));
    }

	// DESC ���ڰ� ��Ű� ��ü�� DX��ü�� �����Ҷ� �ʿ��� �����μ� ��Ʈ�÷��� ��������
	// ������ ��ü�� �����Ҷ� ������ ���� �ѱ�� ��ü�̴�
	// �Ѹ���� ��ü ������ �ʿ��� ������ ���� �ѱ�� �װ��� DESC��ü�̴�
    // Ŀ�ǵ�ť ��ũ����(��������) ���� 
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	// Ŀ�ŵ�ť ����
    ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

    // ����ü�� ��ũ����(��������) ����
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = FrameCount;
    swapChainDesc.Width = m_width;
    swapChainDesc.Height = m_height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

	// ����ü�� ����
    ComPtr<IDXGISwapChain1> swapChain;
    ThrowIfFailed(factory->CreateSwapChainForHwnd(
        m_commandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
        Win32Application::GetHwnd(),
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain
        ));

    // ALT + Enter�� ��ü ȭ�� ���´�
    ThrowIfFailed(factory->MakeWindowAssociation(Win32Application::GetHwnd(), DXGI_MWA_NO_ALT_ENTER));

	// ����ü�ο� ���� ����� �ε��� �����´�
    ThrowIfFailed(swapChain.As(&m_swapChain));
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	/*********************************************************************
	�׸��� ����� ���� �ϱ����� �ش� �׸��� ����� ������ ���ҽ����� ������ ���������ο� �������Ѵ�
	�׷��� ���ҽ����� ���������ο� ���� ���̴°��� �ƴϴ�
	������ ���������ο� ���̴� ���� �ش� ���ҽ��� �����ϴ� ������( Descriptor)�̴�

	�����ڴ� ���ҽ��� ������ ��ġ�� ũ�ⰰ�� ���ҽ��� ������ ���� ���ҽ��� GPU ������ �������̴�

	���ҽ� -> ������ -> GPU
	GPU -> ������ -> ���ҽ�

	������( Descriptor)�ڵ��� ��ġ�� �߰����� �������� �δ� ������ �ڿ��̶�°��� �������̶��̴�
	���� �ڿ��� ������ ������������ ���� �ٸ� �ܰ�(stage)�鿡�� ����Ҽ��ִ�

	�ؽ��ĸ� ����Ÿ�Ͽ��� ����ϰ� ������ �ܰ迡�� ���̴� �ڿ����� ��� �ϴ°��̴�

	���ҽ� ��ü�� �ڽ��� ����Ÿ�ٿ��� ���̴��� ���̴����� ����ϴ����� ���ؼ� �𸥴�

	������( Descriptor)�� DirectD3D���� �ڿ��� ��� ����� �˷��ش�
	� ���������� �ܰ迡 ������ �ϴ���

	���� ������ DirectD3D ���ҽ��� ��� �ҷ����ϴ�

	��������( Descriptor Heap)�� �����ڵ��� �迭�̴�
	�������α׷�(CPU)�� ����ϴ� �����ڵ��� ����Ǵ� ���� �ٷ� ���������ϴ�
	*********************************************************************/
    // Create descriptor heaps.
    {
        // ����Ÿ���� ��������(���ҽ���) ����
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = FrameCount;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

        m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    }

    // Create frame resources.
    {
		// ����Ÿ�Ϻ� ��ũ�������� ù��° ��ġ�� �ڵ�� �޴´�
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

        // FrameCount��ŭ ����Ÿ��(���ҽ�)�� ���� �Ѵ�
        for (UINT n = 0; n < FrameCount; n++)
        {
            ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
            m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, m_rtvDescriptorSize);
        }
    }

	/*********************************************************************
	��� ���(command list)
	����Ʈ�� �����ϰ� ����Ÿ���� ����� �׸��� ȣ���� �����϶�� ����� ����ϴ� ��ü

	�޸� �Ҵ���(CommandAllocator)
	��� ����� ���(����)�� ���� �޸� ��ü

	��� ��⿭(command queue)
	������ ��ɵ��� ť������ �ڷ�� ��� �ִٰ� GPU�� ����� ������ ��ü?

	��� ����� ����� �������� ID3D12GraphicsCommandList::Close ȣ���Ͽ� ��ɱ���� �������� �˸���

	ID3D12CommandQueue::ExecuteCommandLists�� ��� ����� ���� �Ѵ� �����Ҷ���
	�޸� �Ҵ���(CommandAllocator)���� ��ɵ��� �����Ѵ�

	ID3D12CommandQueue::ExecuteCommandListsȣ����
	ID3D12GraphicsCommandList::Reset�Ѵٸ� ID3D12GraphicsCommandList�� ���� ������ �ʰ�
	��� ��ϵ��� �ٽ� ��� �Ҽ��ִ� Reset�� ó�� ������������ ���� ���·� �����

	�ϳ��� �������� �ϼ��ϴ� �� �ʿ��� ������ ��ɵ��� ��� GPU�� ���� �� �Ŀ���
	CommandAllocator::Reset�Ͽ� ���� �Ѵ�
	std::vector::clear ȣ��� ����ϴ�

	GPU�� ����� �Ҵ��ڿ� ��� ��� ����� ���������� Ȯ�������� ������ ��� �Ҵ��ڸ� Reset�ϸ� �ȵȴ�
	�׷��� ����� ��� �ϱ� ������
	CommandAllocator::Reset , ID3D12GraphicsCommandList::Reset�ϴ°� ����
	*********************************************************************/
    ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));

	// Create the command list.
	ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList)));

	// Command lists are created in the recording state, but there is nothing
	// to record yet. The main loop expects it to be closed, so close it now.
	ThrowIfFailed(m_commandList->Close());
}

// Load the sample assets.
void DX12SampleBox::LoadAssets()
{
    // Create an empty root signature.
    {
		/*
		root signature(��Ʈ ����)�� �׸��� ȣ�� ���� �������α׷��� �ݵ�� ������ ���������ο� ����� �ϴ� �ڿ����� ����
		�� �ڿ����� ���̴� �Է� �������͵鿡�� ��� ���� �Ǵ����� �����Ѵ�.
		init�� �Ҷ� D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT �̶�� �÷��׸� �־��µ�
		�� �÷��״� "���� �Է� ����� (���ؽ� ���� ���ε� ��Ʈ�� �����ϴ� �Է� ���̾ƿ��� �ʿ���)�� ����ϵ��� �����մϴ�. 
		�� �÷��׸� �����ϸ� �ϳ��� ��Ʈ �μ� ������ �Ϻ� �ϵ��� ���� �� �� �ֽ��ϴ�. 
		�Է� ������� �ʿ����� ���� ��쿡���� �÷��׸� �����Ͻʽÿ�.
		��, ����ȭ�� �߿����� �ʽ��ϴ�." ��� �Ѵ�.... ���õ� �κ��� MSDN�� �ѹ� ���캸�� ( ��ũ )

		�ƹ�ư ID3DBlob �̶�� ������ signature�� error �� �����ߴµ� Interface D3D Blob(Binary Large Object, [���? �̶�� �г�?]) ��� ���̴�
		*/
        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
        ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
    }

    // Create the pipeline state, which includes compiling and loading shaders.
    {
		/*********************************************************************
		������ �غ� �������� ���ҽ����� �غ� ���� �ϰ� �װ�ü���� ������ ����ϱ� ���ؼ� ������ ������������ ����� �ϴµ� 
		������ ������ ���� ��ü�� �����Ѵ� ���̴� ��Ʈ�ñ״��� ���
		*********************************************************************/

        ComPtr<ID3DBlob> vertexShader;
        ComPtr<ID3DBlob> pixelShader;

#if defined(_DEBUG)
        // Enable better shader debugging with the graphics debugging tools.
        UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        UINT compileFlags = 0;
#endif

        ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"shaders.hlsl").c_str(), nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
        ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"shaders.hlsl").c_str(), nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));

        // Define the vertex input layout.
        D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };

        // Describe and create the graphics pipeline state object (PSO).
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
        psoDesc.pRootSignature = m_rootSignature.Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState.DepthEnable = FALSE;
        psoDesc.DepthStencilState.StencilEnable = FALSE;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;
        ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
    }

    // Create the vertex buffer.
    {
        // Define the geometry for a triangle.
        Vertex triangleVertices[] =
        {
            { { 0.0f, 0.25f * m_aspectRatio, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
            { { 0.25f, -0.25f * m_aspectRatio, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
            { { -0.25f, -0.25f * m_aspectRatio, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
        };

        const UINT vertexBufferSize = sizeof(triangleVertices);

        // Note: using upload heaps to transfer static data like vert buffers is not 
        // recommended. Every time the GPU needs it, the upload heap will be marshalled 
        // over. Please read up on Default Heap usage. An upload heap is used here for 
        // code simplicity and because there are very few verts to actually transfer.
        ThrowIfFailed(m_device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_vertexBuffer)));

        // Copy the triangle data to the vertex buffer.
        UINT8* pVertexDataBegin;
        CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
        ThrowIfFailed(m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
        memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
        m_vertexBuffer->Unmap(0, nullptr);

        // Initialize the vertex buffer view.
        m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
        m_vertexBufferView.StrideInBytes = sizeof(Vertex);
        m_vertexBufferView.SizeInBytes = vertexBufferSize;
    }

    // Create synchronization objects and wait until assets have been uploaded to the GPU.
    {
        ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
        m_fenceValue = 1;

        // Create an event handle to use for frame synchronization.
        m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (m_fenceEvent == nullptr)
        {
            ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
        }

        // Wait for the command list to execute; we are reusing the same command 
        // list in our main loop but for now, we just want to wait for setup to 
        // complete before continuing.
        WaitForPreviousFrame();
    }
}

// Update frame-based values.
void DX12SampleBox::OnUpdate()
{
}

// Render the scene.
void DX12SampleBox::OnRender()
{
	// ����� �������ϴ� �� �ʿ��� ��� ����� ��� ��Ͽ� ����մϴ�.
    PopulateCommandList();

    // Execute the command list.
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // ���۹� ����Ʈ ���� ��ü
    ThrowIfFailed(m_swapChain->Present(1, 0));

    WaitForPreviousFrame();
}

void DX12SampleBox::PopulateCommandList()
{
	// ��ɸ���Ʈ �Ҵ� �ڵ��� ������
	// ��� ����� GPU���� ������ ���ƽ��ϴ�. �ۿ��� ����ؾ��ϴ�
	// GPU ���� ������ �����ϴ� ��Ÿ��.
    ThrowIfFailed(m_commandAllocator->Reset());

	// �׷��� Ư�� ��ɿ��� ExecuteCommandList()�� ȣ��Ǹ�
	// ��Ͽ��� �������� ��� ����� �缳�� �� �� �ֽ��ϴ�.
	// �ٽ� �����Ͻʽÿ�.
    ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get()));

    // Set necessary state.
    m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
    m_commandList->RSSetViewports(1, &m_viewport);
    m_commandList->RSSetScissorRects(1, &m_scissorRect);

    // Indicate that the back buffer will be used as a render target.
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    // Record commands.
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    m_commandList->DrawInstanced(3, 1, 0, 0);

    // Indicate that the back buffer will now be used to present.
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    ThrowIfFailed(m_commandList->Close());
}

void DX12SampleBox::WaitForPreviousFrame()
{

	// ����ϱ� ���� �������� ��ٷȴٰ� ����ϴ� ���� ���� �ٶ������� �ʽ��ϴ�.
	// �̰��� �ܼ�ȭ�� ���� ���� �� �ڵ��Դϴ�. D3D12HelloFrameBuffering
	// ȿ������ �ڿ� ����� ���� �潺�� ����ϴ� ����� �����ִ� �����Դϴ�.
	// GPU Ȱ�뵵�� �ش�ȭ�մϴ�.

	// ��Ÿ�� ���� ��ȣ�ϰ� ������ŵ�ϴ�.
    const UINT64 fence = m_fenceValue;
    ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), fence));
    m_fenceValue++;

	// ���� �������� ���� ������ ��ٸ��ϴ�.
    if (m_fence->GetCompletedValue() < fence)
    {
        ThrowIfFailed(m_fence->SetEventOnCompletion(fence, m_fenceEvent));
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }

    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}
