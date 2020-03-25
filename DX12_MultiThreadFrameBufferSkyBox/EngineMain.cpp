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
		switch (wParam) {
		case VK_SPACE:
			AddBuffe();
			break;
		case VK_RIGHT:
			break;
		case VK_UP:
			break;
		case VK_DOWN:
			break;
		}
		return;
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		JInput::instance().OnMouseDown(hwnd, wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return;

		// Mouse button being released (while the cursor is currently over our window)
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		JInput::instance().OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return;

		// Cursor moves over the window (or outside, while we're currently capturing it)
	case WM_MOUSEMOVE:
		JInput::instance().OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		if (wParam & BUTTON_STATE::RIGHT)
		{
			JInputPoint MouseRot = JInput::instance().GetMouseRot();
			mCamera->Rotate(MouseRot.y, MouseRot.x);
		}
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

	{// FrameResource Multithread
		for (int iCount=0; iCount<iFrameCount; iCount++)
		{
			//mFrameResource[iCount].Init(mpDevice.Get());

			{// FenceValue
				mFrameResource[iCount].mFenceValue = 0;
			}

			{// Pre
				ThrowIfFailed(mpDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mFrameResource[iCount].mPreCMDAllocator)));
				ThrowIfFailed(mpDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mFrameResource[iCount].mPreCMDAllocator.Get(), nullptr, IID_PPV_ARGS(&mFrameResource[iCount].mPreCMDList)));
				//ThrowIfFailed(mFrameResource[iCount].mPreCMDList->Close());
			}

			{// SkyBox
				ThrowIfFailed(mpDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mFrameResource[iCount].mSkyCMDAllocator)));
				ThrowIfFailed(mpDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mFrameResource[iCount].mSkyCMDAllocator.Get(), nullptr, IID_PPV_ARGS(&mFrameResource[iCount].mSkyCMDList)));
				ThrowIfFailed(mFrameResource[iCount].mSkyCMDList->Close());
			}

			{// Post
				ThrowIfFailed(mpDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mFrameResource[iCount].mPostCMDAllocator)));
				ThrowIfFailed(mpDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mFrameResource[iCount].mPostCMDAllocator.Get(), nullptr, IID_PPV_ARGS(&mFrameResource[iCount].mPostCMDList)));
				ThrowIfFailed(mFrameResource[iCount].mPostCMDList->Close());
			}

			{// Multithread
				// �����Ӹ��ҽ����� �켱 ��ũ��Ʈ���� �ȳְ� �۾� 
				// ������ ���� �˾ƿ���
				SYSTEM_INFO stSysInfo;
				GetSystemInfo(&stSysInfo);
				iNumberOfProcessors = 4;// static_cast<int>(stSysInfo.dwNumberOfProcessors);
				for (int i = 0; i < iNumberOfProcessors; i++)
				{
					ID3D12CommandAllocator* pCMDAllocator = nullptr;
					ID3D12GraphicsCommandList* pCMDList = nullptr;
					ThrowIfFailed(mpDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&pCMDAllocator)));
					ThrowIfFailed(mpDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, pCMDAllocator, nullptr, IID_PPV_ARGS(&pCMDList)));

					mFrameResource[iCount].mVecMultithreadCMDAllocator.push_back(pCMDAllocator);
					mFrameResource[iCount].mVecMultithreadCMDList.push_back(pCMDList);
					ThrowIfFailed(pCMDList->Close());
				}

			}
		}
	}

	// CreateSwapChain
	{
		// ����ü�� ��ũ����(��������) ����
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.BufferCount = iSwapChainBufferCount;
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
		miCurrentmiBackBufferIndex = mpSwapChain->GetCurrentBackBufferIndex();
	}

	// CreateRenderTarget
	{
		// ����Ÿ���� ��������(���ҽ���) ����
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = iSwapChainBufferCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		mRtvDescriptorHeap.Create(mpDevice.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, iSwapChainBufferCount, false);

		miRtvDescriptorSize = mpDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		// ����Ÿ�Ϻ� ��ũ�������� ù��° ��ġ�� �ڵ�� �޴´�
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mRtvDescriptorHeap.pDH->GetCPUDescriptorHandleForHeapStart());

		// FrameCount��ŭ ����Ÿ��(���ҽ�)�� ���� �Ѵ�
		for (UINT n = 0; n < iSwapChainBufferCount; n++)
		{
			ComPtr<ID3D12Resource> rtt;
			ThrowIfFailed(mpSwapChain->GetBuffer(n, IID_PPV_ARGS(&rtt)));
			mvRenderTargets.push_back(rtt);
			mpDevice->CreateRenderTargetView(mvRenderTargets[n].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(1, miRtvDescriptorSize);
		}
	}

	// ���ؽ� ���� ����
	{
		mObj.push_back(new JMesh("square", mpDevice.Get(), nullptr, XMFLOAT3(0.5f, 0.8f, 0.0f)));
		mObj.push_back(new JMesh("square", mpDevice.Get(), nullptr, XMFLOAT3(-0.1f, 0.3f, 1.0f)));
		mObj.push_back(new JMesh("square", mpDevice.Get(), nullptr, XMFLOAT3(-0.7f, -0.2f, 2.0f)));
		mObj.push_back(new JMesh("square", mpDevice.Get(), nullptr, XMFLOAT3(-1.3f, -0.7f, 3.0f)));
		mSkyBoxObj = new JMesh("square", mpDevice.Get(), nullptr, XMFLOAT3(0.01f, 0.01f, 0.01f));
	}

	// DescriptorHeap �����ÿ� NumDescriptors�� ũ�� ��Ƽ� ����
	UINT NumDescriptors = 131072;// 131072, 8192;//1,000,000+(��ũ��Ʈ ���� �ִ�?) ��ũ��Ʈ���� ������ �ִ� �� ��ũ��Ʈ���� { �������(CBV) + �̹���(SRV) + ���ĵ��������׼�����(UAV) }
	// CreateDescriptorHeap
	{
		// Describe and create a constant buffer view (CBV) descriptor heap.
		// Flags indicate that this descriptor heap can be bound to the pipeline 
		// and that descriptors contained in it can be referenced by a root table.
		/*D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
		cbvHeapDesc.NumDescriptors = NumDescriptors;
		cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		ThrowIfFailed(mpDevice->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&mpCBVHeap)));*/
		mCBVDescriptorHeap.Create(mpDevice.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, NumDescriptors, true);
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
			D3D12_CPU_DESCRIPTOR_HANDLE handle = mCBVDescriptorHeap.Append();
			mpDevice->CreateConstantBufferView(&descBuffer, handle);

			D3D12_GPU_DESCRIPTOR_HANDLE gpuhandle = mCBVDescriptorHeap.GetCurrentGPU();
			mObj[i]->mGPUHANDLE.ptr = gpuhandle.ptr;
		}

		//
		descBuffer.BufferLocation = mpConstantBuffer->GetGPUVirtualAddress() + mObj.size() * mElementByteSize;
		D3D12_CPU_DESCRIPTOR_HANDLE handle = mCBVDescriptorHeap.Append();
		mpDevice->CreateConstantBufferView(&descBuffer, handle);

		D3D12_GPU_DESCRIPTOR_HANDLE gpuhandle = mCBVDescriptorHeap.GetCurrentGPU();
		mSkyBoxObj->mGPUHANDLE.ptr = gpuhandle.ptr;

		ThrowIfFailed(mpConstantBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mPCBVDataBegin)));
	}

	// CreateTexture
	{
		// Texture Load
		auto woodCrateTex = std::make_unique<JTexture>();
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

		// �ؽ��� �� ����
		D3D12_CPU_DESCRIPTOR_HANDLE hDescriptor = mCBVDescriptorHeap.Append();
		D3D12_GPU_DESCRIPTOR_HANDLE hGpuDescriptor = mCBVDescriptorHeap.GetCurrentGPU();
		mTextures["bricks"]->mGPUHANDLE.ptr = hGpuDescriptor.ptr;
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

	// Sky CreateTexture
	{
		// Texture Load
		auto woodCrateTex = std::make_unique<JTexture>();
		woodCrateTex->Name = "skymap0";
		woodCrateTex->Filename = L"Textures/skymap0.dds";
		mTextures["skymap0"] = std::move(woodCrateTex);

		ResourceUploadBatch resourceUpload(mpDevice.Get());
		resourceUpload.Begin();
		ThrowIfFailed(
			CreateDDSTextureFromFile(mpDevice.Get(), resourceUpload, L"Textures/skymap0.dds", mTextures["skymap0"]->Resource.ReleaseAndGetAddressOf())
		);

		// Upload the resources to the GPU.
		auto uploadResourcesFinished = resourceUpload.End(mpCommandQueue.Get());
		// Wait for the upload thread to terminate
		uploadResourcesFinished.wait();

		// �ؽ��� �� ����
		D3D12_CPU_DESCRIPTOR_HANDLE hDescriptor = mCBVDescriptorHeap.Append();
		D3D12_GPU_DESCRIPTOR_HANDLE hGpuDescriptor = mCBVDescriptorHeap.GetCurrentGPU();
		mTextures["skymap0"]->mGPUHANDLE.ptr = hGpuDescriptor.ptr;
		auto woodCrateTexss = mTextures["skymap0"]->Resource;
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = woodCrateTexss->GetDesc().Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = woodCrateTexss->GetDesc().MipLevels;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		mpDevice->CreateShaderResourceView(woodCrateTexss.Get(), &srvDesc, hDescriptor);
	}

	// CreateSampler
	{
		// ���÷� ���� ��ũ��Ʈ ���� ���� ���� SetDescriptorHeaps�Ҷ� CBV_SRV_UAV�� SAMPLER�� �ΰ� �ѱ�� �ȴ�
		// SetGraphicsRootDescriptorTable()�Ҷ��� ��ũ��Ʈ���̺� ������ �ε��� ����ϸ� �ȴ�
		mSampleDescriptorHeap.Create(mpDevice.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 2, true);

		D3D12_SAMPLER_DESC sampler = {};
		sampler.Filter = D3D12_FILTER_ANISOTROPIC;
		sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler.MaxAnisotropy = 2;
		sampler.MinLOD = -1.0f;
		sampler.MaxLOD = 1.0f;
		D3D12_CPU_DESCRIPTOR_HANDLE mSampler = mSampleDescriptorHeap.Append();
		UINT HandleIncrementSize = mpDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
		mpDevice->CreateSampler(&sampler, mSampler);

		mSampler.ptr += static_cast<size_t>(1 * HandleIncrementSize);
		mpDevice->CreateSampler(&sampler, mSampler);
	}

	//Create DSV 
	{
		mDsvDescriptorHeap.Create(mpDevice.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);

		CD3DX12_HEAP_PROPERTIES heapProperty(D3D12_HEAP_TYPE_DEFAULT);

		D3D12_RESOURCE_DESC resourceDesc;
		ZeroMemory(&resourceDesc, sizeof(resourceDesc));
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resourceDesc.Alignment = 0;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.SampleDesc.Quality = 0;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.Width = (UINT)width;
		resourceDesc.Height = (UINT)height;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_CLEAR_VALUE clearVal;
		clearVal = { DXGI_FORMAT_D24_UNORM_S8_UINT , 1.0f };

		ThrowIfFailed(mpDevice->CreateCommittedResource(
			&heapProperty,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&clearVal,
			IID_PPV_ARGS(mDsvTexture.GetAddressOf())
		));
		D3D12_DEPTH_STENCIL_VIEW_DESC depthdesc;
		ZeroMemory(&depthdesc, sizeof(depthdesc));
		depthdesc.Texture2D.MipSlice = 0;
		depthdesc.Format = resourceDesc.Format;
		depthdesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		depthdesc.Flags = D3D12_DSV_FLAG_NONE;

		mpDevice->CreateDepthStencilView(mDsvTexture.Get(), &depthdesc, mDsvDescriptorHeap.pDH->GetCPUDescriptorHandleForHeapStart());

		// Pre CommandList ResourceBarrier
		{
			for (int iCount = 0; iCount < iFrameCount; iCount++)
			{
				ID3D12GraphicsCommandList* pPreCommandList = mFrameResource[iCount].mPreCMDList.Get();
				pPreCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mDsvTexture.Get(),
					D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));
			}
		}

		//���̴��� ���� �ؽ��ķ� ����¹�?
		D3D12_CPU_DESCRIPTOR_HANDLE hDescriptor = mCBVDescriptorHeap.Append();

		D3D12_SHADER_RESOURCE_VIEW_DESC descSRV;
		ZeroMemory(&descSRV, sizeof(descSRV));
		descSRV.Texture2D.MipLevels = resourceDesc.MipLevels;
		descSRV.Texture2D.MostDetailedMip = 0;
		descSRV.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		descSRV.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		descSRV.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		mpDevice->CreateShaderResourceView(mDsvTexture.Get(), &descSRV, hDescriptor);
	}

	// Pre CommandList Close
	{
		for (int iCount = 0; iCount < iFrameCount; iCount++)
		{
			ID3D12GraphicsCommandList* pPreCommandList = mFrameResource[iCount].mPreCMDList.Get();
			pPreCommandList->Close();
		}
	}

	// Shader RootSignature PipelineState Create
	{
		for (int iCount = 0; iCount < eRenderPass_Max; iCount++)
		{
			//  ���Ŀ��� DescriptorHeap(�������, ���÷�)�� ���⼭ �����ϰ� ����
			mRenderPass[iCount].Init(static_cast<eRenderPass>(iCount), mpDevice.Get());
		}
	}

	// CreateFence
	{
		ThrowIfFailed(mpDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence)));
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

	mTimerClass.Initialize();
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
	//for (int i = 0; i < mObj.size(); ++i)
	{
		size_t iAddress = mObj.size();
		// BufferLocation�� �������(����) �ȿ������� ��ġ����
		descBuffer.BufferLocation = mpConstantBuffer->GetGPUVirtualAddress() + iAddress * mElementByteSize;
		D3D12_CPU_DESCRIPTOR_HANDLE handle = mCBVDescriptorHeap.Append();
		// ������� 2���� �ʱ⿡ ���� ������� �ؽ��ĸ� �������
		// Ű�ٿ��� �ϸ� ������۸� �ϳ� �߰� �ϴµ� �ε��� 2�� �̹� �ؽ��İ� �־ (iAddress+1)�ؾ��Ѵ�
		mpDevice->CreateConstantBufferView(&descBuffer, handle);
	}

	ThrowIfFailed(mpConstantBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mPCBVDataBegin)));

	mObj.push_back(new JMesh("square", mpDevice.Get(), nullptr, XMFLOAT3(-1.3f, 0.0f, 0.0f)));

	// �ý��İ� �ϳ� 4���ε����� �־ �������� iLastIndex + 1�� �ѹ��� ���ش�
	size_t iLastIndex = (mObj.size() - 1);
	D3D12_GPU_DESCRIPTOR_HANDLE gpuhandle = mCBVDescriptorHeap.GetCurrentGPU();
	mObj[iLastIndex]->mGPUHANDLE.ptr = gpuhandle.ptr;
}


