#include "stdafx.h"
#include "EngineMain.h"

LRESULT CALLBACK EngineMain::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// Handle any messages the switch statement didn't.
	static EngineMain *pthis = nullptr; // this �� ����� ������ 
	switch (message)
	{
	case WM_CREATE:
		// ������ �� �Ѱܹ��� this �����͸� �����Ͽ� ���� Ŭ���� ��������� ����Ѵ� 
		pthis = (EngineMain*)(((LPCREATESTRUCT)lParam)->lpCreateParams);
		return 0;
	case WM_PAINT:
		//hDC = BeginPaint(hWnd, &ps);
		//GetClientRect(hWnd, &rect);
		//// ����� ������ �� ������ �����͸� ���� �����Ѵ�. paClass-> 
		//// �׳� m_szMsg �� �����ϸ� ������ ������ ����. 
		//DrawText(hDC, paClass->m_szMsg, -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		//EndPaint(hWnd, &ps);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	if (pthis)
	{
		pthis->WindowsCallback(hWnd, message, wParam, lParam);
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

void EngineMain::WindowsCallback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//mTestDX.HandleWindowsCallback(hwnd, uMsg, wParam, lParam);
	mTestDX002.HandleWindowsCallback(hwnd, uMsg, wParam, lParam);
}

void EngineMain::Init(HINSTANCE& _hInstance, HINSTANCE& _hPrevInstance, LPSTR& _lpCmdLine, int& _nCmdShow)
{
	hInstance = _hInstance;
	hPrevInstance = _hPrevInstance;
	lpCmdLin = _lpCmdLine;
	nCmdShow = _nCmdShow;
	hWndPlatformWindows = S_OK;
}


void EngineMain::InitPlatformWindows(UINT windowWidth /*= 480*/, UINT windowHeight /*= 640*/)
{
	// Window information
	TCHAR szWindowClass[] = _T("JEngineTest");
	TCHAR szTitle[] = _T("JEngineTest");

	// Create window class
	WNDCLASSEX wincx;
	wincx.cbSize = sizeof(WNDCLASSEX);
	wincx.style = CS_HREDRAW | CS_VREDRAW;
	wincx.lpfnWndProc = EngineMain::WndProc;
	//wincx.lpfnWndProc = (WNDPROC)fd::make_async_msg_proc(fd::make_delegate(&EngineMain::WndProc, this));

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
		this
	);
	if (!hWndPlatformWindows)
	{
		MessageBox(NULL, _T("Unable to create window!"), _T("MA-DX12 Sample"), NULL);
		return;
	}

	ShowWindow(hWndPlatformWindows, nCmdShow);

	gWndProp.width = windowWidth;
	gWndProp.height = windowHeight;
	gWndProp.aspectRatio = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
	gWndProp.hwnd = hWndPlatformWindows;
}


void EngineMain::InitModules()
{
	// Ÿ�̸� ���
	mTimerClass.Initialize();

	// ���� ������ ���

	// ��� �ʱ�ȭ

	// ������ ���
	//mGraphicsDevice.Init();
	//mTestDX.Init();
	mTestDX002.Init();
}

void EngineMain::OnUpdate()
{
	mTimerClass.Frame();

	//mTestDX.OnUpdate(mTimerClass.GetTime());
	mTestDX002.OnUpdate(mTimerClass.GetTime());
}
void EngineMain::OnRender()
{
	//mTestDX.OnRender();
	mTestDX002.OnRender();
}
