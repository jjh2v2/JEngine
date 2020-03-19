#pragma once
#include "stdafx.h"
#include "Mesh.h"
#include "Camera.h"
#include "d3dUtil.h"
#include "JInput.h"
#include "timerclass.h"
#include "DirectXTK12/ResourceUploadBatch.h"
#include "DescriptorHeapObject.h"
#include "RenderPassResource.h"

const UINT		iSwapChainBufferCount = 3;
const UINT		iFrameCount = 3;

class JMesh : public Mesh
{
public:
	D3D12_GPU_DESCRIPTOR_HANDLE mGPUHANDLE;
	JMesh(std::string objFile, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, XMFLOAT3 _position)
		: Mesh(objFile, device, commandList, _position)
	{
	};
};

class JTexture : public Texture
{
public:
	D3D12_GPU_DESCRIPTOR_HANDLE mGPUHANDLE;
};

struct SceneConstantBuffer
{
	XMFLOAT4X4 worldViewProjection;
	XMFLOAT4X4 world;
	//XMFLOAT4X4 model;
	//XMFLOAT4X4 view;
	//XMFLOAT4X4 projection;
	//XMFLOAT4 ambientColor;
	//BOOL sampleShadowMap;
	//BOOL padding[3];        // Must be aligned to be made up of N float4s.
	//LightState lights[NumLights];
};


class FrameResource
{
public:
	/*
	ID3D12CommandAllocator
	command list에 추가된 명령들은 이 Allocator의 메모리에 저장이 된다.
	
	ID3D12GraphicsCommandList
	그리기 명령들이 즉시 실행되는것이 아니라 명령들을 command list에 추가만 한다.
	(ExecuteCommandLists는 바로 실행하는것이 아니라 queue에 추가를 하는것이다.)
	GPU가 queue에 쌓인 명령들을 뽑아서 실행한다.
	명령들을 command list에 모두 추가 한 뒤
	위의 ExecuteCommandLists를 실행 하기 전에 ID3D12GraphicsCommandList::Close를 호출해서
	Direct3D에게 명령 기록이 끝났다고 알려주어야 한다.
	현재 명령들을 추가하는 명령 목록을 제외한 모든 명령 목록은 Close로 다 닫혀있어야 하고
	command list를 생성하거나 재설정하게 되면 command list를 열려있는 상태가 되어있음을 주의해야 한다.
	그리고 ID3D12CommandList::Reset 메서드라는것이 있는데
	이것은 내부의 메모리들을 새로운 명령들을 기록하는데 재사용 할 수 있게 한다.
	command list를 새로 처음 생성했을 때와 같은 상태로 만들지만 command list에 있는 명령들에게는 영향이 미치지 않는다.
	그리고 ID3D12CommandAllocator::Reset메서드는 명령 할당자의 메모리를 다음 프레임을 위해서 재사용 할 수 있게 해준다.
	위의 Reset들은 delete로 싹 날리고 new로 다시 메모리 공간을 잡는것이 아닌
	내부를 처음처럼 비워준다는 생각을 하면 될것 같다.
	(std::vector::clear 처럼...)
	주의할점은 GPU가 명령 할당자에 담긴 모든 명령을 실행 했음이 확실해지기 전까지는 Reset으로 재설정 하지 말아야 한다는점.
	*/

	// 멀티쓰레드용
	std::vector<ID3D12GraphicsCommandList*>			mVecMultithreadCMDList;
	std::vector<ID3D12CommandAllocator*>			mVecMultithreadCMDAllocator;

	// Pre ClearDepthStencilView등 사전에 뭘가 그려야할때 이팩트등?
	ComPtr<ID3D12CommandAllocator>					mPreCMDAllocator;
	ComPtr<ID3D12GraphicsCommandList>				mPreCMDList;

	// Post 스카이박스나 UI 그린다
	ComPtr<ID3D12CommandAllocator>					mPostCMDAllocator;
	ComPtr<ID3D12GraphicsCommandList>				mPostCMDList;

	UINT64											mFenceValue;

