#pragma once
#include "stdafx.h"

class DXDevice
{
public:
	DEVICE_VENDOR_ID mVendorId;
	DXGI_ADAPTER_DESC mAdapterDesc;
	MS::ComPtr<ID3D12Device> mDevice;
	MS::ComPtr<ID3D12CommandQueue> mCommandQueue;
	MS::ComPtr<ID3D12CommandAllocator> mCommandAllocator;
	std::array<MS::ComPtr<ID3D12Resource>, NumRenderTargets> mRenderTargets;

	MS::ComPtr<ID3D12Fence> mFence;
	std::array<UINT64, NumRenderTargets> mFenceValues;
	UINT64 mCurrentFenceValue;
	HANDLE mFenceEvent;

	MS::ComPtr<ID3D12DescriptorHeap> mRtvHeap;
	UINT mRtvDescriptorSize;
	MS::ComPtr<ID3D12DescriptorHeap> mCbvSrvUavHeap;
	UINT mCbvSrvUavDescriptorSize;

	MS::ComPtr<IDXGIFactory4> mDXGIFactory;
	MS::ComPtr<IDXGISwapChain3> mSwapChain;

	DXDevice(IDXGIAdapter* adapter, D3D_FEATURE_LEVEL MaxFeatureLevel);
	~DXDevice();
	void CreateDescriptorHeaps(D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc, D3D12_DESCRIPTOR_HEAP_DESC cbvSrvUavHeapDesc);
	void CreateSwapChain(DXDevice* dxDevice, DXGI_SWAP_CHAIN_DESC1 swapChainDesc);
};
