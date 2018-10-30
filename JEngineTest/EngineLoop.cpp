#include "stdafx.h"
#include "EngineLoop.h"

LRESULT CALLBACK EngineLoop::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// Handle any messages the switch statement didn't.
	return DefWindowProc(hWnd, message, wParam, lParam);
}

void EngineLoop::Init(HINSTANCE& _hInstance, HINSTANCE& _hPrevInstance, LPSTR& _lpCmdLine, int& _nCmdShow)
{
	hInstance = _hInstance;
	hPrevInstance = _hPrevInstance;
	lpCmdLin = _lpCmdLine;
	nCmdShow = _nCmdShow;
	hWndPlatformWindows = S_OK;
}


void EngineLoop::InitPlatformWindows()
{
	// Window information
	static TCHAR szWindowClass[] = _T("JEngineTest");
	static TCHAR szTitle[] = _T("JEngineTest");

	// Create window class
	WNDCLASSEX wincx;
	wincx.cbSize = sizeof(WNDCLASSEX);
	wincx.style = CS_HREDRAW | CS_VREDRAW;
	wincx.lpfnWndProc = EngineLoop::WndProc;
	wincx.cbClsExtra = 0;
	wincx.cbWndExtra = 0;
	wincx.hInstance = hInstance;
	wincx.hIcon = NULL;
	wincx.hCursor = LoadCursor(NULL, IDC_ARROW);
	wincx.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wincx.lpszMenuName = NULL;
	wincx.lpszClassName = szWindowClass;
	wincx.hIconSm = NULL;

	// Register window
	if (!RegisterClassEx(&wincx))
	{
		MessageBox(NULL, _T("Unable to Register window!"), _T("MA-DX12 Sample"), NULL);
		return;
	}

	UINT windowHeight = 480;
	UINT windowWidth = 640;

	// Create the window
	hWndPlatformWindows = CreateWindow(
		szWindowClass,
		szTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		windowWidth, windowHeight,
		NULL,
		NULL,
		hInstance,
		NULL
	);
	if (!hWndPlatformWindows)
	{
		MessageBox(NULL, _T("Unable to create window!"), _T("MA-DX12 Sample"), NULL);
		return;
	}

	ShowWindow(hWndPlatformWindows, nCmdShow);
}


void EngineLoop::InitModules()
{
	// 런데링 모듈
	mGraphicsDevice.Init();
	// 타이머 모듈
	// 파일 관리자 모듈
	// 등등 초기화
}