	/* 
	개수가 많은 상수버퍼는 그냥 메인으로 하나만 쓰고 UI나 스카이박스 같이 후에 하는것들중에 개수가 많지 않은 상수버퍼는 프레임마다 상수버퍼를 만들어서 사용 해도 된다?
	멀티 쓰레드에서 많은 수의 오브젝트의 상수 버퍼를 프레임 마다 가지고 있을수는 있지만 메모리를 너무 차지 할거 같고 비효율 적인거같아서?
	우선 지금은 상수버퍼는 프레임 마다 안쓰겟다 나중에 디퍼드 렌더링대나 써봐야겟다
	D3D12_CPU_DESCRIPTOR_HANDLE						mMultithreadConstantsCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE						mMultithreadConstantsGPU;
	D3D12_CPU_DESCRIPTOR_HANDLE						mMultithreadSampleCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE						mMultithreadSampleGPU;

	D3D12_CPU_DESCRIPTOR_HANDLE						mPreConstantsCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE						mPreConstantsGPU;
	D3D12_CPU_DESCRIPTOR_HANDLE						mPreSampleCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE						mPreSampleGPU;

	D3D12_CPU_DESCRIPTOR_HANDLE						mPostConstantsCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE						mPostConstantsGPU;
	D3D12_CPU_DESCRIPTOR_HANDLE						mPostSampleCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE						mPostSampleGPU;
	*/
};

class EngineMain
{
public:
	~EngineMain()
	{
		mpConstantBuffer->Unmap(0, nullptr);
		mpConstantBuffer->Release();
		mpConstantBuffer.Detach();
		mDsvTexture->Release();
		mDsvTexture.Detach();

		for (int iIndex = 0; iIndex < mvRenderTargets.size(); iIndex++)
		{
			mvRenderTargets[iIndex]->Release();
			mvRenderTargets[iIndex].Detach();
		}
	}

public:
	HINSTANCE hInstance;
	HINSTANCE hPrevInstance;
	LPSTR lpCmdLin;
	int nCmdShow;
	HWND hWndPlatformWindows;

	UINT width;
	UINT height;
	float aspectRatio;
	HWND hwnd;

	TimerClass mTimerClass;
	Camera* mCamera;
	BYTE* mPCBVDataBegin;
	std::vector<JMesh *> mObj;
	std::unordered_map<std::string, std::unique_ptr<JTexture>> mTextures;

	int				iNumberOfProcessors = 4;

public:
	CD3DX12_VIEWPORT mViewport;
	CD3DX12_RECT mScissorRect;
	ComPtr<ID3D12Fence> mFence;
	HANDLE mFenceEvent;
	void WaitForPreviousFrame();

public:
	//ComPtr<ID3DBlob>						mpVertexShader;
	//ComPtr<ID3DBlob>						mpPixelShader;
	//ComPtr<ID3D12PipelineState>				mpPipelineState;
	//ComPtr<ID3D12RootSignature>				mpRootSignature;

public:
	ComPtr<ID3D12Resource>					mpConstantBuffer;

	DescriptorHeapObject					mCBVDescriptorHeap;
	//ComPtr<ID3D12DescriptorHeap>			mpCBVHeap;

	DescriptorHeapObject					mSampleDescriptorHeap;
	//ComPtr<ID3D12DescriptorHeap>			mpSampleHeap;

	DescriptorHeapObject					mDsvDescriptorHeap;
	//ComPtr<ID3D12DescriptorHeap>			mpDsvHeap;

public:
	RenderPassResource						mRenderPass[eRenderPass_Max];

public:
	ComPtr<IDXGIFactory4>					mpFactory;
	ComPtr<ID3D12Device>					mpDevice;
	ComPtr<ID3D12CommandQueue>				mpCommandQueue;

	FrameResource							mFrameResource[iFrameCount];
	UINT64									mCurrentFrameIndex = 0;

	ComPtr<IDXGISwapChain3>					mpSwapChain;
	DescriptorHeapObject					mRtvDescriptorHeap;
	//ComPtr<ID3D12DescriptorHeap>			mpRtvHeap;
	std::vector<ComPtr<ID3D12Resource>>		mvRenderTargets;
	UINT									miRtvDescriptorSize;

	ComPtr<ID3D12Resource>					mDsvTexture;

	UINT									miCurrentmiBackBufferIndex;

public:
	void Init(HINSTANCE _hInstance, HINSTANCE _hPrevInstance, LPSTR _lpCmdLine, int _nCmdShow);
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	void WindowsCallback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void InitPlatformWindows(UINT windowWidth = 480, UINT windowHeight = 640);
	void InitModules();

	void OnUpdate();
	void OnRender();
	void AddBuffe();
};

