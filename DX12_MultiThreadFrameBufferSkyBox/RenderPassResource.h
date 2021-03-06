#pragma once
#include "stdafx.h"
#include "d3dUtil.h"
#include "DirectXTK12/ResourceUploadBatch.h"
#include "DescriptorHeapObject.h"

// Enum
enum eRenderPass
{
	eRenderPass_Normal = 0,
	eRenderPass_Pre,
	eRenderPass_Sky,
	eRenderPass_Post,
	eRenderPass_Max
};

class RenderPassResource
{
public:
	eRenderPass								mePass;

	// Shader
	ComPtr<ID3DBlob>						mpVertexShader;
	ComPtr<ID3DBlob>						mpPixelShader;

	// RootSignature
	ComPtr<ID3D12RootSignature>				mpRootSignature;
	// PipelineState
	ComPtr<ID3D12PipelineState>				mpPipelineState;

	// ConstantBufferView
	BYTE*									mPCBVDataBegin;
	ComPtr<ID3D12Resource>					mpConstantBuffer;

	// DescriptorHeap
	DescriptorHeapObject					mCBVDescriptorHeap;
	DescriptorHeapObject					mSampleDescriptorHeap;

	ID3D12Device*							mpDevice;

public:
	RenderPassResource()
		: mePass(eRenderPass_Max)
	{

	}

	void Init(eRenderPass ePass, ID3D12Device* pDevice)
	{
		mePass = ePass;
		mpDevice = pDevice;

		switch (ePass)
		{
		case eRenderPass_Normal:
			InitNormal();
			break;
		case eRenderPass_Pre:
			InitPre();
			break;
		case eRenderPass_Sky:
			InitSky();
			break;
		case eRenderPass_Post:
			InitPost();
			break;
		case eRenderPass_Max:
			break;
		}
	};

