#include "DXAssetsPipelineState.h"

DXAssetsPipelineState::DXAssetsPipelineState(std::string strAssetname, ComPtr<ID3D12RootSignature> pRootSignature, DXAssetsShader& rAssetsShader) :
	DXAssets(strAssetname, ePipelineState)
{
	CreatePipelineState(pRootSignature, rAssetsShader);
}

void DXAssetsPipelineState::CreatePipelineState(ComPtr<ID3D12RootSignature> pRootSignature, DXAssetsShader& rAssetsShader)
{
	// Define the vertex input layout.
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	// Describe and create the graphics pipeline state object (PSO).
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
	psoDesc.pRootSignature = pRootSignature.Get();
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(rAssetsShader.GetVertexShader().Get());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(rAssetsShader.GetPixelShader().Get());
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = FALSE;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;
	
	ThrowIfFailed(DXPipelineManager::Get()->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mpPipelineState)));
}

ComPtr<ID3D12PipelineState> DXAssetsPipelineState::GetPipelineState()
{
	return mpPipelineState;
}
