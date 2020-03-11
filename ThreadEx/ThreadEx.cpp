// https://adolys.tistory.com/entry/%EC%8A%A4%EB%A0%88%EB%93%9C-%EC%9D%B4%EB%B2%A4%ED%8A%B8Thread-Event
//

#include "pch.h"
#include <iostream>
#include <thread>
#include <windows.h>
#include <stdio.h>



#define BUFSIZE 16



HANDLE hReadEvent;
HANDLE hWriteEvent;
char buf[BUFSIZE];



DWORD WINAPI WriteThread(LPVOID arg)
{
	DWORD retval;

	for (int k = 0; k < 10; k++) {
		retval = WaitForSingleObject(hReadEvent, INFINITE);     // 읽기 완료를 기다림(Read이벤트 끝날때까지 대기)
		if (retval == WAIT_FAILED) break;



		// 공유 버퍼에 데이터 쓰기
		for (int i = 0; i < BUFSIZE; i++)
			buf[i] = 3;
		SetEvent(hWriteEvent);                                              // 쓰기 완료를 알림(Write이벤트 신호 상태로 변경)

	}
	CloseHandle(hWriteEvent);                                            // 이벤트 제거
	return 0;
}



DWORD WINAPI ReadThread(LPVOID arg)
{
	DWORD retval;

	while (1) {
		retval = WaitForSingleObject(hWriteEvent, INFINITE);     // 쓰기 완료를 기다림(Write이벤트 끝날때까지 대기)
		if (retval == WAIT_FAILED) break;


		// 읽은 데이터를 출력
		printf("Thread %d:\t", GetCurrentThreadId());
		for (int i = 0; i < BUFSIZE; i++)
			printf("%d ", buf[i]);
		printf("\n");


		// 버퍼를 0으로 초기화
		ZeroMemory(buf, sizeof(buf));                                   // WriteThread가 실행 안되면 출력값은 '0' 이다.

		SetEvent(hReadEvent);                                             // 읽기 완료를 알림(Read이벤트 신호 상태로 변경)
	}
	return 0;
}

void Render(int index)
{
	printf("Thread %d\n", index);
	for (int i=0; i<1000; i++)
	{
		printf("%d",index);
	}
}

int main(void)
{
	std::thread thread1(Render, 1);
	std::thread thread2(Render, 2);
	std::thread thread3(Render, 3);


	thread1.join();
	thread2.join();
	thread3.join();


	// 이벤트 생성

	// Read와 Write 둘다 자동 리셋 이벤트로 생성

	// 대기중인 스레드중 하나의 스레드만 이벤트 제어권을 넘겨받고 자동으로 비신호 상태가 된다.
	hReadEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (hReadEvent == NULL) return -1;
	hWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (hWriteEvent == NULL) return -1;

	// 세 개의 스레드 생성
	HANDLE hThread[3];
	DWORD ThreadId[3];
	hThread[0] = CreateThread(NULL, 0, WriteThread, NULL, 0, &ThreadId[0]);
	hThread[1] = CreateThread(NULL, 0, ReadThread, NULL, 0, &ThreadId[1]);
	hThread[2] = CreateThread(NULL, 0, ReadThread, NULL, 0, &ThreadId[2]);

	// 스레드 종료 대기
	WaitForMultipleObjects(3, hThread, TRUE, INFINITE);


	// 이벤트 제거
	CloseHandle(hReadEvent);
	printf("모든 작업을 완료했습니다.\n");
	return 0;
}