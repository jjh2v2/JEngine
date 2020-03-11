#pragma once

// file
#include <fstream>

// Windows includes
#include <Windows.h>
#include <wrl.h>
#include <wrl/client.h>
#include <winnt.h>
#include <thread>
#include <winerror.h>
#include <algorithm>
#include <Windowsx.h>

// C RunTime Header Files includes
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>

// STL includes
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <conio.h>
#include <vector>
#include <array>
#include <conio.h>
#include <codecvt>
#include <locale>
#include <unordered_map>// std::map


// DX12 includes
#include <d3d12.h>
#include <dxgi1_4.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include "d3dx12.h"

// DX12 3rd party
#include "SourceFiles/Common/MathHelper.h"
#include <DirectXCollision.h>

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

extern const int gNumFrameResources;

namespace MS = Microsoft::WRL;
using namespace DirectX;
using Microsoft::WRL::ComPtr;

inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw;
	}
}

enum Devices
{
	Device_Primary,
	Device_Secondary,
	Device_Count
};

enum RenderProperties
{
	NumRenderTargets = 3
};

enum DEVICE_VENDOR_ID
{
	eNONE = 0,
	eNVIDIA,
	eINTEL,
	eAMD,
	eWARP
};

typedef struct WindowProperties
{
	UINT width;
	UINT height;
	float aspectRatio;
	HWND hwnd;
} WindowProperties;

extern WindowProperties gWndProp;

