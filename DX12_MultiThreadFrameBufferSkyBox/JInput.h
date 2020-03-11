#pragma once
#include "stdafx.h"


enum BUTTON_STATE {
	LEFT = 0x0001,
	RIGHT = 0x0002,
};
struct JInputPoint
{
	float x;
	float y;
};

class JInput
{
public: 
	static JInput& instance() {
		static JInput* instance = new JInput();
		return *instance;
	};
private: 
	JInput() :
		buttonState(),
		prevMousePos(),
		prevMouseRot()
	{
		buttonState = 0x00;
		prevMousePos.x = 0; prevMousePos.y = 0;
		prevMouseRot.x = 0; prevMouseRot.y = 0;
	};
	WPARAM buttonState;
	JInputPoint prevMousePos;
	JInputPoint prevMouseRot;

public:
	WPARAM& GetButtonState()
	{
		return buttonState;
	};
	JInputPoint& GetMousePoint()
	{
		return prevMousePos;
	};
	JInputPoint& GetMouseRot()
	{
		return prevMouseRot;
	};
	void OnMouseDown(HWND hwnd, WPARAM buttonState, int x, int y);
	void OnMouseUp(WPARAM buttonState, int x, int y);
	void OnMouseMove(WPARAM buttonState, int x, int y);
};

