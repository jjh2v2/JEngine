#pragma once
#include "../../stdafx.h"
#include "DXAssetsShader.h"
#include "DXAssetsRootSignature.h"
#include "DXAssetsPipelineState.h"
#include "DXAssetsConstantBuffer.h"

class DXAssetsManager
{
private:
	DXAssetsManager() { };
public:
	static DXAssetsManager* Get() {
		static DXAssetsManager* instance = new DXAssetsManager();
		return instance;
	};

	void Init();

private:
	std::unordered_map<std::string, DXAssetsShader>				mMapAssetsShader;
	std::unordered_map<std::string, DXAssetsRootSignature>		mMapAssetsRootSignature;
	std::unordered_map<std::string, DXAssetsPipelineState>		mMapAssetsPipelineState;
	std::unordered_map<std::string, DXAssetsConstantBuffer>		mMapAssetsConstantBuffer;

	void CreateAssetsShader(std::string sKey, std::wstring sFilePath);
	void CreateAssetsRootSignature(std::string sKey);
	void CreateAssetsPipelineState(std::string sKey, ComPtr<ID3D12RootSignature> pRootSignature, DXAssetsShader& rAssetsShader);
	void CreateAssetsConstantBuffer(std::string sKey);

public:
	DXAssetsShader&				GetAssetsShader(std::string sKey);
	DXAssetsRootSignature&		GetAssetsRootSignature(std::string sKey);
	DXAssetsPipelineState&		GetAssetsPipelineState(std::string sKey);
	DXAssetsConstantBuffer&		GetAssetsConstantBuffer(std::string sKey);
};