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

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently.

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN            // Exclude rarely-used stuff from Windows headers.
#endif

#include <windows.h>
#include <string>
#include <wrl.h>
#include <process.h>
#include <shellapi.h>

// file
#include <fstream>

// Windows includes
#include <Windows.h>
#include <wrl.h>
#include <wrl/client.h>
#include <winnt.h>
#include <thread>
#include <winerror.h>

// C RunTime Header Files includes
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>

// STL includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <conio.h>
#include <vector>
#include <array>
#include <conio.h>


// DX12 includes
#include <d3d12.h>
#include <dxgi1_4.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include "d3dx12.h"


#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"Dxgi.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"d3dcompiler.lib")


// Input includes
#include <Xinput.h>
#pragma comment(lib,"xinput.lib")

// Sound include
#include <xaudio2.h>
#pragma comment(lib,"xaudio2.lib")

#define SINGLETHREADED_ FALSE

static const UINT FrameCount = 3;

static const UINT NumContexts = 3;
static const UINT NumLights = 3;        // Keep this in sync with "shaders.hlsl".

static const UINT TitleThrottle = 200;    // Only update the titlebar every X number of frames.

// Command list submissions from main thread.
static const int CommandListCount = 3;
static const int CommandListPre = 0;
static const int CommandListMid = 1;
static const int CommandListPost = 2;
