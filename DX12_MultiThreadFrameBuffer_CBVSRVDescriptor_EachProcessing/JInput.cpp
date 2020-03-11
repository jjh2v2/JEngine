#pragma once
#include "stdafx.h"
#include "JInput.h"


void JInput::OnMouseDown(HWND hwnd, WPARAM buttonState, int x, int y)
{
	this->buttonState = buttonState;
	prevMousePos.x = static_cast<float>(x);
	prevMousePos.y = static_cast<float>(y);
	SetCapture(hwnd);
	//selectedEntities.clear();
	//for (int i = 0; i < entities.size(); ++i)
	//{
	//	float distance;
	//	if (IsIntersecting(entities[i], camera, x, y, distance))
	//	{
	//		selectedEntities.push_back(entities[i]);
	//		printf("Intersecting %d\n", i);
	//		break;
	//	}
	//}

}

void JInput::OnMouseUp(WPARAM buttonState, int x, int y)
{
	this->buttonState = buttonState;
	ReleaseCapture();
}

void JInput::OnMouseMove(WPARAM buttonState, int x, int y)
{
	this->buttonState = buttonState;
	if (this->buttonState & BUTTON_STATE::RIGHT)
	{
		prevMouseRot.x = (x - prevMousePos.x) * 0.005f;
		prevMouseRot.y = (y - prevMousePos.y) * 0.005f;
		//float xDiff = (x - prevMousePos.x) * 0.005f;
		//float yDiff = (y - prevMousePos.y) * 0.005f;
		//mCamera->Rotate(yDiff, xDiff);
	}

	prevMousePos.x = static_cast<float>(x);
	prevMousePos.y = static_cast<float>(y);
}
