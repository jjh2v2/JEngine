#include "stdafx.h"
#include "GraphicsDeviceIne_DX12.h"


GraphicsDeviceIne_DX12::GraphicsDeviceIne_DX12()
{
}


GraphicsDeviceIne_DX12::~GraphicsDeviceIne_DX12()
{
}

void GraphicsDeviceIne_DX12::Init()
{
	UINT dxgiFactoryFlags = 0;
#ifdef DEBUG
	// Enable the D3D12 debug layer - must be done before devices are made
	{
		ID3D12Debug* debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
			debugController->Release();
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif

	Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;
	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&dxgiFactory)));
	
	/*
	// Enumerate the primary adapter
	MS::ComPtr<IDXGIAdapter> primaryAdapter;//IDXGIAdapter* primaryAdapter;
	DXGI_ADAPTER_DESC primaryAdapterDesc;
	ThrowIfFailed(dxgiFactory->EnumAdapters(Device_Primary, &primaryAdapter)); // Get primaryDevice adapter ptr
	primaryAdapter->GetDesc(&primaryAdapterDesc);

	// Enumerate the secondary adapter - if only one GPU defaults to WARP
	IDXGIAdapter* secondaryAdapter;
	DXGI_ADAPTER_DESC secondaryAdapterDesc;
	HRESULT res = dxgiFactory->EnumAdapters(Device_Secondary, &secondaryAdapter); // Get secondaryDevice adapter ptr
	secondaryAdapter->GetDesc(&secondaryAdapterDesc);
	*/

	MS::ComPtr<IDXGIAdapter> pAdapters;
	//IDXGIAdapter* pAdapters = nullptr;
	for (UINT AdapterIndex = 0; dxgiFactory->EnumAdapters(AdapterIndex, &pAdapters) != DXGI_ERROR_NOT_FOUND; ++AdapterIndex)
	{
		if (pAdapters)
		{
			DXGI_ADAPTER_DESC AdapterDesc;
			pAdapters->GetDesc(&AdapterDesc);
			bool bIsAMD = AdapterDesc.VendorId == 0x1002;
			bool bIsIntel = AdapterDesc.VendorId == 0x8086;
			bool bIsNVIDIA = AdapterDesc.VendorId == 0x10DE;
			bool bIsWARP = AdapterDesc.VendorId == 0x1414;
			if (bIsWARP) continue;

			D3D_FEATURE_LEVEL MaxSupportedFeatureLevel = static_cast<D3D_FEATURE_LEVEL>(0);
			UINT NumNodes = 0;
			const D3D_FEATURE_LEVEL FeatureLevels[] =
			{
				// Add new feature levels that the app supports here.
				D3D_FEATURE_LEVEL_12_1,
				D3D_FEATURE_LEVEL_12_0,
				D3D_FEATURE_LEVEL_11_1,
				D3D_FEATURE_LEVEL_11_0
			};
			ID3D12Device* pDevice = nullptr;

			//장치가 사용하는 최소 기능 단계입니다
			//만약 D3D_FEATURE_LEVEL_11_0을 전달할 경우 11.0부터 12.1까지 지원하는 가장 높은 단계의 기능을 사용합니다
			D3D_FEATURE_LEVEL MinFeatureLevel = D3D_FEATURE_LEVEL_11_0;
			
			HRESULT HR = D3D12CreateDevice(*(&pAdapters), MinFeatureLevel, IID_PPV_ARGS(&pDevice));
			if (HR == S_OK)
			{
				// Determine the max feature level supported by the driver and hardware.
				D3D_FEATURE_LEVEL MaxFeatureLevel = MinFeatureLevel;
				D3D12_FEATURE_DATA_FEATURE_LEVELS FeatureLevelCaps = {};
				FeatureLevelCaps.pFeatureLevelsRequested = FeatureLevels;
				FeatureLevelCaps.NumFeatureLevels = _countof(FeatureLevels);
				if (SUCCEEDED(pDevice->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &FeatureLevelCaps, sizeof(FeatureLevelCaps))))
				{
					MaxFeatureLevel = FeatureLevelCaps.MaxSupportedFeatureLevel;
				}

				UINT OutNumDeviceNodes = pDevice->GetNodeCount();
				pDevice->Release();

				D3D_FEATURE_LEVEL OutMaxFeatureLevel = MinFeatureLevel;
				DXDevice* dsf = new DXDevice(*(&pAdapters), OutMaxFeatureLevel);
				devices.push_back(dsf);
				int i = 0;
			}
		}
	}

	// Describe descriptor heaps - RTV and CBV/SRV/UAV

	// Describe and create the swap chain.

	// Create descriptor heaps.

	// Create frame resources.
}
