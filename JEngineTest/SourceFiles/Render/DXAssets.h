#pragma once
#include "../../stdafx.h"
#include "DXPipelineManager.h"
#include "Buffer/ConstantBuffer.h"

enum eAssetsClass
{
	eNone = 0,
	eShader,
	eRootSignature,
	ePipelineState,
	eConstantBuffer
};

class DXAssets
{
public:
	DXAssets()
	{
		msAssetname = "";
		meAssetsClass = eNone;
	};

	DXAssets(std::string mStrAssetname, eAssetsClass eClass)
	{
		msAssetname = mStrAssetname;
		meAssetsClass = eClass;
	};
	std::string				msAssetname;
	eAssetsClass			meAssetsClass;
};

