#include "EngineMain.h"

LRESULT CALLBACK EngineMain::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// Handle any messages the switch statement didn't.
	static EngineMain *pthis = nullptr; // this 를 대신할 포인터 
	switch (message)
	{
	case WM_CREATE:
		// 생성될 때 넘겨받은 this 포인터를 보관하여 추후 클래스 멤버참조시 사용한다 
		pthis = (EngineMain*)(((LPCREATESTRUCT)lParam)->lpCreateParams);
		return 0;
	case WM_PAINT:
		//hDC = BeginPaint(hWnd, &ps);
		//GetClientRect(hWnd, &rect);
		//// 멤버에 접근할 때 보관한 포인터를 통해 접근한다. paClass-> 
		//// 그냥 m_szMsg 로 접근하면 컴파일 에러가 난다. 
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
		AddBuffe();
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
	
		// 디바이스 생성전에 하드웨어 가져오기
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
		// 디바이스 생성
		ThrowIfFailed(D3D12CreateDevice(
			adapter.Get(),
			D3D_FEATURE_LEVEL_12_1,
			IID_PPV_ARGS(&mpDevice)
		));
	}

	// CreateCommandQueue
	{
		// DESC 약자가 들거간 객체는 DX객체를 생성할때 필요한 정보로서 비트플래그 형식으로
		// 각각의 객체를 생성할때 정보를 만들어서 넘기는 객체이다
		// 한마디로 객체 생성에 필요한 정보를 만들어서 넘긴다 그것이 DESC객체이다
		// 커맨드큐 디스크립션(생성정보) 설정 
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		ThrowIfFailed(mpDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mpCommandQueue)));
	}

	// CreateCommandAllocator
	{
		//ThrowIfFailed(mpDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mpCommandAllocator)));
	}

	// CreateCommandList
	{
		//ThrowIfFailed(mpDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mpCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&mpCommandList)));
		//명령기록이 끝났음을 알린다
		//ThrowIfFailed(mpCommandList->Close());
	}
	
	{// FrameResource Multithread
		for (int iCount=0; iCount<iFrameCount; iCount++)
		{
			//mFrameResource[iCount].Init(mpDevice.Get());

			{// FenceValue
				mFenceValue = 0;
			}

			{// Pre
				ThrowIfFailed(mpDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mFrameResource[iCount].mPreCMDAllocator)));
				ThrowIfFailed(mpDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mFrameResource[iCount].mPreCMDAllocator.Get(), nullptr, IID_PPV_ARGS(&mFrameResource[iCount].mPreCMDList)));
				ThrowIfFailed(mFrameResource[iCount].mPreCMDList->Close());
			}

			{// Post
				ThrowIfFailed(mpDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mFrameResource[iCount].mPostCMDAllocator)));
				ThrowIfFailed(mpDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mFrameResource[iCount].mPostCMDAllocator.Get(), nullptr, IID_PPV_ARGS(&mFrameResource[iCount].mPostCMDList)));
				ThrowIfFailed(mFrameResource[iCount].mPostCMDList->Close());
			}

			{// Multithread
				// 프레임리소스에는 우선 디스크립트힙은 안넣고 작업 
				// 스레스 개수 알아오기
				SYSTEM_INFO stSysInfo;
				GetSystemInfo(&stSysInfo);
				int dwNumberOfProcessors = 2;// static_cast<int>(stSysInfo.dwNumberOfProcessors);
				for (int i = 0; i < dwNumberOfProcessors; i++)
				{
					ID3D12CommandAllocator* pCMDAllocator = nullptr;
					ID3D12GraphicsCommandList* pCMDList = nullptr;
					ThrowIfFailed(mpDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&pCMDAllocator)));
					ThrowIfFailed(mpDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, pCMDAllocator, nullptr, IID_PPV_ARGS(&pCMDList)));

					//mFrameResource[iCount].mVecMultithreadCMDAllocator[i] = pCMDAllocator;
					//mFrameResource[iCount].mVecMultithreadCMDList[i] = pCMDList;

					mFrameResource[iCount].mVecMultithreadCMDAllocator.push_back(pCMDAllocator);
					mFrameResource[iCount].mVecMultithreadCMDList.push_back(pCMDList);
					pCMDList->Close();
				}

			}
		}
	}

	// CreateSwapChain
	{
		// 스왑체인 디스크립션(생성정보) 설정
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.BufferCount = iSwapChainBufferCount;
		swapChainDesc.Width = width;
		swapChainDesc.Height = height;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count = 1;

		// 스왑체인 생성
		ComPtr<IDXGISwapChain1> swapChain;
		ThrowIfFailed(mpFactory->CreateSwapChainForHwnd(
			mpCommandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
			hwnd,
			&swapChainDesc,
			nullptr,
			nullptr,
			&swapChain
		));

		// ALT + Enter로 전체 화면 막는다
		ThrowIfFailed(mpFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));

		// 스왑체인에 현제 백버퍼 인덱스 가져온다
		ThrowIfFailed(swapChain.As(&mpSwapChain));
		miCurrentFrameIndex = mpSwapChain->GetCurrentBackBufferIndex();
	}

	// CreateRenderTarget
	{
		// 랜더타켓의 서술자힙(리소스뷰) 생성
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = iSwapChainBufferCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(mpDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&mpRtvHeap)));

		miRtvDescriptorSize = mpDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		// 랜더타켓뷰 디스크립션힙의 첫번째 위치를 핸들로 받는다
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mpRtvHeap->GetCPUDescriptorHandleForHeapStart());

		// FrameCount만큼 랜더타켓(리소스)을 생성 한다
		for (UINT n = 0; n < iSwapChainBufferCount; n++)
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
		std::wstring mcsFileName = L"C:/Users/jjh/Desktop/Project/JEngine/DX12_MultiThread/shaders_0.hlsl";
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
		CD3DX12_DESCRIPTOR_RANGE1 ranges[3];
		//ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, iSRVRegister, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
		//ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, iSRVRegister + 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
		ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, iSRVRegister /*+ 1*/, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
		ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE); // s0
		//range[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0); 4은 t0 ~ t3 까지 4개가 있다는걸 알리는거?
		//range[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0); 1개씩 따로 설정 가능 하다
		//range[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
		//range[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 16, 0);
		//range[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);

		CD3DX12_ROOT_PARAMETER1 rootParameters[3];
		//rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
		//rootParameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);
		rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_ALL);
		rootParameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);
		rootParameters[2].InitAsDescriptorTable(1, &ranges[2], D3D12_SHADER_VISIBILITY_PIXEL); // s0
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
		UINT sdfdf = sizeof(pointWrap) / sizeof(CD3DX12_STATIC_SAMPLER_DESC);
		UINT iRootParameters = _countof(rootParameters);
		//rootSignatureDesc.Init_1_1(iRootParameters, rootParameters, sdfdf, pointWrap, rootSignatureFlags);
		// 디스크립트힙에서 디스크립트테이블로 설정할거면 여기서 샘플러 넣으면 안된다
		rootSignatureDesc.Init_1_1(iRootParameters, rootParameters, 0, nullptr, rootSignatureFlags);

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

	// 버텍스 버퍼 생성
	{
		mObj.push_back(new Mesh("square", mpDevice.Get(), nullptr, XMFLOAT3(0.5f, 0.0f, 0.0f)));
		mObj.push_back(new Mesh("square", mpDevice.Get(), nullptr, XMFLOAT3(-0.1f, 0.0f, 0.0f)));
		mObj.push_back(new Mesh("square", mpDevice.Get(), nullptr, XMFLOAT3(-0.7f, 0.0f, 0.0f)));
		mObj.push_back(new Mesh("square", mpDevice.Get(), nullptr, XMFLOAT3(-1.3f, 0.0f, 0.0f)));
	}

	// DescriptorHeap 생성시에 NumDescriptors를 크게 잡아서 하자
	UINT NumDescriptors = 8192;//디스크립트힙이 가질수 있는 총 디스크립트갯수 { 상수버퍼(CBV) + 이미지(SRV) + 정렬되지않은액세스뷰(UAV) }
	// CreateDescriptorHeap
	{
		// Describe and create a constant buffer view (CBV) descriptor heap.
		// Flags indicate that this descriptor heap can be bound to the pipeline 
		// and that descriptors contained in it can be referenced by a root table.
		D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
		cbvHeapDesc.NumDescriptors = NumDescriptors;// 해당 디스크립힙에서 사용할 전체 상수버퍼 개수 꼭 다수가아니여도 되는듯??
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

	// CreateTexture
	{
		// Texture Load
		auto woodCrateTex = std::make_unique<Texture>();
		woodCrateTex->Name = "bricks";
		woodCrateTex->Filename = L"Textures/bricks.dds";
		mTextures["bricks"] = std::move(woodCrateTex);

		ResourceUploadBatch resourceUpload(mpDevice.Get());
		resourceUpload.Begin();
		ThrowIfFailed(
			CreateDDSTextureFromFile(mpDevice.Get(), resourceUpload, L"Textures/bricks.dds", mTextures["bricks"]->Resource.ReleaseAndGetAddressOf())
		);

		// Upload the resources to the GPU.
		auto uploadResourcesFinished = resourceUpload.End(mpCommandQueue.Get());
		// Wait for the upload thread to terminate
		uploadResourcesFinished.wait();

		// 텍스쳐 재 생성
		D3D12_CPU_DESCRIPTOR_HANDLE hDescriptor(mpCBVHeap->GetCPUDescriptorHandleForHeapStart());
		UINT HandleIncrementSize = mpDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		hDescriptor.ptr += static_cast<size_t>(mObj.size() * HandleIncrementSize);
		//hDescriptor.Offset()
		auto woodCrateTexss = mTextures["bricks"]->Resource;
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = woodCrateTexss->GetDesc().Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = woodCrateTexss->GetDesc().MipLevels;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		mpDevice->CreateShaderResourceView(woodCrateTexss.Get(), &srvDesc, hDescriptor);
	}

	{// CreateSampler
		// 샘플러 전용 디스크립트 힙을 따로 만들어서 SetDescriptorHeaps할때 CBV_SRV_UAV힙 SAMPLER힙 두개 넘기면 된다
		// SetGraphicsRootDescriptorTable()할때는 디스크립트테이블 설정한 인덱스 사용하면 된다
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		desc.Flags = true ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		desc.NumDescriptors = 2;
		ThrowIfFailed(mpDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&mSampleHeap)));

		D3D12_SAMPLER_DESC sampler = {};
		sampler.Filter = D3D12_FILTER_ANISOTROPIC;
		sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler.MaxAnisotropy = 2;
		sampler.MinLOD = -1.0f;
		sampler.MaxLOD = 1.0f;
		D3D12_CPU_DESCRIPTOR_HANDLE mSampler(mSampleHeap->GetCPUDescriptorHandleForHeapStart());
		UINT HandleIncrementSize = mpDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
		mSampler.ptr += static_cast<size_t>(0 * HandleIncrementSize);
		mpDevice->CreateSampler(&sampler, mSampler);

		mSampler.ptr += static_cast<size_t>(1 * HandleIncrementSize);
		mpDevice->CreateSampler(&sampler, mSampler);
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
}

