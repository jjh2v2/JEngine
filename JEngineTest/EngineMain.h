#pragma once
#include "stdafx.h"
#include "GraphicsDeviceIne_DX12.h"
#include "TestDX.h"

class EngineMain
{
private:
	HINSTANCE hInstance;
	HINSTANCE hPrevInstance;
	LPSTR lpCmdLin;
	int nCmdShow;
	HWND hWndPlatformWindows;

private:
	//GraphicsDeviceIne_DX12 mGraphicsDevice;
	TestDX mTestDX;

public:
	EngineMain() :
		hInstance(S_OK),
		hPrevInstance(S_OK),
		lpCmdLin(0),
		nCmdShow(0),
		mTestDX()
		//mGraphicsDevice()
	{
	};
	virtual ~EngineMain()
	{};

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	void Init(HINSTANCE& _hInstance, HINSTANCE& _hPrevInstance, LPSTR& _lpCmdLine, int& _nCmdShow);
	void InitPlatformWindows(UINT windowWidth = 480, UINT windowHeight = 640);
	void InitModules();

	void OnUpdate();
	void OnRender();

private:
};