void EngineMain::OnUpdate()
{
	mTimerClass.Frame();
	float deletaTime = mTimerClass.GetTime();
	mCamera->Update(deletaTime);

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
		ds.x += 0.2f * deletaTime;
		mObj[iCount]->SetPosition(ds);
		mConstantBufferData.world = mObj[iCount]->GetWorld();
		size_t d = (sizeof(SceneConstantBuffer) + 255) & ~255;;
		memcpy(&mPCBVDataBegin[iCount * d], &mConstantBufferData, sizeof(mConstantBufferData));
	}

	{
		SceneConstantBuffer mConstantBufferData;

		XMMATRIX wvpMat = XMLoadFloat4x4(&mSkyBoxObj->mWorld) * viewMat * projMat; // create wvp matrix
		XMMATRIX transposed = XMMatrixTranspose(wvpMat); // must transpose wvp matrix for the gpu
		XMFLOAT4X4 wvp;
		XMStoreFloat4x4(&wvp, transposed); // store transposed wvp matrix in constant buffer
		mConstantBufferData.worldViewProjection = wvp;
		XMFLOAT3 ds = mSkyBoxObj->GetPosition();
		//ds.x += 0.2f * deletaTime;
		mSkyBoxObj->SetPosition(ds);
		mConstantBufferData.world = mSkyBoxObj->GetWorld();
		size_t d = (sizeof(SceneConstantBuffer) + 255) & ~255;;
		memcpy(&mPCBVDataBegin[mObj.size() * d], &mConstantBufferData, sizeof(mConstantBufferData));
	}
}

