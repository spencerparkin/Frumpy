#pragma once

#include "resource.h"
#include "Scene.h"
#include "Camera.h"
#include "FileAssets/Image.h"
#include "FileAssets/Mesh.h"
#include "Renderer.h"
#include "ProfileBlock.h"
#include "Vector.h"

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

	void HandleKeyboardInput(double deltaTimeSeconds);

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
	Frumpy::Renderer* renderer;
	Frumpy::List<Frumpy::FileFormat::Asset*> assetList;
	double rotationAngle;
	double rotationRate;
	TimingStat frumpyRenderTime;
	TimingStat demoBlitTime;
	TimingStat demoMessageTime;
	DWORD lastMouseMove;
};