	void CreateConstantBuffer()
	{
		// DescriptorHeap 생성시에 NumDescriptors를 크게 잡아서 하자
		UINT NumDescriptors = 131072;// 131072, 8192;//1,000,000+(디스크립트 개당 최대?) 디스크립트힙이 가질수 있는 총 디스크립트갯수 { 상수버퍼(CBV) + 이미지(SRV) + 정렬되지않은액세스뷰(UAV) }
		// CreateDescriptorHeap
		{
			// Describe and create a constant buffer view (CBV) descriptor heap.
			// Flags indicate that this descriptor heap can be bound to the pipeline 
			// and that descriptors contained in it can be referenced by a root table.
			/*D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
			cbvHeapDesc.NumDescriptors = NumDescriptors;
			cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			ThrowIfFailed(mpDevice->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&mpCBVHeap)));*/
			mCBVDescriptorHeap.Create(mpDevice, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, NumDescriptors, true);
		}

		// CreateConstantBufferView
		{
			UINT mElementByteSize = sizeof(SceneConstantBuffer);
			// (sizeof(SceneConstantBuffer) + 255) & ~255;    // CB size is required to be 256-byte aligned.
			mElementByteSize = (sizeof(SceneConstantBuffer) + 255) & ~255;

			ThrowIfFailed(mpDevice->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(mElementByteSize * NumDescriptors/*mObj.size()*/),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&mpConstantBuffer)));
		}
	}

	void InitNormal()
	{
		// Shader
		{
#if defined(_DEBUG)
			// Enable better shader debugging with the graphics debugging tools.
			UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
			UINT compileFlags = 0;
#endif
			std::wstring mcsFileName = L"C:/Users/jjh/Desktop/Project/JEngine/DX12_MultiThreadFrameBufferSkyBox/shaders_0.hlsl";
			HRESULT hr = S_FALSE;
			hr = D3DCompileFromFile(mcsFileName.c_str(), nullptr, nullptr, "VSMain", "vs_5_1", compileFlags, 0, &mpVertexShader, nullptr);
			if (hr != S_OK)
			{
			}
			hr = D3DCompileFromFile(mcsFileName.c_str(), nullptr, nullptr, "PSMain", "ps_5_1", compileFlags, 0, &mpPixelShader, nullptr);
			if (hr != S_OK)
			{
			}
		}

		CreateConstantBuffer();

		// CreateRootSignature
		{
			D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;// D3D_ROOT_SIGNATURE_VERSION_1_0

			UINT iSRVRegister = 0;// Texture2D DiffuseTexture : register(t0);
			CD3DX12_DESCRIPTOR_RANGE1 ranges[3];
			//ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, iSRVRegister, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
			//ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, iSRVRegister + 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
			ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
			ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, iSRVRegister /*+ 1*/, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
			ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE); // s0
			//range[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0); 4은 t0 ~ t3 까지 4개가 있다는걸 알리는거?
			//range[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0); 1개씩 따로 설정 가능 하다
			//range[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
			//range[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 16, 0);
			//range[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);

			CD3DX12_ROOT_PARAMETER1 rootParameters[3];
			//rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
			//rootParameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);
			rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_ALL);
			rootParameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);
			rootParameters[2].InitAsDescriptorTable(1, &ranges[2], D3D12_SHADER_VISIBILITY_PIXEL); // s0
			//rootParameters[0].InitAsConstantBufferView(0);

			// SAMPLER
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


			// Allow input layout and deny uneccessary access to certain pipeline stages.
			D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
				D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

			CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
			UINT sdfdf = sizeof(pointWrap) / sizeof(CD3DX12_STATIC_SAMPLER_DESC);
			UINT iRootParameters = _countof(rootParameters);
			//rootSignatureDesc.Init_1_1(iRootParameters, rootParameters, sdfdf, pointWrap, rootSignatureFlags);
			// 디스크립트힙에서 디스크립트테이블로 설정할거면 여기서 샘플러 넣으면 안된다
			rootSignatureDesc.Init_1_1(iRootParameters, rootParameters, 0, nullptr, rootSignatureFlags);

			ComPtr<ID3DBlob> signature;
			ComPtr<ID3DBlob> error;
			ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
			ThrowIfFailed(mpDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&mpRootSignature)));
		}

		// CreatePipelineState
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
			D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
			ZeroMemory(&psoDesc, sizeof(psoDesc));
			psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
			psoDesc.pRootSignature = mpRootSignature.Get();
			psoDesc.VS = CD3DX12_SHADER_BYTECODE(mpVertexShader.Get());
			psoDesc.PS = CD3DX12_SHADER_BYTECODE(mpPixelShader.Get());
			psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
			psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
			psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
			{// 알파 블랜드
				psoDesc.BlendState.AlphaToCoverageEnable = false;
				psoDesc.BlendState.IndependentBlendEnable = false;
				D3D12_RENDER_TARGET_BLEND_DESC BlendDesc;
				BlendDesc.BlendEnable = true;
				BlendDesc.LogicOpEnable = false;
				BlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
				BlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
				BlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
				BlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
				BlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
				BlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
				BlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
				BlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
				for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
					psoDesc.BlendState.RenderTarget[i] = BlendDesc;
			}
			psoDesc.SampleMask = UINT_MAX;
			psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			psoDesc.NumRenderTargets = 1;
			psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
			psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;// 이거 안했다고 콘솔에 워링뜬다 DSV와 같은 타입으로
			psoDesc.SampleDesc.Count = 1;

			ThrowIfFailed(mpDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mpPipelineState)));
		}
	};

	void InitPre()
	{
		
	};

	void InitSky()
	{
		// Shader
		{
#if defined(_DEBUG)
			// Enable better shader debugging with the graphics debugging tools.
			UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
			UINT compileFlags = 0;
#endif
			std::wstring mcsFileName = L"C:/Users/jjh/Desktop/Project/JEngine/DX12_MultiThreadFrameBufferSkyBox/skybox.hlsl";
			HRESULT hr = S_FALSE;
			hr = D3DCompileFromFile(mcsFileName.c_str(), nullptr, nullptr, "VSMain", "vs_5_1", compileFlags, 0, &mpVertexShader, nullptr);
			if (hr != S_OK)
			{
			}
			hr = D3DCompileFromFile(mcsFileName.c_str(), nullptr, nullptr, "PSMain", "ps_5_1", compileFlags, 0, &mpPixelShader, nullptr);
			if (hr != S_OK)
			{
			}
		}

		CreateConstantBuffer();

		// CreateRootSignature
		{
			D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;// D3D_ROOT_SIGNATURE_VERSION_1_0

			UINT iRegister = 0;// Texture2D DiffuseTexture : register(t0);
			CD3DX12_DESCRIPTOR_RANGE1 ranges[3];
			//ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, iSRVRegister, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
			//ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, iSRVRegister + 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
			ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, iRegister, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
			ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, iRegister /*+ 1*/, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
			ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, iRegister, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE); // s0
			//range[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0); 4은 t0 ~ t3 까지 4개가 있다는걸 알리는거?
			//range[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0); 1개씩 따로 설정 가능 하다
			//range[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
			//range[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 16, 0);
			//range[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);

			CD3DX12_ROOT_PARAMETER1 rootParameters[3];
			//rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
			//rootParameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);
			rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_ALL);
			rootParameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);
			rootParameters[2].InitAsDescriptorTable(1, &ranges[2], D3D12_SHADER_VISIBILITY_PIXEL); // s0
			//rootParameters[0].InitAsConstantBufferView(0);


			// Allow input layout and deny uneccessary access to certain pipeline stages.
			D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
				D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

			CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
			UINT iRootParameters = _countof(rootParameters);
			//rootSignatureDesc.Init_1_1(iRootParameters, rootParameters, sdfdf, pointWrap, rootSignatureFlags);
			// 디스크립트힙에서 디스크립트테이블로 설정할거면 여기서 샘플러 넣으면 안된다
			rootSignatureDesc.Init_1_1(iRootParameters, rootParameters, 0, nullptr, rootSignatureFlags);

			ComPtr<ID3DBlob> signature;
			ComPtr<ID3DBlob> error;
			ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
			ThrowIfFailed(mpDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&mpRootSignature)));

			/*ComPtr<ID3D12RootSignature> mpRootSignatureeeee;
			std::vector<uint8_t> VSBytecode = LoadFile("C:/Users/jjh/Desktop/Project/JEngine/DX12_MultiThreadFrameBufferSkyBox/skybox.hlsl", "rb");
			ThrowIfFailed(mpDevice->CreateRootSignature(0, VSBytecode.data(), VSBytecode.size(), IID_PPV_ARGS(&mpRootSignatureeeee)));
			int sdiewrwer = 0;
			int	intsdewr = sdiewrwer;*/
		}

		// CreatePipelineState
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
			D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
			ZeroMemory(&psoDesc, sizeof(psoDesc));
			psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
			psoDesc.pRootSignature = mpRootSignature.Get();
			psoDesc.VS = CD3DX12_SHADER_BYTECODE(mpVertexShader.Get());
			psoDesc.PS = CD3DX12_SHADER_BYTECODE(mpPixelShader.Get());
			psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
			psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
			psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
			{// 알파 블랜드
				psoDesc.BlendState.AlphaToCoverageEnable = false;
				psoDesc.BlendState.IndependentBlendEnable = false;
				D3D12_RENDER_TARGET_BLEND_DESC BlendDesc;
				BlendDesc.BlendEnable = true;
				BlendDesc.LogicOpEnable = false;
				BlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
				BlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
				BlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
				BlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
				BlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
				BlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
				BlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
				BlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
				for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
					psoDesc.BlendState.RenderTarget[i] = BlendDesc;
			}
			psoDesc.SampleDesc.Count = 1;
			psoDesc.SampleMask = UINT_MAX;
			psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			psoDesc.NumRenderTargets = 1;
			psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
			psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;// 이거 안했다고 콘솔에 워링뜬다 DSV와 같은 타입으로

			ThrowIfFailed(mpDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mpPipelineState)));
		}
	};

	void InitPost()
	{

	};

	std::vector<uint8_t> LoadFile(const char* Name, const char* Parm)
	{
		FILE* File = fopen(Name, Parm);
		//EA_ASSERT(File);
		fseek(File, 0, SEEK_END);
		long Size = ftell(File);
		if (Size <= 0)
		{
			//EA_ASSERT(0);
			return std::vector<uint8_t>();
		}
		std::vector<uint8_t> Content(Size);
		fseek(File, 0, SEEK_SET);
		fread(Content.data(), 1, Content.size(), File);
		fclose(File);
		return Content;
	}
};

