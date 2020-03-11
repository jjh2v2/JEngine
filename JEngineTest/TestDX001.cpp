#include "stdafx.h"
#include "TestDX001.h"

void TestDX001::Test()
{
	// 디스크립트힙은 재성성 안해도 된다

	// 상수 버퍼 하나로 같은 오브젝트를 관리 할수도 있지만
	// 오브젝트하나당 상수 버퍼 하나씩 만들어도 된다 이게 편할듯 하다 같은 오브젝트가 다수면 인스턴스렌더링을 해주면 되고

	// 상수 버퍼 재 생성
	mConstantBuffer->Unmap(0, nullptr);
	mConstantBuffer->Release();
	mConstantBuffer.Detach();

	UINT mElementByteSize = sizeof(SceneConstantBuffer);
	// (sizeof(SceneConstantBuffer) + 255) & ~255;    // CB size is required to be 256-byte aligned.
	mElementByteSize = (mElementByteSize + 255) & ~255;

    ThrowIfFailed(DXPipelineManager::Get()->GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(mElementByteSize * mObj.size() + 1),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&mConstantBuffer)));

	// Describe and create a constant buffer view.
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = mConstantBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = mElementByteSize;    // CB size is required to be 256-byte aligned.
	DXPipelineManager::Get()->GetDevice()->CreateConstantBufferView(&cbvDesc, mCBVHeap->GetCPUDescriptorHandleForHeapStart());
	ThrowIfFailed(mConstantBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mPCBVDataBegin)));
	
	mObj.push_back(new Mesh("square", DXPipelineManager::Get()->GetDevice().Get(), mCommandList.Get(), XMFLOAT3(-1.0f, 0.0f, 0.0f)));


	// 텍스쳐 재 생성
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mCBVHeap->GetCPUDescriptorHandleForHeapStart());
	auto woodCrateTexss = mTextures["bricks"]->Resource;
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = woodCrateTexss->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = woodCrateTexss->GetDesc().MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	mDevice->CreateShaderResourceView(woodCrateTexss.Get(), &srvDesc, hDescriptor);
}

