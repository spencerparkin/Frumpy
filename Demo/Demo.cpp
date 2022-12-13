// Demo.cpp : Defines the entry point for the application.
//

// Helpful resource used here: https://www.youtube.com/watch?v=q1fMa8Hufmg

#include "framework.h"
#include "Demo.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HWND hWnd = NULL;
BITMAPINFO frameBitmapInfo;
HBITMAP frameBitmapHandle = NULL;
HDC frameDCHandle = NULL;
unsigned int* framePixelBuffer = NULL;
bool exitProgram = false;

void FillFramebufferWithColor(unsigned char red, unsigned char green, unsigned char blue)
{
    for (int i = 0; i < frameBitmapInfo.bmiHeader.biHeight; i++)
    {
        for (int j = 0; j < frameBitmapInfo.bmiHeader.biWidth; j++)
        {
            unsigned char* pixel = (unsigned char*)&framePixelBuffer[i * frameBitmapInfo.bmiHeader.biWidth + j];
            pixel[0] = blue;
            pixel[1] = green;
            pixel[2] = red;
            pixel[4] = 0x00;        // alpha?
        }
    }
}

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    frameBitmapInfo.bmiHeader.biSize = sizeof(frameBitmapInfo.bmiHeader);
    frameBitmapInfo.bmiHeader.biPlanes = 1;
    frameBitmapInfo.bmiHeader.biBitCount = 32;
    frameBitmapInfo.bmiHeader.biCompression = BI_RGB;

    frameDCHandle = CreateCompatibleDC(NULL);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_DEMO, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DEMO));

    MSG msg;

    // Main message loop:
    while (!exitProgram)
    {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            DispatchMessage(&msg);

        static unsigned char red = 0x00;
        static unsigned char green = 0x00;
        static unsigned char blue = 0x00;
        FillFramebufferWithColor(red++, green, blue);
        green += 2;
        blue += 3;

        InvalidateRect(hWnd, NULL, FALSE);
        UpdateWindow(hWnd);
    }

#if 0       // Old message loop...
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
#endif

    if (frameBitmapHandle != NULL)
    {
        DeleteObject(frameBitmapHandle);
        frameBitmapHandle = NULL;
    }

    return (int) msg.wParam;
}


//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DEMO));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_DEMO);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            
            BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right - ps.rcPaint.left, ps.rcPaint.bottom - ps.rcPaint.top, frameDCHandle, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_SIZE:
        frameBitmapInfo.bmiHeader.biWidth = LOWORD(lParam);
        frameBitmapInfo.bmiHeader.biHeight = HIWORD(lParam);

        if (frameBitmapHandle != NULL)
        {
            DeleteObject(frameBitmapHandle);
            frameBitmapHandle = NULL;
        }

        frameBitmapHandle = CreateDIBSection(NULL, &frameBitmapInfo, DIB_RGB_COLORS, (void**)&framePixelBuffer, NULL, 0);
        // TODO: Perform error handling.
        SelectObject(frameDCHandle, frameBitmapHandle);

        FillFramebufferWithColor(0xFF, 0x00, 0x00);

        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        exitProgram = true;
        break;
    case WM_QUIT:   // Not sure why we don't get this.
        exitProgram = true;
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
