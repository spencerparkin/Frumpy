#pragma once

#include "resource.h"

#define MAX_LOADSTRING 100

class Demo
{
public:
	Demo();
	virtual ~Demo();

	bool Setup(HINSTANCE hInstance, int nCmdShow);
	int Shutdown();
	void Run();

private:

	void FillFramebufferWithColor(unsigned char red, unsigned char green, unsigned char blue);

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

	MSG msg;
	HINSTANCE hInst;
	WCHAR szTitle[MAX_LOADSTRING];
	WCHAR szWindowClass[MAX_LOADSTRING];
	HWND hWnd = NULL;
	BITMAPINFO frameBitmapInfo;
	HBITMAP frameBitmapHandle;
	HDC frameDCHandle;
	unsigned int* framePixelBuffer;
	bool exitProgram;
};