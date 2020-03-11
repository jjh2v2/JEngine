#include "EngineMain.h"

LRESULT CALLBACK EngineMain::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// Handle any messages the switch statement didn't.
	static EngineMain *pthis = nullptr; // this �� ����� ������ 
	switch (message)
	{
	case WM_CREATE:
		// ������ �� �Ѱܹ��� this �����͸� �����Ͽ� ���� Ŭ���� ��������� ����Ѵ� 
		pthis = (EngineMain*)(((LPCREATESTRUCT)lParam)->lpCreateParams);
		return 0;
	case WM_PAINT:
		//hDC = BeginPaint(hWnd, &ps);
		//GetClientRect(hWnd, &rect);
		//// ����� ������ �� ������ �����͸� ���� �����Ѵ�. paClass-> 
		//// �׳� m_szMsg �� �����ϸ� ������ ������ ����. 
		//DrawText(hDC, paClass->m_szMsg, -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		//EndPaint(hWnd, &ps);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	if (pthis)
	{
		pthis->WindowsCallback(hWnd, message, wParam, lParam);
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

void EngineMain::WindowsCallback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{

	case WM_KEYDOWN:
		Test();
		return;
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		//JInput::instance().OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return;

		// Mouse button being released (while the cursor is currently over our window)
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		//JInput::instance().OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return;

		// Cursor moves over the window (or outside, while we're currently capturing it)
	case WM_MOUSEMOVE:
		//JInput::instance().OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		//if (wParam & BUTTON_STATE::RIGHT)
		//{
		//	JInputPoint MouseRot = JInput::instance().GetMouseRot();
		//	mCamera->Rotate(MouseRot.y, MouseRot.x);
		//}
		return;
		// Mouse wheel is scrolled
	case WM_MOUSEWHEEL:
		//OnMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return;
	}
}

void EngineMain::Init(HINSTANCE _hInstance, HINSTANCE _hPrevInstance, LPSTR _lpCmdLine, int _nCmdShow)
{
	hInstance = _hInstance;
	hPrevInstance = _hPrevInstance;
	lpCmdLin = _lpCmdLine;
	nCmdShow = _nCmdShow;
	hWndPlatformWindows = S_OK;
}


void EngineMain::InitPlatformWindows(UINT windowWidth /*= 480*/, UINT windowHeight /*= 640*/)
{
	// Window information
	TCHAR szWindowClass[] = _T("JEngineTest");
	TCHAR szTitle[] = _T("JEngineTest");

	// Create window class
	WNDCLASSEX wincx;
	wincx.cbSize = sizeof(WNDCLASSEX);
	wincx.style = CS_HREDRAW | CS_VREDRAW;
	wincx.lpfnWndProc = EngineMain::WndProc;
	//wincx.lpfnWndProc = (WNDPROC)fd::make_async_msg_proc(fd::make_delegate(&EngineMain::WndProc, this));

	wincx.cbClsExtra = 0;
	wincx.cbWndExtra = 0;
	wincx.hInstance = hInstance;
	wincx.hIcon = NULL;
	wincx.hCursor = LoadCursor(NULL, IDC_ARROW);
	wincx.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wincx.lpszMenuName = NULL;
	wincx.lpszClassName = szWindowClass;
	wincx.hIconSm = NULL;

	// Register window
	if (!RegisterClassEx(&wincx))
	{
		MessageBox(NULL, _T("Unable to Register window!"), _T("MA-DX12 Sample"), NULL);
		return;
	}

	// Create the window
	hWndPlatformWindows = CreateWindow(
		szWindowClass,
		szTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		windowWidth, windowHeight,
		NULL,
		NULL,
		hInstance,
		this
	);
	if (!hWndPlatformWindows)
	{
		MessageBox(NULL, _T("Unable to create window!"), _T("MA-DX12 Sample"), NULL);
		return;
	}

	ShowWindow(hWndPlatformWindows, nCmdShow);

	width = windowWidth;
	height = windowHeight;
	aspectRatio = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
	hwnd = hWndPlatformWindows;

}


void EngineMain::InitModules()
{
	{
		mViewport.Width = static_cast<float>(width);
		mViewport.Height = static_cast<float>(height);
		mViewport.MaxDepth = 1.0f;

		mScissorRect.right = static_cast<LONG>(width);
		mScissorRect.bottom = static_cast<LONG>(height);
	}

	// Create Device
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
	
		// ����̽� �������� �ϵ���� ��������
		ComPtr<IDXGIAdapter1> adapter;
		for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != mpFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				continue;
			}

			ComPtr<ID3D12Device> _device;
			if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&_device))))
			{
				break;
			}
		}
		// ����̽� ����
		ThrowIfFailed(D3D12CreateDevice(
			adapter.Get(),
			D3D_FEATURE_LEVEL_12_1,
			IID_PPV_ARGS(&mpDevice)
		));
	}

	// CreateCommandQueue
	{
		// DESC ���ڰ� ��Ű� ��ü�� DX��ü�� �����Ҷ� �ʿ��� �����μ� ��Ʈ�÷��� ��������
		// ������ ��ü�� �����Ҷ� ������ ���� �ѱ�� ��ü�̴�
		// �Ѹ���� ��ü ������ �ʿ��� ������ ���� �ѱ�� �װ��� DESC��ü�̴�
		// Ŀ�ǵ�ť ��ũ����(��������) ���� 
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		ThrowIfFailed(mpDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mpCommandQueue)));
	}

	// CreateCommandAllocator
	{
		ThrowIfFailed(mpDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mpCommandAllocator)));
	}

	// CreateCommandList
	{
		ThrowIfFailed(mpDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mpCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&mpCommandList)));
	}
	
	// CreateSwapChain
	{
		// ����ü�� ��ũ����(��������) ����
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.BufferCount = iFrameCount;
		swapChainDesc.Width = width;
		swapChainDesc.Height = height;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count = 1;

		// ����ü�� ����
		ComPtr<IDXGISwapChain1> swapChain;
		ThrowIfFailed(mpFactory->CreateSwapChainForHwnd(
			mpCommandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
			hwnd,
			&swapChainDesc,
			nullptr,
			nullptr,
			&swapChain
		));

		// ALT + Enter�� ��ü ȭ�� ���´�
		ThrowIfFailed(mpFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));

		// ����ü�ο� ���� ����� �ε��� �����´�
		ThrowIfFailed(swapChain.As(&mpSwapChain));
		miCurrentFrameIndex = mpSwapChain->GetCurrentBackBufferIndex();
	}

	// CreateRenderTarget
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

	// Shader
	{
#if defined(_DEBUG)
		// Enable better shader debugging with the graphics debugging tools.
		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT compileFlags = 0;
#endif
		std::wstring mcsFileName = L"C:/Users/jjh/Desktop/Project/JEngine/DX12_Init/shaders_0.hlsl";
		HRESULT hr = S_FALSE;
		hr = D3DCompileFromFile(mcsFileName.c_str(), nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &mpVertexShader, nullptr);
		if (hr != S_OK)
		{
		}
		hr = D3DCompileFromFile(mcsFileName.c_str(), nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &mpPixelShader, nullptr);
		if (hr != S_OK)
		{
		}
	}

	// CreateRootSignature
	{
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;// D3D_ROOT_SIGNATURE_VERSION_1_0

		UINT iSRVRegister = 0;// Texture2D DiffuseTexture : register(t0);
		CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
		//ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, iSRVRegister, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
		//ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, iSRVRegister + 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
		//range[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0); 4�� t0 ~ t3 ���� 4���� �ִٴ°� �˸��°�?
		//range[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0); 1���� ���� ���� ���� �ϴ�
		//range[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
		//range[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 16, 0);
		//range[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);

		CD3DX12_ROOT_PARAMETER1 rootParameters[1];
		//rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
		//rootParameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);
		rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_ALL);
		//rootParameters[0].InitAsConstantBufferView(0);

		// SAMPLER
		const CD3DX12_STATIC_SAMPLER_DESC pointWrap[2] = {
			{0, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_WRAP },

			{1, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP} // addressW
		}; // addressW


		// Allow input layout and deny uneccessary access to certain pipeline stages.
		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		UINT sdfdf = 0;// sizeof(pointWrap) / sizeof(CD3DX12_STATIC_SAMPLER_DESC);
		UINT iRootParameters = _countof(rootParameters);
		rootSignatureDesc.Init_1_1(iRootParameters, rootParameters, sdfdf, pointWrap, rootSignatureFlags);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
		ThrowIfFailed(mpDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&mpRootSignature)));
	}

	// CreatePipelineState
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
		psoDesc.pRootSignature = mpRootSignature.Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(mpVertexShader.Get());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(mpPixelShader.Get());
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState.DepthEnable = FALSE;
		psoDesc.DepthStencilState.StencilEnable = FALSE;
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.SampleDesc.Count = 1;

		ThrowIfFailed(mpDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mpPipelineState)));
	}

	// ���ؽ� ���� ����
	{
		mObj.push_back(new Mesh("square", mpDevice.Get(), mpCommandList.Get(), XMFLOAT3(0.5f, 0.0f, 0.0f)));
		mObj.push_back(new Mesh("square", mpDevice.Get(), mpCommandList.Get(), XMFLOAT3(-0.5f, 0.0f, 0.0f)));
	}

	// DescriptorHeap �����ÿ� NumDescriptors�� ũ�� ��Ƽ� ����
	UINT NumDescriptors = 8192;//��ũ��Ʈ���� ������ �ִ� �� ��ũ��Ʈ���� { �������(CBV) + �̹���(SRV) + ���ĵ��������׼�����(UAV) }
	// CreateDescriptorHeap
	{
		// Describe and create a constant buffer view (CBV) descriptor heap.
		// Flags indicate that this descriptor heap can be bound to the pipeline 
		// and that descriptors contained in it can be referenced by a root table.
		D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
		cbvHeapDesc.NumDescriptors = NumDescriptors;// �ش� ��ũ�������� ����� ��ü ������� ���� �� �ټ����ƴϿ��� �Ǵµ�??
		cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		ThrowIfFailed(mpDevice->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&mpCBVHeap)));
	}
	
	// CreateConstantBufferView
	{
		UINT mElementByteSize = sizeof(SceneConstantBuffer);
		// (sizeof(SceneConstantBuffer) + 255) & ~255;    // CB size is required to be 256-byte aligned.
		mElementByteSize = (sizeof(SceneConstantBuffer) + 255) & ~255;

		ThrowIfFailed(mpDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(mElementByteSize * NumDescriptors/*mObj.size()*/),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&mpConstantBuffer)));


		D3D12_CONSTANT_BUFFER_VIEW_DESC	descBuffer;
		descBuffer.BufferLocation = mpConstantBuffer->GetGPUVirtualAddress();
		descBuffer.SizeInBytes = mElementByteSize;
		UINT HandleIncrementSize = mpDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		for (int i = 0; i < mObj.size(); ++i)
		{
			descBuffer.BufferLocation = mpConstantBuffer->GetGPUVirtualAddress() + i * mElementByteSize;
			D3D12_CPU_DESCRIPTOR_HANDLE handle = mpCBVHeap->GetCPUDescriptorHandleForHeapStart();
			handle.ptr += static_cast<size_t>(i * HandleIncrementSize);
			mpDevice->CreateConstantBufferView(&descBuffer, handle);
		}

		ThrowIfFailed(mpConstantBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mPCBVDataBegin)));
	}

	// CreateFence
	{
		ThrowIfFailed(mpDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence)));
		mFenceValue = 1;
		// Create an event handle to use for frame synchronization.
		mFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (mFenceEvent == nullptr)
		{
			ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
		}
	}


	// Wait for the command list to execute; we are reusing the same command 
	// list in our main loop but for now, we just want to wait for setup to 
	// complete before continuing.
	WaitForPreviousFrame();

	mCamera = new Camera((float)width, (float)height);

	//��ɱ���� �������� �˸���
	ThrowIfFailed(mpCommandList->Close());
}