void EngineMain::OnRender()
{
	/*
	0. SimpleDirectPBR SkyBox.hlsl : https://docs.microsoft.com/ko-kr/windows/win32/direct3d12/specifying-root-signatures-in-hlsl
	���̴� + RootSignature + PipelineState �Ѽ�Ʈ�� ���µ� �ѵ�
	0. PipelineState��ü �����Ӻ� ���� ����� �ڵ� �����
	1. ��ī�� �ڽ� (ElixirEngine.sln)
	2. PBR? ����Ÿ�ٿ�����?
	3. ���۵� ������
	4. ���̴� �ڿ� ���� ������� �ؽ��� ���÷�
	*/
	concurrency::parallel_for<UINT>(0, iNumberOfProcessors, [&](UINT threadIndex) 
	{
		ID3D12CommandAllocator* pCommandAllocator = mFrameResource[mCurrentFrameIndex].mVecMultithreadCMDAllocator[threadIndex];
		ID3D12GraphicsCommandList* pCommandList = mFrameResource[mCurrentFrameIndex].mVecMultithreadCMDList[threadIndex];
		ThrowIfFailed(pCommandAllocator->Reset());
		ThrowIfFailed(pCommandList->Reset(pCommandAllocator, mRenderPass[eRenderPass_Normal].mpPipelineState.Get()));

		pCommandList->SetPipelineState(mRenderPass[eRenderPass_Normal].mpPipelineState.Get());
		pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pCommandList->RSSetViewports(1, &mViewport);
		pCommandList->RSSetScissorRects(1, &mScissorRect);
		
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mRtvDescriptorHeap.pDH->GetCPUDescriptorHandleForHeapStart(), miCurrentmiBackBufferIndex, miRtvDescriptorSize);
		pCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &mDsvDescriptorHeap.pDH->GetCPUDescriptorHandleForHeapStart());

		{// ���ҽ� ����, ���������λ��� ����
			/*
			Ƽ��ũ��Ʈ������ ���� ������ ��ũ��Ʈ�� �ϳ��� �����
			������ۿ� �̹����� ������ ��ũ��Ʈ�� �ڵ� �ּҸ� ����ؼ� ������Ʈ���� �ڵ� �ּҸ� ������ �ֵ��� �Ѵ�?
			��ũ��Ʈ�� �ڵ� �ּ� ����� ���� �ϱ� ���ؼ��� �ؽ��ĸ� ���� �����ϰ� ��� ���۸� ���߿� �ϴ¹�������?
			�ؽ��Ĵ� �����밳 �ƽ�ġ �����ָ� �ɰŰ����� ������۴� ���ϱ޼������� �þ�� �־?
			*/
			pCommandList->SetGraphicsRootSignature(mRenderPass[eRenderPass_Normal].mpRootSignature.Get());
			// ��ũ��Ʈ���� ���������� ������ �ȵȴ� D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV�� ���� Ÿ�� ���� �ȵ�
			ID3D12DescriptorHeap* ppHeaps[] = { mCBVDescriptorHeap.pDH.Get(), mSampleDescriptorHeap.pDH.Get() };
			pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
			for (int i = threadIndex; i < mObj.size(); i += iNumberOfProcessors)
			{
				pCommandList->SetGraphicsRootDescriptorTable(0, mObj[i]->mGPUHANDLE);
				pCommandList->SetGraphicsRootDescriptorTable(1, mTextures["bricks"]->mGPUHANDLE);

				D3D12_GPU_DESCRIPTOR_HANDLE mSampler = mSampleDescriptorHeap.GetCurrentGPU(0);;
				UINT mSmapleDescriptorSize = mpDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
				mSampler.ptr += 0 * mSmapleDescriptorSize;
				pCommandList->SetGraphicsRootDescriptorTable(2, mSampler);

				pCommandList->IASetVertexBuffers(0, 1, &mObj[i]->GetVertexBufferView());
				pCommandList->IASetIndexBuffer(&mObj[i]->GetIndexBufferView());
				pCommandList->DrawIndexedInstanced(mObj[i]->GetIndexCount(), 1, 0, 0, 0);
				/*UINT mCbvSrvDescriptorSize = mpDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				if (i < 4)
				{
					D3D12_GPU_DESCRIPTOR_HANDLE CBV(mpCBVHeap->GetGPUDescriptorHandleForHeapStart());
					CBV.ptr += i * mCbvSrvDescriptorSize;// CBV.Offset(i * mCbvSrvDescriptorSize);
					pCommandList->SetGraphicsRootDescriptorTable(0, CBV);
				}
				else if (i >= 4)
				{
					// ������� 2���� �ʱ⿡ ���� ������� �ؽ��ĸ� �������
					// Ű�ٿ��� �ϸ� ������۸� �ϳ� �߰� �ϴµ� �ε��� 2�� �̹� �ؽ��İ� �־ (i+1)�ؾ��Ѵ�
					D3D12_GPU_DESCRIPTOR_HANDLE CBV(mpCBVHeap->GetGPUDescriptorHandleForHeapStart());
					CBV.ptr += (i + 1) * mCbvSrvDescriptorSize;// CBV.Offset(i * mCbvSrvDescriptorSize);
					pCommandList->SetGraphicsRootDescriptorTable(0, CBV);
				}

				//D3D12_GPU_DESCRIPTOR_HANDLE CBV(mpCBVHeap->GetGPUDescriptorHandleForHeapStart());
				//UINT mCbvSrvDescriptorSize = mpDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				//CBV.ptr += i * mCbvSrvDescriptorSize;// CBV.Offset(i * mCbvSrvDescriptorSize);
				//pCommandList->SetGraphicsRootDescriptorTable(0, CBV);
				// t1 ��ũ��Ʈ���� ���� �Ҷ����� �����ΰ�?????????? ���� �ֵ��� ���� ������Ʈ ���� ��������
				D3D12_GPU_DESCRIPTOR_HANDLE SRV(mpCBVHeap->GetGPUDescriptorHandleForHeapStart());
				SRV.ptr += 4 * mCbvSrvDescriptorSize;// CBV.Offset(i * mCbvSrvDescriptorSize);
				pCommandList->SetGraphicsRootDescriptorTable(1, SRV);
				// t2
				//pCommandList->SetGraphicsRootDescriptorTable(1, tex);
				// s1
				D3D12_GPU_DESCRIPTOR_HANDLE mSampler(mpSampleHeap->GetGPUDescriptorHandleForHeapStart());
				UINT mSmapleDescriptorSize = mpDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
				mSampler.ptr += 0 * mSmapleDescriptorSize;
				pCommandList->SetGraphicsRootDescriptorTable(2, mSampler);

				pCommandList->IASetVertexBuffers(0, 1, &mObj[i]->GetVertexBufferView());
				pCommandList->IASetIndexBuffer(&mObj[i]->GetIndexBufferView());
				pCommandList->DrawIndexedInstanced(mObj[i]->GetIndexCount(), 1, 0, 0, 0);*/
			}
		}
		//��ɱ���� �������� �˸���
		ThrowIfFailed(pCommandList->Close());
	});
	

	ID3D12CommandAllocator* pPreCommandAllocator = mFrameResource[mCurrentFrameIndex].mPreCMDAllocator.Get();
	ID3D12GraphicsCommandList* pPreCommandList = mFrameResource[mCurrentFrameIndex].mPreCMDList.Get();

	ID3D12CommandAllocator* pSkyCommandAllocator = mFrameResource[mCurrentFrameIndex].mSkyCMDAllocator.Get();
	ID3D12GraphicsCommandList* pSkyCommandList = mFrameResource[mCurrentFrameIndex].mSkyCMDList.Get();

	ID3D12CommandAllocator* pPostCommandAllocator = mFrameResource[mCurrentFrameIndex].mPostCMDAllocator.Get();
	ID3D12GraphicsCommandList* pPostCommandList = mFrameResource[mCurrentFrameIndex].mPostCMDList.Get();

	{// Pre
		ThrowIfFailed(pPreCommandAllocator->Reset());
		ThrowIfFailed(pPreCommandList->Reset(pPreCommandAllocator, mRenderPass[eRenderPass_Normal].mpPipelineState.Get()));

		pPreCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mvRenderTargets[miCurrentmiBackBufferIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mRtvDescriptorHeap.pDH->GetCPUDescriptorHandleForHeapStart(), miCurrentmiBackBufferIndex, miRtvDescriptorSize);
		pPreCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &mDsvDescriptorHeap.pDH->GetCPUDescriptorHandleForHeapStart());
		// Record commands.
		const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
		pPreCommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		pPreCommandList->ClearDepthStencilView(mDsvDescriptorHeap.pDH->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0xff, 0, nullptr);
		ThrowIfFailed(pPreCommandList->Close());
	}
	
	{// Sky
		ThrowIfFailed(pSkyCommandAllocator->Reset());
		ThrowIfFailed(pSkyCommandList->Reset(pSkyCommandAllocator, mRenderPass[eRenderPass_Sky].mpPipelineState.Get()));

		///////////
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mRtvDescriptorHeap.pDH->GetCPUDescriptorHandleForHeapStart(), miCurrentmiBackBufferIndex, miRtvDescriptorSize);
		pSkyCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &mDsvDescriptorHeap.pDH->GetCPUDescriptorHandleForHeapStart());
		pSkyCommandList->RSSetViewports(1, &mViewport);
		pSkyCommandList->RSSetScissorRects(1, &mScissorRect);

		pSkyCommandList->SetPipelineState(mRenderPass[eRenderPass_Sky].mpPipelineState.Get());
		pSkyCommandList->SetGraphicsRootSignature(mRenderPass[eRenderPass_Sky].mpRootSignature.Get());
		pSkyCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		ID3D12DescriptorHeap* ppHeaps[] = { mCBVDescriptorHeap.pDH.Get(), mSampleDescriptorHeap.pDH.Get() };
		pSkyCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

		pSkyCommandList->SetGraphicsRootDescriptorTable(0, mSkyBoxObj->mGPUHANDLE);
		pSkyCommandList->SetGraphicsRootDescriptorTable(1, mTextures["skymap0"]->mGPUHANDLE);
		D3D12_GPU_DESCRIPTOR_HANDLE mSampler = mSampleDescriptorHeap.GetCurrentGPU(0);;
		UINT mSmapleDescriptorSize = mpDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
		mSampler.ptr += 0 * mSmapleDescriptorSize;
		pSkyCommandList->SetGraphicsRootDescriptorTable(2, mSampler);

		pSkyCommandList->IASetVertexBuffers(0, 1, &mSkyBoxObj->GetVertexBufferView());
		pSkyCommandList->IASetIndexBuffer(&mSkyBoxObj->GetIndexBufferView());
		pSkyCommandList->DrawIndexedInstanced(mSkyBoxObj->GetIndexCount(), 1, 0, 0, 0);
		///////////

		// Indicate that the back buffer will now be used to present.
		ThrowIfFailed(pSkyCommandList->Close());
	}

	{// Post
		ThrowIfFailed(pPostCommandAllocator->Reset());
		ThrowIfFailed(pPostCommandList->Reset(pPostCommandAllocator, mRenderPass[eRenderPass_Post].mpPipelineState.Get()));

		// Indicate that the back buffer will now be used to present.
		pPostCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mvRenderTargets[miCurrentmiBackBufferIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
		ThrowIfFailed(pPostCommandList->Close());
	}

	// Execute the command list.
	std::vector<ID3D12GraphicsCommandList*> ppCommandLists;

	ppCommandLists.push_back(pPreCommandList);
	for (int iCount=0; iCount< mFrameResource[mCurrentFrameIndex].mVecMultithreadCMDList.size(); iCount++)
	{
		ppCommandLists.push_back(mFrameResource[mCurrentFrameIndex].mVecMultithreadCMDList[iCount]);
	}
	ppCommandLists.push_back(pSkyCommandList);
	ppCommandLists.push_back(pPostCommandList);

	mpCommandQueue->ExecuteCommandLists(
		(UINT)ppCommandLists.size(), 
		reinterpret_cast<ID3D12CommandList * const *>(ppCommandLists.data())
	);

	/*ID3D12CommandList* ppCommandLists[] = {
		pPreCommandList,
		mFrameResource[mCurrentFrameIndex].mVecMultithreadCMDList[0],
		mFrameResource[mCurrentFrameIndex].mVecMultithreadCMDList[1],
		mFrameResource[mCurrentFrameIndex].mVecMultithreadCMDList[2],
		mFrameResource[mCurrentFrameIndex].mVecMultithreadCMDList[3],
		pSkyCommandList,
		pPostCommandList };
	mpCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);*/

	// ���۹� ����Ʈ ���� ��ü
	ThrowIfFailed(mpSwapChain->Present(1, 0));

	WaitForPreviousFrame();
}