void TestDX001::HandleWindowsCallback(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{

	case WM_KEYDOWN:
		Test();
		return;
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		JInput::instance().OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
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

/*******************************************
생성자 소멸자
*******************************************/
TestDX001::TestDX001()
{
}

TestDX001::~TestDX001()
{
}

/*******************************************
Init
*******************************************/
void TestDX001::Init()
{
	mViewport.Width = static_cast<float>(gWndProp.width);
	mViewport.Height = static_cast<float>(gWndProp.height);
	mViewport.MaxDepth = 1.0f;
	mScissorRect.right = static_cast<LONG>(gWndProp.width);
	mScissorRect.bottom = static_cast<LONG>(gWndProp.height);

	mAspectRatio = static_cast<float>(gWndProp.width) / static_cast<float>(gWndProp.height);

	mCamera = new Camera((float)gWndProp.width, (float)gWndProp.height);

	DXPipelineManager::Get()->Init(mFrameIndex, FrameCount);
	mDevice = DXPipelineManager::Get()->GetDevice();
	mCommandQueue = DXPipelineManager::Get()->GetCommandQueue();
	mCommandAllocator = DXPipelineManager::Get()->GetCommandAllocator();
	mCommandList = DXPipelineManager::Get()->GetCommandList();

	mSwapChain = DXPipelineManager::Get()->GetSwapChain();
	mRtvHeap = DXPipelineManager::Get()->GetRtvHeap();
	mRenderTargets = DXPipelineManager::Get()->GetRenderTargets();
	mRtvDescriptorSize = DXPipelineManager::Get()->GetRtvDescriptorSize();

	DXAssetsManager::Get()->Init();
	mVertexShader = DXAssetsManager::Get()->GetAssetsShader("shaders_0").GetVertexShader();
	mPixelShader = DXAssetsManager::Get()->GetAssetsShader("shaders_0").GetPixelShader();
	mRootSignature = DXAssetsManager::Get()->GetAssetsRootSignature("shaders_0").GetRootSignature();
	mPipelineState = DXAssetsManager::Get()->GetAssetsPipelineState("shaders_0").GetPipelineState();

	mCBVHeap = DXAssetsManager::Get()->GetAssetsConstantBuffer("shaders_0").GetCBVHeap();
	mConstantBuffer = DXAssetsManager::Get()->GetAssetsConstantBuffer("shaders_0").GetCBVBuffer();

	ThrowIfFailed(mConstantBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mPCBVDataBegin)));
	//memcpy(&mPCBVDataBegin[0 * 256], &mConstantBufferData, sizeof(mConstantBufferData));
	//mConstantBuffer->Unmap(0, nullptr);

	////////////////
	auto woodCrateTex = std::make_unique<Texture>();
	woodCrateTex->Name = "bricks";
	woodCrateTex->Filename = L"Textures/bricks.dds";
	mTextures["bricks"] = std::move(woodCrateTex);

	ResourceUploadBatch resourceUpload(mDevice.Get());
	resourceUpload.Begin();
	ThrowIfFailed(
		CreateDDSTextureFromFile(mDevice.Get(), resourceUpload, L"Textures/bricks.dds", mTextures["bricks"]->Resource.ReleaseAndGetAddressOf())
	);

	// Upload the resources to the GPU.
	auto uploadResourcesFinished = resourceUpload.End(mCommandQueue.Get());
	// Wait for the upload thread to terminate
	uploadResourcesFinished.wait();

	// Fill out the heap with actual descriptors.
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mCBVHeap->GetCPUDescriptorHandleForHeapStart());

	auto woodCrateTexss = mTextures["bricks"]->Resource;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = woodCrateTexss->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = woodCrateTexss->GetDesc().MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	mDevice->CreateShaderResourceView(woodCrateTexss.Get(), &srvDesc, hDescriptor);
	////////////////


	//CreateFence
	ThrowIfFailed(mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence)));
	mFenceValue = 1;
	// Create an event handle to use for frame synchronization.
	mFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (mFenceEvent == nullptr)
	{
		ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
	}
	//CreateFence

	// Wait for the command list to execute; we are reusing the same command 
	// list in our main loop but for now, we just want to wait for setup to 
	// complete before continuing.
	WaitForPreviousFrame();

	// 버텍스 버퍼 생성
	mObj.push_back(new Mesh("square", DXPipelineManager::Get()->GetDevice().Get(), mCommandList.Get(), XMFLOAT3(0.5f, 0.0f, 0.0f)));
	mObj.push_back(new Mesh("square", DXPipelineManager::Get()->GetDevice().Get(), mCommandList.Get(), XMFLOAT3(-0.5f, 0.0f, 0.0f)));

	//명령기록이 끝났음을 알린다
	ThrowIfFailed(mCommandList->Close());
}

/*******************************************
OnUpdate
*******************************************/

void TestDX001::OnUpdate(float deltaTime)
{
	mCamera->Update(deltaTime);

	XMMATRIX viewMat = XMLoadFloat4x4(&mCamera->GetViewMatrix()); // load view matrix
	XMMATRIX projMat = XMLoadFloat4x4(&mCamera->GetProjectionMatrix()); // load projection matrix
	for (int iCount = 0; iCount < mObj.size(); iCount++)
	{
		XMMATRIX wvpMat = XMLoadFloat4x4(&mObj[iCount]->mWorld) * viewMat * projMat; // create wvp matrix
		XMMATRIX transposed = XMMatrixTranspose(wvpMat); // must transpose wvp matrix for the gpu
		XMFLOAT4X4 wvp;
		XMStoreFloat4x4(&wvp, transposed); // store transposed wvp matrix in constant buffer
		mConstantBufferData.worldViewProjection = wvp;
		XMFLOAT3 ds = mObj[iCount]->GetPosition();
		ds.x += 0.001f/* * deltaTime*/;
		mObj[iCount]->SetPosition(ds);
		mConstantBufferData.world = mObj[iCount]->GetWorld();
		size_t d = DXAssetsConstantBuffer::CalcConstantBufferByteSize(sizeof(SceneConstantBuffer));
		memcpy(&mPCBVDataBegin[iCount * d], &mConstantBufferData, sizeof(mConstantBufferData));
	}
}

/*******************************************
OnRender
*******************************************/

