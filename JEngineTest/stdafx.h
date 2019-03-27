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

struct Vertex
{
	Vertex() {}
	Vertex(float x, float y, float z, float u, float v) : pos(x, y, z), uv(u, v) {}
	XMFLOAT3 pos;
	XMFLOAT2 uv;
	XMFLOAT3 normal;
	XMFLOAT3 tangent;
	XMFLOAT4 color;
};

struct VertexInstanceData
{
	XMFLOAT3 position;
};

struct ScreenQuadVertex
{
	DirectX::XMFLOAT4 position;
	DirectX::XMFLOAT2 uv;
};