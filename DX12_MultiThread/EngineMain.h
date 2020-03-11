#pragma once
#include "stdafx.h"
#include "Mesh.h"
#include "Camera.h"
#include "d3dUtil.h"
#include "DirectXTK12/ResourceUploadBatch.h"

const UINT iSwapChainBufferCount = 3;
const UINT iFrameCount = 3;

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
	command list�� �߰��� ��ɵ��� �� Allocator�� �޸𸮿� ������ �ȴ�.
	
	ID3D12GraphicsCommandList
	�׸��� ��ɵ��� ��� ����Ǵ°��� �ƴ϶� ��ɵ��� command list�� �߰��� �Ѵ�.
	(ExecuteCommandLists�� �ٷ� �����ϴ°��� �ƴ϶� queue�� �߰��� �ϴ°��̴�.)
	GPU�� queue�� ���� ��ɵ��� �̾Ƽ� �����Ѵ�.
	��ɵ��� command list�� ��� �߰� �� ��
	���� ExecuteCommandLists�� ���� �ϱ� ���� ID3D12GraphicsCommandList::Close�� ȣ���ؼ�
	Direct3D���� ��� ����� �����ٰ� �˷��־�� �Ѵ�.
	���� ��ɵ��� �߰��ϴ� ��� ����� ������ ��� ��� ����� Close�� �� �����־�� �ϰ�
	command list�� �����ϰų� �缳���ϰ� �Ǹ� command list�� �����ִ� ���°� �Ǿ������� �����ؾ� �Ѵ�.
	�׸��� ID3D12CommandList::Reset �޼����°��� �ִµ�
	�̰��� ������ �޸𸮵��� ���ο� ��ɵ��� ����ϴµ� ���� �� �� �ְ� �Ѵ�.
	command list�� ���� ó�� �������� ���� ���� ���·� �������� command list�� �ִ� ��ɵ鿡�Դ� ������ ��ġ�� �ʴ´�.
	�׸��� ID3D12CommandAllocator::Reset�޼���� ��� �Ҵ����� �޸𸮸� ���� �������� ���ؼ� ���� �� �� �ְ� ���ش�.
	���� Reset���� delete�� �� ������ new�� �ٽ� �޸� ������ ��°��� �ƴ�
	���θ� ó��ó�� ����شٴ� ������ �ϸ� �ɰ� ����.
	(std::vector::clear ó��...)
	���������� GPU�� ��� �Ҵ��ڿ� ��� ��� ����� ���� ������ Ȯ�������� �������� Reset���� �缳�� ���� ���ƾ� �Ѵٴ���.
	*/

	// ��Ƽ�������
	std::vector<ID3D12GraphicsCommandList*>			mVecMultithreadCMDList;
	std::vector<ID3D12CommandAllocator*>			mVecMultithreadCMDAllocator;

	// Pre ClearDepthStencilView�� ������ ���� �׷����Ҷ� ����Ʈ��?
	ComPtr<ID3D12CommandAllocator>					mPreCMDAllocator;
	ComPtr<ID3D12GraphicsCommandList>				mPreCMDList;

	// Post ��ī�̹ڽ��� UI �׸���
	ComPtr<ID3D12CommandAllocator>					mPostCMDAllocator;
	ComPtr<ID3D12GraphicsCommandList>				mPostCMDList;

	UINT64											mFenceValue;

	/* 
	������ ���� ������۴� �׳� �������� �ϳ��� ���� UI�� ��ī�̹ڽ� ���� �Ŀ� �ϴ°͵��߿� ������ ���� ���� ������۴� �����Ӹ��� ������۸� ���� ��� �ص� �ȴ�?
	��Ƽ �����忡�� ���� ���� ������Ʈ�� ��� ���۸� ������ ���� ������ �������� ������ �޸𸮸� �ʹ� ���� �Ұ� ���� ��ȿ�� ���ΰŰ��Ƽ�?
	�켱 ������ ������۴� ������ ���� �Ⱦ��ٴ� ���߿� ���۵� �������볪 ����߰ٴ�
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

public:
	void Init(ID3D12Device* pDevice)
	{
		{// FenceValue
			mFenceValue = 0;
		}

		{// Pre
			ThrowIfFailed(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mPreCMDAllocator)));
			ThrowIfFailed(pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mPreCMDAllocator.Get(), nullptr, IID_PPV_ARGS(&mPreCMDList)));
		}

		{// Post
			ThrowIfFailed(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mPostCMDAllocator)));
			ThrowIfFailed(pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mPostCMDAllocator.Get(), nullptr, IID_PPV_ARGS(&mPostCMDList)));
		}

		{// Multithread

			for (int iCount=0; iCount<4; iCount++)
			{
				ComPtr<ID3D12CommandAllocator> pCMDAllocator = nullptr;
				ComPtr<ID3D12GraphicsCommandList> pCMDList = nullptr;
				ThrowIfFailed(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&pCMDAllocator)));
				ThrowIfFailed(pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, pCMDAllocator.Get(), nullptr, IID_PPV_ARGS(&pCMDList)));
				//mVecMultithreadCMDAllocator.push_back(pCMDAllocator.Get());
				//mVecMultithreadCMDList.push_back(pCMDList.Get());
			}

		}

		/*{
			// Create some new resources for initialization
			ID3D12GraphicsCommandList* cmdLst = nullptr;
			ID3D12CommandAllocator* cmdAlloc = nullptr;
			ThrowIfFailed(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdAlloc)));
			ThrowIfFailed(pDevice->CreateCommandList(1, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdAlloc, nullptr, IID_PPV_ARGS(&cmdLst)));
		}*/
	};
};

class EngineMain
{
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

	Camera* mCamera;
	BYTE* mPCBVDataBegin;
	std::vector<Mesh *> mObj;
	std::unordered_map<std::string, std::unique_ptr<Texture>> mTextures;
public:
	CD3DX12_VIEWPORT mViewport;
	CD3DX12_RECT mScissorRect;
	ComPtr<ID3D12Fence> mFence;
	UINT64 mFenceValue;
	HANDLE mFenceEvent;
	void WaitForPreviousFrame();

public:
	ComPtr<ID3DBlob>						mpVertexShader;
	ComPtr<ID3DBlob>						mpPixelShader;
	ComPtr<ID3D12PipelineState>				mpPipelineState;

public:
	ComPtr<ID3D12DescriptorHeap>			mSampleHeap;

public:
	ComPtr<ID3D12DescriptorHeap>			mpCBVHeap;
	ComPtr<ID3D12Resource>					mpConstantBuffer;

public:
	ComPtr<IDXGIFactory4>					mpFactory;
	ComPtr<ID3D12Device>					mpDevice;
	ComPtr<ID3D12CommandQueue>				mpCommandQueue;

	//ComPtr<ID3D12CommandAllocator>			mpCommandAllocator;//command list�� �߰��� ��ɵ��� �� Allocator�� �޸𸮿� ������ �ȴ�.
	//ComPtr<ID3D12GraphicsCommandList>		mpCommandList;

	FrameResource							mFrameResource[iFrameCount];

	ComPtr<IDXGISwapChain3>					mpSwapChain;
	ComPtr<ID3D12DescriptorHeap>			mpRtvHeap;
	std::vector<ComPtr<ID3D12Resource>>		mvRenderTargets;
	UINT									miRtvDescriptorSize;

	ComPtr<ID3D12RootSignature>				mpRootSignature;

	UINT									miCurrentFrameIndex;

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