void TestDX001::OnRender()
{
	// 장면을 렌더링하는 데 필요한 모든 명령을 명령 목록에 기록합니다.
	//PopulateCommandList();
	// 명령리스트 할당 자들은 연관된
	// 명령 목록이 GPU에서 실행을 마쳤습니다. 앱에서 사용해야하는
	// GPU 실행 진행을 결정하는 울타리.
	ThrowIfFailed(mCommandAllocator->Reset());

	// 그러나 특정 명령에서 ExecuteCommandList()가 호출되면
	// 목록에서 언제든지 명령 목록을 재설정 할 수 있습니다.
	// 다시 녹음하십시오.
	ThrowIfFailed(mCommandList->Reset(mCommandAllocator.Get(), mPipelineState.Get()));

	// Set necessary state.
	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

	ID3D12DescriptorHeap* ppHeaps[] = { mCBVHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	//mCommandList->SetGraphicsRootDescriptorTable(0, mCBVHeap->GetGPUDescriptorHandleForHeapStart());
	// SetGraphicsRootDescriptorTable 이걸로도 상수버퍼할수 있는거 같다
	// SetGraphicsRootConstantBufferView 오브젝트 개별 상수버퍼 적용?
	// SetShaderResourceView 오브젝트 개별 택스쳐 적용?

	mCommandList->RSSetViewports(1, &mViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	// Indicate that the back buffer will be used as a render target.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mRenderTargets[mFrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), mFrameIndex, mRtvDescriptorSize);
	mCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	// Record commands.
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	mCommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	for (int i = 0; i < mObj.size(); i++)
	{
		UINT objCBByteSize = (sizeof(SceneConstantBuffer) + 255) & ~255;
		D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = mConstantBuffer->GetGPUVirtualAddress() + i * objCBByteSize;
		mCommandList->SetGraphicsRootConstantBufferView(2, objCBAddress);
		//CD3DX12_GPU_DESCRIPTOR_HANDLE er33(mCBVHeap->GetGPUDescriptorHandleForHeapStart());
		//UINT mCbvSrvDescriptorSize222 = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		//er33.Offset(0, mCbvSrvDescriptorSize222);
		//mCommandList->SetGraphicsRootDescriptorTable(2, er33);

		CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mCBVHeap->GetGPUDescriptorHandleForHeapStart());
		UINT mCbvSrvDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		tex.Offset(0, mCbvSrvDescriptorSize);
		// t1
		mCommandList->SetGraphicsRootDescriptorTable(0, tex);
		// t2
		mCommandList->SetGraphicsRootDescriptorTable(1, tex);
		//

		mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		mCommandList->IASetVertexBuffers(0, 1, &mObj[i]->GetVertexBufferView());
		mCommandList->IASetIndexBuffer(&mObj[i]->GetIndexBufferView());
		mCommandList->DrawIndexedInstanced(mObj[i]->GetIndexCount(), 1, 0, 0, 0);
	}

	// Indicate that the back buffer will now be used to present.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mRenderTargets[mFrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	//명령기록이 끝났음을 알린다
	ThrowIfFailed(mCommandList->Close());

	// Execute the command list.
	ID3D12CommandList* ppCommandLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// 백퍼버 프론트 버퍼 교체
	ThrowIfFailed(mSwapChain->Present(1, 0));

	WaitForPreviousFrame();
}

/*******************************************
WaitForPreviousFrame
*******************************************/

void TestDX001::WaitForPreviousFrame()
{

	// 계속하기 전에 프레임을 기다렸다가 계속하는 것이 가장 바람직하지 않습니다.
	// 이것은 단순화를 위해 구현 된 코드입니다. D3D12HelloFrameBuffering
	// 효율적인 자원 사용을 위해 펜스를 사용하는 방법을 보여주는 샘플입니다.
	// GPU 활용도를 극대화합니다.

	// 울타리 값을 신호하고 증가시킵니다.
	const UINT64 fence = mFenceValue;
	ThrowIfFailed(mCommandQueue->Signal(mFence.Get(), fence));
	mFenceValue++;

	// 이전 프레임이 끝날 때까지 기다립니다.
	if (mFence->GetCompletedValue() < fence)
	{
		ThrowIfFailed(mFence->SetEventOnCompletion(fence, mFenceEvent));
		WaitForSingleObject(mFenceEvent, INFINITE);
	}

	mFrameIndex = mSwapChain->GetCurrentBackBufferIndex();
}