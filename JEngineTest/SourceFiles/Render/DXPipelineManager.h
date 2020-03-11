#pragma once
#include "../../stdafx.h"

class DXPipelineManager
{
private:
	DXPipelineManager() { };
public:
	static DXPipelineManager* Get() {
		static DXPipelineManager* instance = new DXPipelineManager();
		return instance;
	};

	void Init(UINT& rFrameIndex, UINT iFrameCount = 2);

private:
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

	void									CreateHardwareAdapter(IDXGIAdapter1** ppAdapter);
	void									CreateDevice();
	void									CreateCommandQueue();
	void									CreateCommandAllocator();
	void									CreateCommandList();

	void									CreateSwapChain(UINT& rFrameIndex, UINT iFrameCount = 2);
	void									CreateRenderTarget(UINT iFrameCount = 2);

	void									CreateFence();

public:
	ComPtr<ID3D12Device>					GetDevice() { return mpDevice; };
	ComPtr<ID3D12CommandQueue>				GetCommandQueue() { return mpCommandQueue; };
	ComPtr<ID3D12CommandAllocator>			GetCommandAllocator() { return mpCommandAllocator; };
	ComPtr<ID3D12GraphicsCommandList>		GetCommandList() { return mpCommandList; };
	
	ComPtr<IDXGISwapChain3>					GetSwapChain() { return mpSwapChain; };
	ComPtr<ID3D12DescriptorHeap>			GetRtvHeap() { return mpRtvHeap; };
	std::vector<ComPtr<ID3D12Resource>>&	GetRenderTargets() { return mvRenderTargets; };
	UINT									GetRtvDescriptorSize() { return miRtvDescriptorSize; };
};