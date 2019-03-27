#pragma once
#include "stdafx.h"
#include "DXDevice.h"

class GraphicsDeviceIne_DX12
{
public:
	GraphicsDeviceIne_DX12();
	virtual ~GraphicsDeviceIne_DX12();

	void Init();

	std::vector<DXDevice*>& GetDXDevice()
	{
		return devices;
	}

private:
	std::vector<DXDevice*> devices;
};

