#pragma once
#include "stdafx.h"

class DescriptorHeapObject
{
public:
	DescriptorHeapObject() { memset(this, 0, sizeof(*this)); }

	HRESULT Create( ID3D12Device* pDevice, D3D12_DESCRIPTOR_HEAP_TYPE Type, UINT NumDescriptors, bool bShaderVisible = false)
	{
		mMaxSize = NumDescriptors;
		mCurrentSize = 0;

		Desc.Type = Type;
		Desc.NumDescriptors = mMaxSize;
		Desc.Flags = (bShaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : (D3D12_DESCRIPTOR_HEAP_FLAGS)0);

		HRESULT hr = pDevice->CreateDescriptorHeap(&Desc,
			__uuidof(ID3D12DescriptorHeap),
			(void**)&pDH);
		if (FAILED(hr)) return hr;

		hCPUHeapStart = pDH->GetCPUDescriptorHandleForHeapStart();
		if (bShaderVisible)
		{
			hGPUHeapStart = pDH->GetGPUDescriptorHandleForHeapStart();
		}
		else
		{
			hGPUHeapStart.ptr = 0;
		}
		mHandleIncrementSize = pDevice->GetDescriptorHandleIncrementSize(Desc.Type);
		return hr;
	}
	operator ID3D12DescriptorHeap*() { return pDH.Get(); }

	SIZE_T MakeOffsettedCPU(SIZE_T ptr, UINT index)
	{
		SIZE_T offsetted;
		offsetted = ptr + static_cast<SIZE_T>(index * mHandleIncrementSize);
		return offsetted;
	}

	UINT64 MakeOffsettedGPU(UINT64 ptr, UINT index)
	{
		UINT64 offsetted;
		offsetted = ptr + static_cast<UINT64>(index * mHandleIncrementSize);
		return offsetted;
	}


	// 자원을 생성할때 쓰인다
	D3D12_CPU_DESCRIPTOR_HANDLE hCPU(UINT index)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE handle;
		handle.ptr = MakeOffsettedCPU(hCPUHeapStart.ptr, index);
		return handle;
	}
	// 자원을 GPU에 업로드할때 쓰인다
	D3D12_GPU_DESCRIPTOR_HANDLE hGPU(UINT index)
	{
		assert(Desc.Flags&D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
		D3D12_GPU_DESCRIPTOR_HANDLE handle;
		handle.ptr = MakeOffsettedGPU(hGPUHeapStart.ptr, index);
		return handle;
	}

	D3D12_GPU_DESCRIPTOR_HANDLE GPUEnd() { return hGPU(mCurrentSize); }
	INT HandleIncrementSize() { return mHandleIncrementSize; }

	D3D12_CPU_DESCRIPTOR_HANDLE Append()
	{
		assert(mCurrentSize < mMaxSize);
		return hCPU(mCurrentSize++);
	}

	D3D12_GPU_DESCRIPTOR_HANDLE GetCurrentGPU()
	{
		UINT iIndex = GetCurrentIndex() - 1;
		iIndex = iIndex < 0 ? 0 : iIndex;
		return hGPU(iIndex);
	}

	D3D12_GPU_DESCRIPTOR_HANDLE GetCurrentGPU(UINT iIndex)
	{
		iIndex = iIndex < 0 ? 0 : iIndex;
		return hGPU(iIndex);
	}

	UINT GetCurrentIndex() { return mCurrentSize; };

	// Invalidates contents of any previous handles
	void Clear() { mCurrentSize = 0; }
	void Resize(UINT newSize) { mCurrentSize = newSize; }
	UINT Size() const { return mCurrentSize; }

	D3D12_DESCRIPTOR_HEAP_DESC Desc;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> pDH;
	D3D12_CPU_DESCRIPTOR_HANDLE hCPUHeapStart;
	D3D12_GPU_DESCRIPTOR_HANDLE hGPUHeapStart;
	UINT mHandleIncrementSize;
	UINT mCurrentSize = 0;
	UINT mMaxSize = 0;
};