void EngineMain::Test()
{
	mObj.push_back(new Mesh("square", mpDevice.Get(), mpCommandList.Get(), XMFLOAT3(-1.0f, 0.0f, 0.0f)));
}

void EngineMain::WaitForPreviousFrame()
{

	// ����ϱ� ���� �������� ��ٷȴٰ� ����ϴ� ���� ���� �ٶ������� �ʽ��ϴ�.
	// �̰��� �ܼ�ȭ�� ���� ���� �� �ڵ��Դϴ�. D3D12HelloFrameBuffering
	// ȿ������ �ڿ� ����� ���� �潺�� ����ϴ� ����� �����ִ� �����Դϴ�.
	// GPU Ȱ�뵵�� �ش�ȭ�մϴ�.

	// ��Ÿ�� ���� ��ȣ�ϰ� ������ŵ�ϴ�.
	const UINT64 fence = mFenceValue;
	ThrowIfFailed(mpCommandQueue->Signal(mFence.Get(), fence));
	mFenceValue++;

	// ���� �������� ���� ������ ��ٸ��ϴ�.
	if (mFence->GetCompletedValue() < fence)
	{
		ThrowIfFailed(mFence->SetEventOnCompletion(fence, mFenceEvent));
		WaitForSingleObject(mFenceEvent, INFINITE);
	}

	miCurrentFrameIndex = mpSwapChain->GetCurrentBackBufferIndex();
}