void EngineMain::WaitForPreviousFrame()
{
	/*
	// ����ϱ� ���� �������� ��ٷȴٰ� ����ϴ� ���� ���� �ٶ������� �ʽ��ϴ�.
	// �̰��� �ܼ�ȭ�� ���� ���� �� �ڵ��Դϴ�. D3D12HelloFrameBuffering
	// ȿ������ �ڿ� ����� ���� �潺�� ����ϴ� ����� �����ִ� �����Դϴ�.
	// GPU Ȱ�뵵�� �ش�ȭ�մϴ�.

	// ��Ÿ�� ���� ��ȣ�ϰ� ������ŵ�ϴ�.
	UINT64 fence = mFrameResource[mCurrentFrameIndex].mFenceValue;
	ThrowIfFailed(mpCommandQueue->Signal(mFence.Get(), fence));
	mFrameResource[mCurrentFrameIndex].mFenceValue = (fence + 1);

	// ���� �������� ���� ������ ��ٸ��ϴ�.
	if (mFence->GetCompletedValue() < fence)
	{
		ThrowIfFailed(mFence->SetEventOnCompletion(fence, mFenceEvent));
		WaitForSingleObject(mFenceEvent, INFINITE);
	}

	miCurrentmiBackBufferIndex = mpSwapChain->GetCurrentBackBufferIndex();

	mCurrentFrameIndex = (mCurrentFrameIndex + 1) % iFrameCount;*/

	// D3D12HelloFrameBuffering ������Ʈ�� ���� ���α׷��� ��ĥ�� ���� ó�� �ϴµ� ���߿� ����(OnDestroy -> WaitForGpu)
	// Schedule a Signal command in the queue.
	const UINT64 currentFenceValue = mFrameResource[mCurrentFrameIndex].mFenceValue;
	ThrowIfFailed(mpCommandQueue->Signal(mFence.Get(), currentFenceValue));

	// Update the frame index.
	miCurrentmiBackBufferIndex = mpSwapChain->GetCurrentBackBufferIndex();

	// If the next frame is not ready to be rendered yet, wait until it is ready.
	if (mFence->GetCompletedValue() < mFrameResource[mCurrentFrameIndex].mFenceValue)
	{
		ThrowIfFailed(mFence->SetEventOnCompletion(mFrameResource[mCurrentFrameIndex].mFenceValue, mFenceEvent));
		WaitForSingleObjectEx(mFenceEvent, INFINITE, FALSE);
	}

	// Set the fence value for the next frame.
	mFrameResource[mCurrentFrameIndex].mFenceValue = currentFenceValue + 1;

	mCurrentFrameIndex = (mCurrentFrameIndex + 1) % iFrameCount;
}

