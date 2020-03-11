#include "DXAssetsShader.h"

DXAssetsShader::DXAssetsShader(std::string sAssetsName, std::wstring sFileName) :
	DXAssets(sAssetsName, eShader),
	mcsFileName(sFileName)
{
	if (mcsFileName.compare(L"") == false)
		mcsFileName = L"shaders_0.hlsl";

	Load();
};

void DXAssetsShader::Load()
{
#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	HRESULT hr = S_FALSE;

	mbOnVertexShader = true;
	mbOnPixelShader = true;
	mbOnGeometryShader = true;
	mbOnHullShader = true;
	mbOnTessellatorShader = true;
	mbOnDomainShader = true;
	mbOnComputeShader = true;

	hr = D3DCompileFromFile(mcsFileName.c_str(), nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &mpVertexShader, nullptr);
	if (hr != S_OK)
	{
		mbOnVertexShader = false;
		//ThrowIfFailed(hr);
	}
	hr = D3DCompileFromFile(mcsFileName.c_str(), nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &mpPixelShader, nullptr);
	if (hr != S_OK)
	{
		mbOnPixelShader = false;
		//ThrowIfFailed(hr);
	}
	hr = D3DCompileFromFile(mcsFileName.c_str(), nullptr, nullptr, "GSMain", "gs_5_0", compileFlags, 0, &mpGeometryShader, nullptr);
	if (hr != S_OK)
	{
		mbOnGeometryShader = false;
		//ThrowIfFailed(hr);
	}
	hr = D3DCompileFromFile(mcsFileName.c_str(), nullptr, nullptr, "HSMain", "hs_5_0", compileFlags, 0, &mpHullShader, nullptr);
	if (hr != S_OK)
	{
		mbOnHullShader = false;
		//ThrowIfFailed(hr);
	}
	hr = D3DCompileFromFile(mcsFileName.c_str(), nullptr, nullptr, "TSMain", "ts_5_0", compileFlags, 0, &mpTessellatorShader, nullptr);
	if (hr != S_OK)
	{
		mbOnTessellatorShader = false;
		//ThrowIfFailed(hr);
	}
	hr = D3DCompileFromFile(mcsFileName.c_str(), nullptr, nullptr, "DSMain", "ds_5_0", compileFlags, 0, &mpDomainShader, nullptr);
	if (hr != S_OK)
	{
		mbOnDomainShader = false;
		//ThrowIfFailed(hr);
	}
	hr = D3DCompileFromFile(mcsFileName.c_str(), nullptr, nullptr, "CSMain", "cs_5_0", compileFlags, 0, &mpComputeShader, nullptr);
	if (hr != S_OK)
	{
		mbOnComputeShader = false;
		//ThrowIfFailed(hr);
	}


	mHLSLParser.Parse(mcsFileName);
	int iuer = 0;
	int dsfe = iuer;
}