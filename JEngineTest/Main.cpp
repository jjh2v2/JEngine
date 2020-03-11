#include "stdafx.h"
#include "EngineMain.h"

EngineMain gEngineMain;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	gEngineMain.Init(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
	gEngineMain.InitPlatformWindows(1024, 820);
	gEngineMain.InitModules();

	// Variables for FPS
	DWORD dwFrames = 0;
	DWORD dwCurrentTime = 0;
	DWORD dwLastUpdateTime = 0;
	DWORD dwElapsedTime = 0;
	TCHAR szFPS[128];
	szFPS[0] = '\0';

	LARGE_INTEGER frequency, counter, elapsed, lastCounter;
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&counter);

	double framesPerSecond = 0.0, frameAccumulator = 0.0;
	int frameCounter = 0;
	// Main loop.
	MSG msg = { 0 };
	while (true)
	{
		lastCounter = counter;
		QueryPerformanceCounter(&counter);
		elapsed.QuadPart = counter.QuadPart - lastCounter.QuadPart;
		double curFrameTime = static_cast<double>(elapsed.QuadPart) / static_cast<double>(frequency.QuadPart);
		frameCounter++;
		frameAccumulator += curFrameTime;

		// Process any messages in the queue.
		bool is = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
		if (is)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
				break;
		}

		gEngineMain.OnUpdate();
		gEngineMain.OnRender();
		//gRenderObj->OnUpdate();
		//gRenderObj->OnRender();

		//if (frameCounter == 15)
		//{
		//	double frameTimeAvg = frameAccumulator / frameCounter;
		//	double framesPerSecondAvg = 1.0 / frameTimeAvg;
		//	swprintf_s(szFPS, sizeof(szFPS) / sizeof(TCHAR), L"FPS = %.2f",
		//		static_cast<float>(framesPerSecondAvg));
		//	frameAccumulator = frameTimeAvg;
		//	frameCounter = 1;
		//	// Write the FPS onto the window title.
		//	SetWindowText(hWnd, szFPS);
		//}
	}

	return 0;
}
