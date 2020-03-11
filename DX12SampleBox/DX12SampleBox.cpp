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

    ComPtr<IDXGIFactory4> factory;
    ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

    {
		// 디바이스 생성전에 하드웨어 가져오기
        ComPtr<IDXGIAdapter1> hardwareAdapter;
        GetHardwareAdapter(factory.Get(), &hardwareAdapter);
		// 디바이스 생성
        ThrowIfFailed(D3D12CreateDevice(
            hardwareAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&m_device)
            ));
    }

	// DESC 약자가 들거간 객체는 DX객체를 생성할때 필요한 정보로서 비트플래그 형식으로
	// 각각의 객체를 생성할때 정보를 만들어서 넘기는 객체이다
	// 한마디로 객체 생성에 필요한 정보를 만들어서 넘긴다 그것이 DESC객체이다
    // 커맨드큐 디스크립션(생성정보) 설정 
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	// 커매드큐 생서
    ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

    // 스왑체인 디스크립션(생성정보) 설정
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = FrameCount;
    swapChainDesc.Width = m_width;
    swapChainDesc.Height = m_height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

	// 스왑체인 생성
    ComPtr<IDXGISwapChain1> swapChain;
    ThrowIfFailed(factory->CreateSwapChainForHwnd(
        m_commandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
        Win32Application::GetHwnd(),
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain
        ));

    // ALT + Enter로 전체 화면 막는다
    ThrowIfFailed(factory->MakeWindowAssociation(Win32Application::GetHwnd(), DXGI_MWA_NO_ALT_ENTER));

	// 스왑체인에 현제 백버퍼 인덱스 가져온다
    ThrowIfFailed(swapChain.As(&m_swapChain));
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	/*********************************************************************
	그리기 명령을 제출 하기전에 해당 그리기 명령이 참조할 리소스들을 렌더링 파이프라인에 묶여야한다
	그런데 리소스들이 파이프라인에 직접 묶이는것은 아니다
	실제로 파이프라인에 묶이는 것은 해당 리소스을 참조하는 서술자( Descriptor)이다

	서술자는 리소스의 포인터 위치나 크기같은 리소스의 정보를 가진 리소스와 GPU 사이의 간접층이다

	리소스 -> 서술자 -> GPU
	GPU -> 서술자 -> 리소스

	서술자( Descriptor)자들을 거치는 추가적인 간접층을 두는 이유는 자원이라는것이 범용적이라서이다
	같은 자원을 렌더링 파이프라인의 서로 다른 단계(stage)들에서 사용할수있다

	텍스쳐를 렌더타켓에서 사용하고 그이후 단계에서 세이더 자원으로 사용 하는것이다

	리소스 자체는 자신이 렌더타겟에서 쓰이는지 세이더에서 사용하는지에 대해서 모른다

	서술자( Descriptor)는 DirectD3D에서 자원의 사용 방법을 알려준다
	어떤 파이프라인 단계에 쓰여야 하는지

	이전 버전의 DirectD3D 리소스뷰 라고 불럿습니다

	서술자힙( Descriptor Heap)은 서술자들의 배열이다
	응용프로그램(CPU)이 사용하는 서술자들이 저장되는 곳이 바로 서술자힙니다
	*********************************************************************/
    // Create descriptor heaps.
    {
        // 랜더타켓의 서술자힙(리소스뷰) 생성
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = FrameCount;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

        m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    }

    // Create frame resources.
    {
		// 랜더타켓뷰 디스크립션힙의 첫번째 위치를 핸들로 받는다
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

        // FrameCount만큼 랜더타켓(리소스)을 생성 한다
        for (UINT n = 0; n < FrameCount; n++)
        {
            ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
            m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, m_rtvDescriptorSize);
        }
    }

	/*********************************************************************
	명령 목록(command list)
	뷰포트를 설정하고 랜더타켓을 지우고 그리기 호출을 실행하라는 명령을 기록하는 객체

	메모리 할당자(CommandAllocator)
	명령 목록이 기록(저장)된 실제 메모리 객체

	명령 대기열(command queue)
	각각의 명령들을 큐형태의 자료로 들고 있다가 GPU에 명령을 내리는 객체?

	명령 목록이 기록을 다했으면 ID3D12GraphicsCommandList::Close 호출하여 명령기록이 끝났음을 알린다

	ID3D12CommandQueue::ExecuteCommandLists로 명령 목록을 제출 한다 제출할때는
	메모리 할당자(CommandAllocator)에서 명령들을 참조한다

	ID3D12CommandQueue::ExecuteCommandLists호출후
	ID3D12GraphicsCommandList::Reset한다면 ID3D12GraphicsCommandList를 새로 만들지 않고
	명령 목록들을 다시 사용 할수있다 Reset가 처음 생성했을때와 같은 상태로 만든다

	하나의 프레인을 완성하는 데 필요한 렌더링 명령들을 모두 GPU에 제출 한 후에는
	CommandAllocator::Reset하여 재사용 한다
	std::vector::clear 호출과 비슷하다

	GPU가 명령을 할당자에 담긴 모든 명령을 실행했음이 확실해지기 전까지 명령 할당자를 Reset하면 안된다
	그래서 명령을 기록 하기 직전에
	CommandAllocator::Reset , ID3D12GraphicsCommandList::Reset하는거 같다
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
		root signature(루트 서명)은 그리기 호출 전에 응용프로그램이 반드시 렌더링 파이프라인에 묶어야 하는 자원들이 뭔지
		그 자원들이 셰이더 입력 레지스터들에게 어떻게 대응 되는지를 정의한다.
		init을 할때 D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT 이라는 플래그를 주었는데
		이 플래그는 "앱이 입력 어셈블러 (버텍스 버퍼 바인딩 세트를 정의하는 입력 레이아웃이 필요함)를 사용하도록 선택합니다. 
		이 플래그를 생략하면 하나의 루트 인수 공간이 일부 하드웨어에 저장 될 수 있습니다. 
		입력 어셈블러가 필요하지 않은 경우에는이 플래그를 생략하십시오.
		단, 최적화는 중요하지 않습니다." 라고 한다.... 관련된 부분은 MSDN을 한번 살펴보자 ( 링크 )

		아무튼 ID3DBlob 이라는 형으로 signature와 error 을 생성했는데 Interface D3D Blob(Binary Large Object, [블랍? 이라고 읽나?]) 라는 뜻이다
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
		렌더링 준비 과정에서 리소스들을 준비 설정 하고 그객체들을 실제로 사용하기 위해서 랜더링 파이프라인이 묶어야 하는데 
		렌더링 파이프 라인 객체로 관리한다 쉐이더 루트시그니쳐 등등
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
	// 장면을 렌더링하는 데 필요한 모든 명령을 명령 목록에 기록합니다.
    PopulateCommandList();

    // Execute the command list.
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // 백퍼버 프론트 버퍼 교체
    ThrowIfFailed(m_swapChain->Present(1, 0));

    WaitForPreviousFrame();
}

void DX12SampleBox::PopulateCommandList()
{
	// 명령리스트 할당 자들은 연관된
	// 명령 목록이 GPU에서 실행을 마쳤습니다. 앱에서 사용해야하는
	// GPU 실행 진행을 결정하는 울타리.
    ThrowIfFailed(m_commandAllocator->Reset());

	// 그러나 특정 명령에서 ExecuteCommandList()가 호출되면
	// 목록에서 언제든지 명령 목록을 재설정 할 수 있습니다.
	// 다시 녹음하십시오.
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

	// 계속하기 전에 프레임을 기다렸다가 계속하는 것이 가장 바람직하지 않습니다.
	// 이것은 단순화를 위해 구현 된 코드입니다. D3D12HelloFrameBuffering
	// 효율적인 자원 사용을 위해 펜스를 사용하는 방법을 보여주는 샘플입니다.
	// GPU 활용도를 극대화합니다.

	// 울타리 값을 신호하고 증가시킵니다.
    const UINT64 fence = m_fenceValue;
    ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), fence));
    m_fenceValue++;

	// 이전 프레임이 끝날 때까지 기다립니다.
    if (m_fence->GetCompletedValue() < fence)
    {
        ThrowIfFailed(m_fence->SetEventOnCompletion(fence, m_fenceEvent));
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }

    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}