void EngineMain::AddBuffe()
{
	UINT mElementByteSize = sizeof(SceneConstantBuffer);
	// (sizeof(SceneConstantBuffer) + 255) & ~255;    // CB size is required to be 256-byte aligned.
	mElementByteSize = (sizeof(SceneConstantBuffer) + 255) & ~255;

	D3D12_CONSTANT_BUFFER_VIEW_DESC	descBuffer;
	descBuffer.BufferLocation = mpConstantBuffer->GetGPUVirtualAddress();
	descBuffer.SizeInBytes = mElementByteSize;
	UINT HandleIncrementSize = mpDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	//for (int i = 0; i < mObj.size(); ++i)
	{
		size_t iAddress = mObj.size();
		// BufferLocation는 상수버퍼(지역) 안에서만의 위치같다
		descBuffer.BufferLocation = mpConstantBuffer->GetGPUVirtualAddress() + iAddress * mElementByteSize;
		D3D12_CPU_DESCRIPTOR_HANDLE handle = mpCBVHeap->GetCPUDescriptorHandleForHeapStart();
		// 상수버퍼 2개를 초기에 먼저 만들고나서 텍스쳐를 만들었다
		// 키다운을 하면 상수버퍼를 하나 추가 하는데 인덱스 2에 이미 텍스쳐가 있어서 (iAddress+1)해야한다
		handle.ptr += static_cast<size_t>((iAddress+1) * HandleIncrementSize);
		mpDevice->CreateConstantBufferView(&descBuffer, handle);
	}

	ThrowIfFailed(mpConstantBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mPCBVDataBegin)));

	mObj.push_back(new Mesh("square", mpDevice.Get(), nullptr, XMFLOAT3(-1.3f, 0.0f, 0.0f)));
}

