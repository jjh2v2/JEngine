#pragma once
#include "stdafx.h"
//#include "GraphicsDeviceIne_DX12.h"
//#include "TestDX.h"
//#include "TestDX001.h"
#include "TestDX002.h"

//TestDX mTestDX;

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
	//TestDX mTestDX;
	//TestDX001 mTestDX001;
	TestDX002 mTestDX002;

	TimerClass mTimerClass;

public:
	EngineMain() :
		hInstance(S_OK),
		hPrevInstance(S_OK),
		lpCmdLin(0),
		nCmdShow(0),
		mTimerClass(),
		mTestDX002()
		//mTestDX001()
		//mTestDX(),
		//mGraphicsDevice()
	{
	};
	virtual ~EngineMain()
	{};

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	void WindowsCallback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


	void Init(HINSTANCE& _hInstance, HINSTANCE& _hPrevInstance, LPSTR& _lpCmdLine, int& _nCmdShow);
	void InitPlatformWindows(UINT windowWidth = 480, UINT windowHeight = 640);
	void InitModules();

	void OnUpdate();
	void OnRender();

private:
};

