#pragma once

#include "resource.h"
#include "Scene.h"
#include "Camera.h"
#include "Image.h"
#include "Mesh.h"

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

	void UpdateFramebuffer();

	LRESULT HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

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
	Frumpy::Scene* scene;
	Frumpy::Camera* camera;
	Frumpy::Image* image;
	Frumpy::Mesh* mesh;
	double rotationAngle;
	double rotationRate;
};