#pragma once
#include "../../stdafx.h"
#include "DXAssets.h"
#include "../HlslParser/HLSLParser.h"


class DXAssetsRootSignature : public DXAssets
{
private:
	DXAssetsRootSignature() {};

public:
	DXAssetsRootSignature(std::string strAssetname, HLSLParser& rHLSLParser);

private:
	ComPtr<ID3D12RootSignature> mpRootSignature;
	void CreateRootSignature(HLSLParser& rHLSLParser);

public:
	ComPtr<ID3D12RootSignature>& GetRootSignature();
};