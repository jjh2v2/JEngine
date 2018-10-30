#pragma once
#include "stdafx.h"
#include "GraphicsDeviceIne_DX12.h"

class EngineLoop
{
private:
	HINSTANCE hInstance;
	HINSTANCE hPrevInstance;
	LPSTR lpCmdLin;
	int nCmdShow;
	HWND hWndPlatformWindows;

private:
	GraphicsDeviceIne_DX12 mGraphicsDevice;

public:
	EngineLoop() :
		hInstance(S_OK),
		hPrevInstance(S_OK),
		lpCmdLin(0),
		nCmdShow(0),
		mGraphicsDevice()
	{
	};
	virtual ~EngineLoop()
	{};

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	void Init(HINSTANCE& _hInstance, HINSTANCE& _hPrevInstance, LPSTR& _lpCmdLine, int& _nCmdShow);
	void InitPlatformWindows();
	void InitModules();

private:
};