void EngineMain::OnUpdate()
{
	//mCamera->Update(deltaTime);

	XMMATRIX viewMat = XMLoadFloat4x4(&mCamera->GetViewMatrix()); // load view matrix
	XMMATRIX projMat = XMLoadFloat4x4(&mCamera->GetProjectionMatrix()); // load projection matrix
	for (int iCount = 0; iCount < mObj.size(); iCount++)
	{
		SceneConstantBuffer mConstantBufferData;

		XMMATRIX wvpMat = XMLoadFloat4x4(&mObj[iCount]->mWorld) * viewMat * projMat; // create wvp matrix
		XMMATRIX transposed = XMMatrixTranspose(wvpMat); // must transpose wvp matrix for the gpu
		XMFLOAT4X4 wvp;
		XMStoreFloat4x4(&wvp, transposed); // store transposed wvp matrix in constant buffer
		mConstantBufferData.worldViewProjection = wvp;
		XMFLOAT3 ds = mObj[iCount]->GetPosition();
		ds.x += 0.001f/* * deltaTime*/;
		mObj[iCount]->SetPosition(ds);
		mConstantBufferData.world = mObj[iCount]->GetWorld();
		size_t d = (sizeof(SceneConstantBuffer) + 255) & ~255;;
		memcpy(&mPCBVDataBegin[iCount * d], &mConstantBufferData, sizeof(mConstantBufferData));
	}
}
void EngineMain::OnRender()
{

	// ����� �������ϴ� �� �ʿ��� ��� ����� ��� ��Ͽ� ����մϴ�.
	//PopulateCommandList();
	// ��ɸ���Ʈ �Ҵ� �ڵ��� ������
	// ��� ����� GPU���� ������ ���ƽ��ϴ�. �ۿ��� ����ؾ��ϴ�
	// GPU ���� ������ �����ϴ� ��Ÿ��.
	ThrowIfFailed(mpCommandAllocator->Reset());

	// �׷��� Ư�� ��ɿ��� ExecuteCommandList()�� ȣ��Ǹ�
	// ��Ͽ��� �������� ��� ����� �缳�� �� �� �ֽ��ϴ�.
	// �ٽ� �����Ͻʽÿ�.
	ThrowIfFailed(mpCommandList->Reset(mpCommandAllocator.Get(), mpPipelineState.Get()));

	// Set necessary state.
	mpCommandList->SetGraphicsRootSignature(mpRootSignature.Get());

	ID3D12DescriptorHeap* ppHeaps[] = { mpCBVHeap.Get() };
	mpCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	mpCommandList->RSSetViewports(1, &mViewport);
	mpCommandList->RSSetScissorRects(1, &mScissorRect);

	// Indicate that the back buffer will be used as a render target.
	mpCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mvRenderTargets[miCurrentFrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mpRtvHeap->GetCPUDescriptorHandleForHeapStart(), miCurrentFrameIndex, miRtvDescriptorSize);
	mpCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	// Record commands.
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	mpCommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	for (int i = 0; i < mObj.size(); i++)
	{
		CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mpCBVHeap->GetGPUDescriptorHandleForHeapStart());
		UINT mCbvSrvDescriptorSize = mpDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		tex.Offset(i * mCbvSrvDescriptorSize);// tex.ptr += i * mCbvSrvDescriptorSize;
		mpCommandList->SetGraphicsRootDescriptorTable(0, tex);
		// t1
		// t2
		//mpCommandList->SetGraphicsRootDescriptorTable(1, tex);
		//

		mpCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		mpCommandList->IASetVertexBuffers(0, 1, &mObj[i]->GetVertexBufferView());
		mpCommandList->IASetIndexBuffer(&mObj[i]->GetIndexBufferView());
		mpCommandList->DrawIndexedInstanced(mObj[i]->GetIndexCount(), 1, 0, 0, 0);
	}

	// Indicate that the back buffer will now be used to present.
	mpCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mvRenderTargets[miCurrentFrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	//��ɱ���� �������� �˸���
	ThrowIfFailed(mpCommandList->Close());

	// Execute the command list.
	ID3D12CommandList* ppCommandLists[] = { mpCommandList.Get() };
	mpCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// ���۹� ����Ʈ ���� ��ü
	ThrowIfFailed(mpSwapChain->Present(1, 0));

	WaitForPreviousFrame();
}