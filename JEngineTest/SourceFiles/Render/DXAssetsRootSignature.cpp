#include "DXAssetsRootSignature.h"

DXAssetsRootSignature::DXAssetsRootSignature(std::string strAssetname, HLSLParser& rHLSLParser) :
	DXAssets(strAssetname, eRootSignature)
{
	CreateRootSignature(rHLSLParser);
}

void DXAssetsRootSignature::CreateRootSignature(HLSLParser& rHLSLParser)
{
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

	// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

	ComPtr<ID3D12Device> mDevice = DXPipelineManager::Get()->GetDevice();
	if (FAILED(mDevice->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
	{
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}

	/////////
	size_t iCbufferNum = rHLSLParser.mHLSLTokenCount[HLSLToken_Cbuffer];
	size_t iTextureNum = rHLSLParser.mHLSLTokenCount[HLSLToken_Texture2D];
	size_t iSamplerNum = rHLSLParser.mHLSLTokenCount[HLSLToken_SamplerState];
	size_t iDescriptorNum = iCbufferNum + iTextureNum;

	std::vector<CD3DX12_DESCRIPTOR_RANGE1> vRange;
	// 텍스쳐
	for (int i = 0; i < iTextureNum; i++)
	{
		// i는 Texture2D DiffuseTexture : register(t0); t0 t1 t2에 해당한다 iBaseShaderRegister
		CD3DX12_DESCRIPTOR_RANGE1 range;
		range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, i, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
		vRange.push_back(range);
	}
	// 상수버퍼
	for (int i=0; i< iCbufferNum; i++)
	{
		// i는 Texture2D DiffuseTexture : register(t0); t0 t1 t2에 해당한다 iBaseShaderRegister
		CD3DX12_DESCRIPTOR_RANGE1 range;
		range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, i, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
		vRange.push_back(range);
	}

	/*
	InitAsConstantBufferView 이거 쓰면 SetGraphicsRootConstantBufferView이걸 써야 하는거같은데
	InitAsDescriptorTable이걸로해서 쓰는 방법으로 해보자 기타 따른 프로젝트들은 InitAsDescriptorTable이걸 쓰는거 같아서
	물론 업데이트를 받아서 CD3DX12_DESCRIPTOR_RANGE1 이거때문인거 같지만
	https://docs.microsoft.com/ko-kr/windows/win32/direct3d12/root-signature-version-1-1
	*/
	/////////

	UINT iSRVRegister = 0;// Texture2D DiffuseTexture : register(t0);
	CD3DX12_DESCRIPTOR_RANGE1 ranges[3];
	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, iSRVRegister, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
	ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, iSRVRegister+1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
	ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
	//range[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0); 4은 t0 ~ t3 까지 4개가 있다는걸 알리는거?
	//range[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0); 1개씩 따로 설정 가능 하다
	//range[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	//range[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 16, 0);
	//range[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);

	CD3DX12_ROOT_PARAMETER1 rootParameters[3];
	rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[2].InitAsConstantBufferView(0);
	//rootParameters[2].InitAsDescriptorTable(1, &ranges[2], D3D12_SHADER_VISIBILITY_ALL);

	const CD3DX12_STATIC_SAMPLER_DESC pointWrap[2] = {
		{0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP },

		{1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP} // addressW
	}; // addressW

	/*
	struct Transform
	{
		matrix World;
		matrix WorldInverseTranspose;
	};
	StructuredBuffer<Transform> g_TransformBuffer : register(t0);
	rootParameters[1].InitAsShaderResourceView()
	*/

	// Allow input layout and deny uneccessary access to certain pipeline stages.
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	UINT sdfdf = sizeof(pointWrap) / sizeof(CD3DX12_STATIC_SAMPLER_DESC);
	UINT iRootParameters = _countof(rootParameters);
	rootSignatureDesc.Init_1_1(iRootParameters, rootParameters, sdfdf, pointWrap, rootSignatureFlags);

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
	ThrowIfFailed(mDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&mpRootSignature)));
}

ComPtr<ID3D12RootSignature>& DXAssetsRootSignature::GetRootSignature()
{
	return mpRootSignature;
}