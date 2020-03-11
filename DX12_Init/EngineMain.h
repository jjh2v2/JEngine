#pragma once
#include "stdafx.h"
#include "Mesh.h"
#include "Camera.h"

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

	UINT iFrameCount = 3;


	Camera* mCamera;
	BYTE* mPCBVDataBegin;
	std::vector<Mesh *> mObj;
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
	ComPtr<ID3D12DescriptorHeap>			mpCBVHeap;
	ComPtr<ID3D12Resource>					mpConstantBuffer;

public:
	ComPtr<IDXGIFactory4>					mpFactory;
	ComPtr<ID3D12Device>					mpDevice;
	ComPtr<ID3D12CommandQueue>				mpCommandQueue;
	ComPtr<ID3D12CommandAllocator>			mpCommandAllocator;
	ComPtr<ID3D12GraphicsCommandList>		mpCommandList;

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

	void Test();

	void InitPlatformWindows(UINT windowWidth = 480, UINT windowHeight = 640);
	void InitModules();

	void OnUpdate();
	void OnRender();
};

