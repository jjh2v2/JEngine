#pragma once
#include "../../stdafx.h"
#include "DXAssets.h"


class DXAssetsConstantBuffer : public DXAssets
{
private:
	DXAssetsConstantBuffer() {};

public:
	DXAssetsConstantBuffer(std::string strAssetname);

private:
	ComPtr<ID3D12DescriptorHeap>	mpCBVHeap;
	ComPtr<ID3D12Resource>			mpConstantBuffer;
	void							CreateConstantBuffer();

public:
	void							CreateConstantBufferss(size_t iAddCount = 1);
	ComPtr<ID3D12DescriptorHeap>	GetCBVHeap();
	ComPtr<ID3D12Resource>			GetCBVBuffer();

	static UINT CalcConstantBufferByteSize(UINT byteSize)
	{
		// Constant buffers must be a multiple of the minimum hardware
		// allocation size (usually 256 bytes).  So round up to nearest
		// multiple of 256.  We do this by adding 255 and then masking off
		// the lower 2 bytes which store all bits < 256.
		// Example: Suppose byteSize = 300.
		// (300 + 255) & ~255
		// 555 & ~255
		// 0x022B & ~0x00ff
		// 0x022B & 0xff00
		// 0x0200
		// 512
		return (byteSize + 255) & ~255;
	}
	
};