/* ��Ƽ ������ ���� ������ ������ ���
���� ������ ��ɸ���Ʈ (�������ٽ� ����)
��Ƽ ������ ������ ��ɸ���Ʈ
���� ������ ��ɸ���Ʈ (��ī�̹ڽ� UI  ����Ʈ?)

void OnRender_MainThread()
{
	// �������� �����ϵ��� �� �ڽ� ������ �����忡 �˸�
	forworkerIdinworkerIdList
	{
		SetEvent(BeginRendering_Events[workerId]);
	}

	// ���� ��� ����� �������� �غ��ϴ� �� ���˴ϴ�
	// ���� ��� ��� �缳��
	pPreCommandList->Reset(...);

	// �� ���ۿ� ���� ������ ���̼� ���� ������ �庮�� ������ ������� ����
	pPreCommandList->ResourceBarrier(1, (..., D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// �� ������ ������ �� ��ϴ�
	pPreCommandList->ClearRenderTargetView(...);

	// �� ������ ���� / ���ø� �����
	pPreCommandList->ClearDepthStencilView(...);

	// ���� ��� ����� �ٸ� �۾�
	// ...

	// ���� ��� ����� �ݽ��ϴ�
	pPreCommandList->Close();

	// Post Command List�� ������ �� ��ġ �������� ���˴ϴ�
	// �� ���ۿ� ���� ������ ���̼� ���� ������ �庮�� ������ ������� ����
	pPostCommandList->ResourceBarrier(1, (..., D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	// ���� ��ɸ���Ʈ�� ��Ÿ �۾�
	// ...

	// ���� ��ɸ���Ʈ �ݱ�
	pPostCommandList->Close();

	// ���� ��� ��� ����
	pCommandQueue->ExecuteCommandLists(..., pPreCommandList);

	// ��� �۾��� �����尡 Task1���Ϸ� �� ������ ��ٸ��ϴ�.
	WaitForMultipleObjects(Task1_Events);

	// ��� �۾��� �����忡�� Task1�� ���� ��� ��� ����
	pCommandQueue->ExecuteCommandLists(...,pCommandListsForTask1);

	// ��� �۾��� �����尡 Task2���Ϸ� �� ������ ��ٸ��ϴ�.
	WaitForMultipleObjects(Task2_Events);

	// �ϼ� �� ������ ��� ���� (��� �۾��� �����忡�� Task2�� ���� ��� ���)
	pCommandQueue->ExecuteCommandLists(..., pCommandListsForTask2);

	// ...
	// ��� �۾��� �����尡 TaskN�� �Ϸ� �� ������ ��ٸ��ϴ�.
	WaitForMultipleObjects(TaskN_Events);

	// �ϼ� �� ������ ��� ���� (��� �۾��� �����忡�� TaskN�� ��� ���)
	pCommandQueue->ExecuteCommandLists(..., pCommandListsForTaskN);

	// ������ ��� ��� ���� (pPostCommandList)
	pCommandQueue->ExecuteCommandLists(..., pPostCommandList);

	// SwapChain ������ ���̼� ���
	pSwapChain->Present(...);
}
*/


