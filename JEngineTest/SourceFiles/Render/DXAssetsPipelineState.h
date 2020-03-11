#pragma once
#include "../../stdafx.h"
#include "DXAssets.h"
#include "DXAssetsShader.h"


class DXAssetsPipelineState : public DXAssets
{
private:
	DXAssetsPipelineState() {};

public:
	DXAssetsPipelineState(std::string strAssetname, ComPtr<ID3D12RootSignature> pRootSignature, DXAssetsShader& rAssetsShader);

private:
	ComPtr<ID3D12PipelineState> mpPipelineState;
	void CreatePipelineState(ComPtr<ID3D12RootSignature> pRootSignature, DXAssetsShader& rAssetsShader);

public:
	ComPtr<ID3D12PipelineState> GetPipelineState();
};