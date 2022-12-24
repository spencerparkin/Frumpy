#pragma once

#include "resource.h"
#include "Scene.h"
#include "Camera.h"
#include "Image.h"
#include "Mesh.h"
#include "Renderer.h"
#include "ProfileBlock.h"

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

	LRESULT HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

	MSG msg;
	HINSTANCE hInst;
	TCHAR szTitle[MAX_LOADSTRING];
	TCHAR szWindowClass[MAX_LOADSTRING];
	HWND hWnd;
	HWND hWndStatusBar;
	BITMAPINFO frameBitmapInfo;
	HBITMAP frameBitmapHandle;
	HDC frameDCHandle;
	unsigned int* framePixelBuffer;
	bool exitProgram;
	Frumpy::Scene* scene;
	Frumpy::Camera* camera;
	Frumpy::Image* image;
	Frumpy::Image* depthBuffer;
	Frumpy::Mesh* mesh;
	Frumpy::Renderer* renderer;
	double rotationAngle;
	double rotationRate;
	TimingStat frumpyRenderTime;
	TimingStat demoBlitTime;
	TimingStat demoMessageTime;
};