void EngineMain::WaitForPreviousFrame()
{

	// 계속하기 전에 프레임을 기다렸다가 계속하는 것이 가장 바람직하지 않습니다.
	// 이것은 단순화를 위해 구현 된 코드입니다. D3D12HelloFrameBuffering
	// 효율적인 자원 사용을 위해 펜스를 사용하는 방법을 보여주는 샘플입니다.
	// GPU 활용도를 극대화합니다.

	// 울타리 값을 신호하고 증가시킵니다.
	const UINT64 fence = mFenceValue;
	ThrowIfFailed(mpCommandQueue->Signal(mFence.Get(), fence));
	mFenceValue++;

	// 이전 프레임이 끝날 때까지 기다립니다.
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

#define MULTI_THREAD

#ifdef MULTI_THREAD

void EngineMain::OnRender()
{
	std::thread thread1([](int threadIndex, EngineMain* _this)
	{
		ID3D12CommandAllocator* pCommandAllocator = _this->mFrameResource[0].mVecMultithreadCMDAllocator[threadIndex];
		ID3D12GraphicsCommandList* pCommandList = _this->mFrameResource[0].mVecMultithreadCMDList[threadIndex];
		ThrowIfFailed(pCommandAllocator->Reset());
		ThrowIfFailed(pCommandList->Reset(pCommandAllocator, _this->mpPipelineState.Get()));

		pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pCommandList->RSSetViewports(1, &_this->mViewport);
		pCommandList->RSSetScissorRects(1, &_this->mScissorRect);
		
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(_this->mpRtvHeap->GetCPUDescriptorHandleForHeapStart(), _this->miCurrentFrameIndex, _this->miRtvDescriptorSize);
		pCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

		{// 리소스 설정, 파이프라인상태 설정
		/*
		티스크립트갯수를 많이 설정한 디스크립트힙 하나를 만들고
		상수버퍼와 이미지를 생성시 디스크립트힙 핸들 주소를 계산해서 오브젝트마다 핸들 주소를 가지고 있도록 한다?
		디스크립트힙 핸들 주소 계산을 쉽게 하기 위해서는 텍스쳐를 먼저 생성하고 상수 버퍼를 나중에 하는방향으로?
		텍스쳐는 고정몃개 맥스치 정해주면 될거같은데 상수버퍼는 기하급수적으로 늘어날수 있어서?
		*/
			pCommandList->SetGraphicsRootSignature(_this->mpRootSignature.Get());
			ID3D12DescriptorHeap* ppHeaps[] = { _this->mpCBVHeap.Get(), _this->mSampleHeap.Get() };
			pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
			for (int i = threadIndex; i < _this->mObj.size(); i += 2)
			//for (int i = 0; i < _this->mObj.size(); i++)
			{
				UINT mCbvSrvDescriptorSize = _this->mpDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				if (i < 4)
				{
					D3D12_GPU_DESCRIPTOR_HANDLE CBV(_this->mpCBVHeap->GetGPUDescriptorHandleForHeapStart());
					CBV.ptr += i * mCbvSrvDescriptorSize;// CBV.Offset(i * mCbvSrvDescriptorSize);
					pCommandList->SetGraphicsRootDescriptorTable(0, CBV);
				}
				else if (i >= 4)
				{
					// 상수버퍼 2개를 초기에 먼저 만들고나서 텍스쳐를 만들었다
					// 키다운을 하면 상수버퍼를 하나 추가 하는데 인덱스 2에 이미 텍스쳐가 있어서 (i+1)해야한다
					D3D12_GPU_DESCRIPTOR_HANDLE CBV(_this->mpCBVHeap->GetGPUDescriptorHandleForHeapStart());
					CBV.ptr += (i + 1) * mCbvSrvDescriptorSize;// CBV.Offset(i * mCbvSrvDescriptorSize);
					pCommandList->SetGraphicsRootDescriptorTable(0, CBV);
				}
				//D3D12_GPU_DESCRIPTOR_HANDLE CBV(mpCBVHeap->GetGPUDescriptorHandleForHeapStart());
				//UINT mCbvSrvDescriptorSize = mpDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				//CBV.ptr += i * mCbvSrvDescriptorSize;// CBV.Offset(i * mCbvSrvDescriptorSize);
				//pCommandList->SetGraphicsRootDescriptorTable(0, CBV);
				// t1 디스크립트힙에 생성 할때부터 문제인가?????????? 따른 애들이 만든 프로젝트 보고 참고하자
				D3D12_GPU_DESCRIPTOR_HANDLE SRV(_this->mpCBVHeap->GetGPUDescriptorHandleForHeapStart());
				SRV.ptr += 4 * mCbvSrvDescriptorSize;// CBV.Offset(i * mCbvSrvDescriptorSize);
				pCommandList->SetGraphicsRootDescriptorTable(1, SRV);
				// t2
				//pCommandList->SetGraphicsRootDescriptorTable(1, tex);
				// s1
				D3D12_GPU_DESCRIPTOR_HANDLE mSampler(_this->mSampleHeap->GetGPUDescriptorHandleForHeapStart());
				UINT mSmapleDescriptorSize = _this->mpDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
				mSampler.ptr += 1 * mSmapleDescriptorSize;
				pCommandList->SetGraphicsRootDescriptorTable(2, mSampler);

				pCommandList->IASetVertexBuffers(0, 1, &_this->mObj[i]->GetVertexBufferView());
				pCommandList->IASetIndexBuffer(&_this->mObj[i]->GetIndexBufferView());
				pCommandList->DrawIndexedInstanced(_this->mObj[i]->GetIndexCount(), 1, 0, 0, 0);
			}
		}
		//명령기록이 끝났음을 알린다
		ThrowIfFailed(pCommandList->Close());
	}, 0, this);

	std::thread thread2([](int threadIndex, EngineMain* _this)
	{
		ID3D12CommandAllocator* pCommandAllocator = _this->mFrameResource[0].mVecMultithreadCMDAllocator[threadIndex];
		ID3D12GraphicsCommandList* pCommandList = _this->mFrameResource[0].mVecMultithreadCMDList[threadIndex];
		ThrowIfFailed(pCommandAllocator->Reset());
		ThrowIfFailed(pCommandList->Reset(pCommandAllocator, _this->mpPipelineState.Get()));

		pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pCommandList->RSSetViewports(1, &_this->mViewport);
		pCommandList->RSSetScissorRects(1, &_this->mScissorRect);

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(_this->mpRtvHeap->GetCPUDescriptorHandleForHeapStart(), _this->miCurrentFrameIndex, _this->miRtvDescriptorSize);
		pCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

		{// 리소스 설정, 파이프라인상태 설정
		/*
		티스크립트갯수를 많이 설정한 디스크립트힙 하나를 만들고
		상수버퍼와 이미지를 생성시 디스크립트힙 핸들 주소를 계산해서 오브젝트마다 핸들 주소를 가지고 있도록 한다?
		디스크립트힙 핸들 주소 계산을 쉽게 하기 위해서는 텍스쳐를 먼저 생성하고 상수 버퍼를 나중에 하는방향으로?
		텍스쳐는 고정몃개 맥스치 정해주면 될거같은데 상수버퍼는 기하급수적으로 늘어날수 있어서?
		*/
			pCommandList->SetGraphicsRootSignature(_this->mpRootSignature.Get());
			ID3D12DescriptorHeap* ppHeaps[] = { _this->mpCBVHeap.Get(), _this->mSampleHeap.Get() };
			pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
			for (int i = threadIndex; i < _this->mObj.size(); i += 2)
				//for (int i = 0; i < _this->mObj.size(); i++)
			{
				UINT mCbvSrvDescriptorSize = _this->mpDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				if (i < 4)
				{
					D3D12_GPU_DESCRIPTOR_HANDLE CBV(_this->mpCBVHeap->GetGPUDescriptorHandleForHeapStart());
					CBV.ptr += i * mCbvSrvDescriptorSize;// CBV.Offset(i * mCbvSrvDescriptorSize);
					pCommandList->SetGraphicsRootDescriptorTable(0, CBV);
				}
				else if (i >= 4)
				{
					// 상수버퍼 2개를 초기에 먼저 만들고나서 텍스쳐를 만들었다
					// 키다운을 하면 상수버퍼를 하나 추가 하는데 인덱스 2에 이미 텍스쳐가 있어서 (i+1)해야한다
					D3D12_GPU_DESCRIPTOR_HANDLE CBV(_this->mpCBVHeap->GetGPUDescriptorHandleForHeapStart());
					CBV.ptr += (i + 1) * mCbvSrvDescriptorSize;// CBV.Offset(i * mCbvSrvDescriptorSize);
					pCommandList->SetGraphicsRootDescriptorTable(0, CBV);
				}
				//D3D12_GPU_DESCRIPTOR_HANDLE CBV(mpCBVHeap->GetGPUDescriptorHandleForHeapStart());
				//UINT mCbvSrvDescriptorSize = mpDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				//CBV.ptr += i * mCbvSrvDescriptorSize;// CBV.Offset(i * mCbvSrvDescriptorSize);
				//pCommandList->SetGraphicsRootDescriptorTable(0, CBV);
				// t1 디스크립트힙에 생성 할때부터 문제인가?????????? 따른 애들이 만든 프로젝트 보고 참고하자
				D3D12_GPU_DESCRIPTOR_HANDLE SRV(_this->mpCBVHeap->GetGPUDescriptorHandleForHeapStart());
				SRV.ptr += 4 * mCbvSrvDescriptorSize;// CBV.Offset(i * mCbvSrvDescriptorSize);
				pCommandList->SetGraphicsRootDescriptorTable(1, SRV);
				// t2
				//pCommandList->SetGraphicsRootDescriptorTable(1, tex);
				// s1
				D3D12_GPU_DESCRIPTOR_HANDLE mSampler(_this->mSampleHeap->GetGPUDescriptorHandleForHeapStart());
				UINT mSmapleDescriptorSize = _this->mpDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
				mSampler.ptr += 1 * mSmapleDescriptorSize;
				pCommandList->SetGraphicsRootDescriptorTable(2, mSampler);

				pCommandList->IASetVertexBuffers(0, 1, &_this->mObj[i]->GetVertexBufferView());
				pCommandList->IASetIndexBuffer(&_this->mObj[i]->GetIndexBufferView());
				pCommandList->DrawIndexedInstanced(_this->mObj[i]->GetIndexCount(), 1, 0, 0, 0);
			}
		}
		//명령기록이 끝났음을 알린다
		ThrowIfFailed(pCommandList->Close());
	}, 1, this);

	thread1.join();
	thread2.join();

	ID3D12CommandAllocator* pPreCommandAllocator = mFrameResource[0].mPreCMDAllocator.Get();
	ID3D12GraphicsCommandList* pPreCommandList = mFrameResource[0].mPreCMDList.Get();
	ID3D12CommandAllocator* pPostCommandAllocator = mFrameResource[0].mPostCMDAllocator.Get();
	ID3D12GraphicsCommandList* pPostCommandList = mFrameResource[0].mPostCMDList.Get();

	{// Pre
		ThrowIfFailed(pPreCommandAllocator->Reset());
		ThrowIfFailed(pPreCommandList->Reset(pPreCommandAllocator, mpPipelineState.Get()));

		pPreCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mvRenderTargets[miCurrentFrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mpRtvHeap->GetCPUDescriptorHandleForHeapStart(), miCurrentFrameIndex, miRtvDescriptorSize);
		pPreCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
		// Record commands.
		const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
		pPreCommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		ThrowIfFailed(pPreCommandList->Close());
	}
	
	{// Post
		ThrowIfFailed(pPostCommandAllocator->Reset());
		ThrowIfFailed(pPostCommandList->Reset(pPostCommandAllocator, mpPipelineState.Get()));

		// Indicate that the back buffer will now be used to present.
		pPostCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mvRenderTargets[miCurrentFrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
		ThrowIfFailed(pPostCommandList->Close());
	}

	// Execute the command list.
	ID3D12CommandList* ppCommandLists[] = { pPreCommandList, mFrameResource[0].mVecMultithreadCMDList[0], mFrameResource[0].mVecMultithreadCMDList[1], pPostCommandList };
	mpCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// 백퍼버 프론트 버퍼 교체
	ThrowIfFailed(mpSwapChain->Present(1, 0));

	WaitForPreviousFrame();
}

#else
void EngineMain::OnRender()
{

	// 장면을 렌더링하는 데 필요한 모든 명령을 명령 목록에 기록합니다.
	// PopulateCommandList();
	// 명령리스트 할당 자들은 연관된
	// 명령 목록이 GPU에서 실행을 마쳤습니다. 앱에서 사용해야하는
	// GPU 실행 진행을 결정하는 울타리.
	ThrowIfFailed(mpCommandAllocator->Reset());

	// 그러나 특정 명령에서 ExecuteCommandList()가 호출되면
	// 목록에서 언제든지 명령 목록을 재설정 할 수 있습니다.
	// 다시 녹음하십시오.
	ThrowIfFailed(mpCommandList->Reset(mpCommandAllocator.Get(), mpPipelineState.Get()));

	// Common state
	{// 뷰포트 렌터타겟 설정
		//cmdLst->IASetIndexBuffer(&mAsteroidIndexBufferView);
		//cmdLst->IASetVertexBuffers(0, 1, &mAsteroidVertexBufferView);
		//cmdLst->RSSetViewports(1, &mViewPort);
		//cmdLst->RSSetScissorRects(1, &mScissorRect);
		//cmdLst->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//cmdLst->OMSetRenderTargets(1, &renderTargetView, true, &mDepthStencilView);

		mpCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		mpCommandList->RSSetViewports(1, &mViewport);
		mpCommandList->RSSetScissorRects(1, &mScissorRect);
		// Indicate that the back buffer will be used as a render target.
		mpCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mvRenderTargets[miCurrentFrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mpRtvHeap->GetCPUDescriptorHandleForHeapStart(), miCurrentFrameIndex, miRtvDescriptorSize);
		mpCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
		// Record commands.
		const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
		mpCommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	}

	{// 리소스 설정, 파이프라인상태 설정
		/*
		티스크립트갯수를 많이 설정한 디스크립트힙 하나를 만들고
		상수버퍼와 이미지를 생성시 디스크립트힙 핸들 주소를 계산해서 오브젝트마다 핸들 주소를 가지고 있도록 한다?
		디스크립트힙 핸들 주소 계산을 쉽게 하기 위해서는 텍스쳐를 먼저 생성하고 상수 버퍼를 나중에 하는방향으로?
		텍스쳐는 고정몃개 맥스치 정해주면 될거같은데 상수버퍼는 기하급수적으로 늘어날수 있어서?
		*/
		mpCommandList->SetGraphicsRootSignature(mpRootSignature.Get());
		ID3D12DescriptorHeap* ppHeaps[] = { mpCBVHeap.Get(), mSampleHeap.Get() };
		mpCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
		for (int i = 0; i < mObj.size(); i++)
		{
			UINT mCbvSrvDescriptorSize = mpDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			if (i < 4)
			{
				D3D12_GPU_DESCRIPTOR_HANDLE CBV(mpCBVHeap->GetGPUDescriptorHandleForHeapStart());
				CBV.ptr += i * mCbvSrvDescriptorSize;// CBV.Offset(i * mCbvSrvDescriptorSize);
				mpCommandList->SetGraphicsRootDescriptorTable(0, CBV);
			}
			else if (i >= 4)
			{
				// 상수버퍼 2개를 초기에 먼저 만들고나서 텍스쳐를 만들었다
				// 키다운을 하면 상수버퍼를 하나 추가 하는데 인덱스 2에 이미 텍스쳐가 있어서 (i+1)해야한다
				D3D12_GPU_DESCRIPTOR_HANDLE CBV(mpCBVHeap->GetGPUDescriptorHandleForHeapStart());
				CBV.ptr += (i + 1) * mCbvSrvDescriptorSize;// CBV.Offset(i * mCbvSrvDescriptorSize);
				mpCommandList->SetGraphicsRootDescriptorTable(0, CBV);
			}
			//D3D12_GPU_DESCRIPTOR_HANDLE CBV(mpCBVHeap->GetGPUDescriptorHandleForHeapStart());
			//UINT mCbvSrvDescriptorSize = mpDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			//CBV.ptr += i * mCbvSrvDescriptorSize;// CBV.Offset(i * mCbvSrvDescriptorSize);
			//mpCommandList->SetGraphicsRootDescriptorTable(0, CBV);
			// t1 디스크립트힙에 생성 할때부터 문제인가?????????? 따른 애들이 만든 프로젝트 보고 참고하자
			D3D12_GPU_DESCRIPTOR_HANDLE SRV(mpCBVHeap->GetGPUDescriptorHandleForHeapStart());
			SRV.ptr += 4 * mCbvSrvDescriptorSize;// CBV.Offset(i * mCbvSrvDescriptorSize);
			mpCommandList->SetGraphicsRootDescriptorTable(1, SRV);
			// t2
			//mpCommandList->SetGraphicsRootDescriptorTable(1, tex);
			// s1
			D3D12_GPU_DESCRIPTOR_HANDLE mSampler(mSampleHeap->GetGPUDescriptorHandleForHeapStart());
			UINT mSmapleDescriptorSize = mpDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
			mSampler.ptr += 1 * mSmapleDescriptorSize;
			mpCommandList->SetGraphicsRootDescriptorTable(2, mSampler);

			mpCommandList->IASetVertexBuffers(0, 1, &mObj[i]->GetVertexBufferView());
			mpCommandList->IASetIndexBuffer(&mObj[i]->GetIndexBufferView());
			mpCommandList->DrawIndexedInstanced(mObj[i]->GetIndexCount(), 1, 0, 0, 0);
		}
	}

	// Indicate that the back buffer will now be used to present.
	mpCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mvRenderTargets[miCurrentFrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	//명령기록이 끝났음을 알린다
	ThrowIfFailed(mpCommandList->Close());

	// Execute the command list.
	ID3D12CommandList* ppCommandLists[] = { mpCommandList.Get() };
	mpCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// 백퍼버 프론트 버퍼 교체
	ThrowIfFailed(mpSwapChain->Present(1, 0));

	WaitForPreviousFrame();
}
#endif // MULTI_THREAD

/* 멀티 스레드 메인 스레드 렌더링 기능
사전 렌더링 명령리스트 (뎁스스텐실 설정)
멀티 스레드 렌더링 명령리스트
사후 렌더링 명령리스트 (스카이박스 UI  이펙트?)

void OnRender_MainThread()
{
	// 렌더링을 시작하도록 각 자식 렌더링 스레드에 알림
	forworkerIdinworkerIdList
	{
		SetEvent(BeginRendering_Events[workerId]);
	}

	// 사전 명령 목록은 렌더링을 준비하는 데 사용됩니다
	// 사전 명령 목록 재설정
	pPreCommandList->Reset(...);

	// 백 버퍼에 대한 프리젠 테이션 상태 사이의 장벽을 렌더링 대상으로 설정
	pPreCommandList->ResourceBarrier(1, (..., D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// 백 버퍼의 색상을 지 웁니다
	pPreCommandList->ClearRenderTargetView(...);

	// 백 버퍼의 깊이 / 템플릿 지우기
	pPreCommandList->ClearDepthStencilView(...);

	// 사전 명령 목록의 다른 작업
	// ...

	// 사전 명령 목록을 닫습니다
	pPreCommandList->Close();

	// Post Command List는 렌더링 후 터치 마무리에 사용됩니다
	// 백 버퍼에 대한 프리젠 테이션 상태 사이의 장벽을 렌더링 대상으로 설정
	pPostCommandList->ResourceBarrier(1, (..., D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	// 사후 명령리스트의 기타 작업
	// ...

	// 사후 명령리스트 닫기
	pPostCommandList->Close();

	// 사전 명령 목록 제출
	pCommandQueue->ExecuteCommandLists(..., pPreCommandList);

	// 모든 작업자 스레드가 Task1가완료 될 때까지 기다립니다.
	WaitForMultipleObjects(Task1_Events);

	// 모든 작업자 스레드에서 Task1에 대한 명령 목록 제출
	pCommandQueue->ExecuteCommandLists(...,pCommandListsForTask1);

	// 모든 작업자 스레드가 Task2가완료 될 때까지 기다립니다.
	WaitForMultipleObjects(Task2_Events);

	// 완성 된 렌더링 명령 제출 (모든 작업자 스레드에서 Task2에 대한 명령 목록)
	pCommandQueue->ExecuteCommandLists(..., pCommandListsForTask2);

	// ...
	// 모든 작업자 스레드가 TaskN을 완료 할 때까지 기다립니다.
	WaitForMultipleObjects(TaskN_Events);

	// 완성 된 렌더링 명령 제출 (모든 작업자 스레드에서 TaskN의 명령 목록)
	pCommandQueue->ExecuteCommandLists(..., pCommandListsForTaskN);

	// 나머지 명령 목록 제출 (pPostCommandList)
	pCommandQueue->ExecuteCommandLists(..., pPostCommandList);

	// SwapChain 프리젠 테이션 사용
	pSwapChain->Present(...);
}
*/


/* 멀티 스레드 자식 스레드 렌더링 기능
void OnRender_WorkerThread(workerId)
{
	// 각 루프는 자식 스레드에서 하나의 프레임 렌더링을 나타냅니다.
	while (running)
	{
		// 메인 스레드의 이벤트 알림이 한 프레임 렌더링을 시작할 때까지 기다립니다.
		WaitForSingleObject(BeginRendering_Events[workerId]);

		// Rendering subtask1
		{
			pCommandList1->SetGraphicsRootSignature(...);
			pCommandList1->IASetVertexBuffers(...);
			pCommandList1->IASetIndexBuffer(...);
			//...
			pCommandList1->DrawIndexedInstanced(...);
			pCommandList1->Close();

			// 현재 작업자 스레드에서 렌더링 하위 작업 1이 완료되었음을 기본 스레드에 알립니다.
			SetEvent(Task1_Events[workerId]);
		}

		// Rendering subtask2
		{
			pCommandList2->SetGraphicsRootSignature(...);
			pCommandList2->IASetVertexBuffers(...);
			pCommandList2->IASetIndexBuffer(...);
			//...
			pCommandList2->DrawIndexedInstanced(...);
			pCommandList2->Close();

			// 현재 작업자 스레드에서 렌더링 서브 태스크 2가 완료되었음을 기본 스레드에 알리십시오.
			SetEvent(Task2_Events[workerId]);
		}

		// 더 많은 렌더링 하위 작업
		//...

		// Rendering subtaskN
		{
			pCommandListN->SetGraphicsRootSignature(...);
			pCommandListN->IASetVertexBuffers(...);
			pCommandListN->IASetIndexBuffer(...);
			//...
			pCommandListN->DrawIndexedInstanced(...);
			pCommandListN->Close();

			// 현재 작업자 스레드에서 렌더링 하위 작업 N이 완료되었음을 기본 스레드에 알립니다.
			SetEvent(TaskN_Events[workerId]);
		}
	}
}
*/