/* ��Ƽ ������ �ڽ� ������ ������ ���
void OnRender_WorkerThread(workerId)
{
	// �� ������ �ڽ� �����忡�� �ϳ��� ������ �������� ��Ÿ���ϴ�.
	while (running)
	{
		// ���� �������� �̺�Ʈ �˸��� �� ������ �������� ������ ������ ��ٸ��ϴ�.
		WaitForSingleObject(BeginRendering_Events[workerId]);

		// Rendering subtask1
		{
			pCommandList1->SetGraphicsRootSignature(...);
			pCommandList1->IASetVertexBuffers(...);
			pCommandList1->IASetIndexBuffer(...);
			//...
			pCommandList1->DrawIndexedInstanced(...);
			pCommandList1->Close();

			// ���� �۾��� �����忡�� ������ ���� �۾� 1�� �Ϸ�Ǿ����� �⺻ �����忡 �˸��ϴ�.
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

			// ���� �۾��� �����忡�� ������ ���� �½�ũ 2�� �Ϸ�Ǿ����� �⺻ �����忡 �˸��ʽÿ�.
			SetEvent(Task2_Events[workerId]);
		}

		// �� ���� ������ ���� �۾�
		//...

		// Rendering subtaskN
		{
			pCommandListN->SetGraphicsRootSignature(...);
			pCommandListN->IASetVertexBuffers(...);
			pCommandListN->IASetIndexBuffer(...);
			//...
			pCommandListN->DrawIndexedInstanced(...);
			pCommandListN->Close();

			// ���� �۾��� �����忡�� ������ ���� �۾� N�� �Ϸ�Ǿ����� �⺻ �����忡 �˸��ϴ�.
			SetEvent(TaskN_Events[workerId]);
		}
	}
}
*/