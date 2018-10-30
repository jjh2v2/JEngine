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


#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"Dxgi.lib")
#pragma comment(lib,"dxguid.lib")

// Input includes
#include <Xinput.h>
#pragma comment(lib,"xinput.lib")

// Sound include
#include <xaudio2.h>
#pragma comment(lib,"xaudio2.lib")

namespace MS = Microsoft::WRL;

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