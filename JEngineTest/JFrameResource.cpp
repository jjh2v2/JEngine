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

#include "stdafx.h"
#include "JFrameResource.h"

JFrameResource::JFrameResource(ID3D12Device* pDevice, ID3D12DescriptorHeap* pDsvHeap, ID3D12DescriptorHeap* pCbvSrvHeap, D3D12_VIEWPORT* pViewport, UINT frameResourceIndex) :
    m_fenceValue(0)
{
    for (UINT i = 0; i < CommandListCount; i++)
    {
        ThrowIfFailed(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[i])));
        ThrowIfFailed(pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[i].Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandLists[i])));

        // Close these command lists; don't record into them for now.
        ThrowIfFailed(m_commandLists[i]->Close());
    }

	for (UINT i = 0; i < NumContexts; i++)
	{
		// Create command list allocators for worker threads. One alloc is 
		// for the shadow pass command list, and one is for the scene pass.
		ThrowIfFailed(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_sceneCommandAllocators[i])));

		ThrowIfFailed(pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_sceneCommandAllocators[i].Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_sceneCommandLists[i])));

		// Close these command lists; don't record into them for now. We will 
		// reset them to a recording state when we start the render loop.
		ThrowIfFailed(m_sceneCommandLists[i]->Close());
	}
}

JFrameResource::~JFrameResource()
{
    for (int i = 0; i < CommandListCount; i++)
    {
        m_commandAllocators[i] = nullptr;
        m_commandLists[i] = nullptr;
    }

    m_shadowConstantBuffer = nullptr;
    m_sceneConstantBuffer = nullptr;

    for (int i = 0; i < NumContexts; i++)
    {
        //m_shadowCommandLists[i] = nullptr;
        //m_shadowCommandAllocators[i] = nullptr;

        m_sceneCommandLists[i] = nullptr;
        m_sceneCommandAllocators[i] = nullptr;
    }

    m_shadowTexture = nullptr;
}
