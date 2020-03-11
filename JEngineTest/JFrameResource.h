//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#pragma once

#include "stdafx.h"
#include "Camera.h"
#include "SourceFiles/Render/Buffer/ConstantBuffer.h"

using namespace DirectX;
using namespace Microsoft::WRL;

const int CommandListCount = 2;//스왑체인 버퍼 갯수많큼? 랜터타켓 개수만큼?
const int NumContexts = 2;// 쓰레드 갯수만큼

class JFrameResource
{
public:
    ComPtr<ID3D12CommandAllocator> m_commandAllocators[CommandListCount];
    ComPtr<ID3D12GraphicsCommandList> m_commandLists[CommandListCount];

    ComPtr<ID3D12CommandAllocator> m_sceneCommandAllocators[NumContexts];
    ComPtr<ID3D12GraphicsCommandList> m_sceneCommandLists[NumContexts];

    UINT64 m_fenceValue;

private:
    ComPtr<ID3D12PipelineState> m_pipelineState;
    ComPtr<ID3D12PipelineState> m_pipelineStateShadowMap;
    ComPtr<ID3D12Resource> m_shadowTexture;
    D3D12_CPU_DESCRIPTOR_HANDLE m_shadowDepthView;
    ComPtr<ID3D12Resource> m_shadowConstantBuffer;
    ComPtr<ID3D12Resource> m_sceneConstantBuffer;
    SceneConstantBuffer* mp_shadowConstantBufferWO;        // WRITE-ONLY pointer to the shadow pass constant buffer.
    SceneConstantBuffer* mp_sceneConstantBufferWO;        // WRITE-ONLY pointer to the scene pass constant buffer.
    D3D12_GPU_DESCRIPTOR_HANDLE m_nullSrvHandle;    // Null SRV for out of bounds behavior.
    D3D12_GPU_DESCRIPTOR_HANDLE m_shadowDepthHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE m_shadowCbvHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE m_sceneCbvHandle;

public:
	JFrameResource(ID3D12Device* pDevice, ID3D12DescriptorHeap* pDsvHeap, ID3D12DescriptorHeap* pCbvSrvHeap, D3D12_VIEWPORT* pViewport, UINT frameResourceIndex);
    ~JFrameResource();
};
