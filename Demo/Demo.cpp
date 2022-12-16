// Demo.cpp : Defines the entry point for the application.
//

// Helpful resource used here: https://www.youtube.com/watch?v=q1fMa8Hufmg

#include "framework.h"
#include "Demo.h"

Demo::Demo()
{
    this->szTitle[0] = '\0';
    this->szWindowClass[0] = '\0';
    this->hWnd = NULL;
    this->frameBitmapHandle = nullptr;
    this->frameDCHandle = nullptr;
    this->framePixelBuffer = nullptr;
    this->exitProgram = false;
}

/*virtual*/ Demo::~Demo()
{
}

void Demo::FillFramebufferWithColor(unsigned char red, unsigned char green, unsigned char blue)
{
    for (int i = 0; i < this->frameBitmapInfo.bmiHeader.biHeight; i++)
    {
        for (int j = 0; j < this->frameBitmapInfo.bmiHeader.biWidth; j++)
        {
            unsigned char* pixel = (unsigned char*)&this->framePixelBuffer[i * this->frameBitmapInfo.bmiHeader.biWidth + j];
            pixel[0] = blue;
            pixel[1] = green;
            pixel[2] = red;
            pixel[4] = 0x00;        // alpha?
        }
    }
}

bool Demo::Setup(HINSTANCE hInstance, int nCmdShow)
{
    this->hInst = hInstance;
    this->exitProgram = false;

    this->frameBitmapInfo.bmiHeader.biSize = sizeof(frameBitmapInfo.bmiHeader);
    this->frameBitmapInfo.bmiHeader.biPlanes = 1;
    this->frameBitmapInfo.bmiHeader.biBitCount = 32;
    this->frameBitmapInfo.bmiHeader.biCompression = BI_RGB;

    this->frameDCHandle = CreateCompatibleDC(NULL);
    // TODO: Error handling.

    LoadStringW(this->hInst, IDS_APP_TITLE, this->szTitle, MAX_LOADSTRING);
    LoadStringW(this->hInst, IDC_DEMO, this->szWindowClass, MAX_LOADSTRING);

    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = &Demo::WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = sizeof(Demo*);
    wcex.hInstance = this->hInst;
    wcex.hIcon = LoadIcon(this->hInst, MAKEINTRESOURCE(IDI_DEMO));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_DEMO);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(this->hInst, MAKEINTRESOURCE(IDI_SMALL));
    ATOM atom = RegisterClassExW(&wcex);
    // TODO: Error handling.

    this->hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, this->hInst, nullptr);
    if (!this->hWnd)
        return false;

    SetWindowLongPtr(this->hWnd, 0, (LONG)this);

    ShowWindow(this->hWnd, nCmdShow);
    UpdateWindow(this->hWnd);

    HACCEL hAccelTable = LoadAccelerators(this->hInst, MAKEINTRESOURCE(IDC_DEMO));

    return true;
}

void Demo::Run()
{
    while (!this->exitProgram)
    {
        // Flush the message queue as far as possible.
        while (PeekMessage(&this->msg, NULL, 0, 0, PM_REMOVE))
            DispatchMessage(&this->msg);

        // Render a frame.
        static unsigned char red = 0x00;
        static unsigned char green = 0x00;
        static unsigned char blue = 0x00;
        FillFramebufferWithColor(red++, green, blue);
        green += 2;
        blue += 3;

        // Ask windows to have us repaint our window.
        InvalidateRect(this->hWnd, NULL, FALSE);
        UpdateWindow(this->hWnd);
    }
}

int Demo::Shutdown()
{
    if (this->frameBitmapHandle != NULL)
    {
        DeleteObject(this->frameBitmapHandle);
        this->frameBitmapHandle = NULL;
    }

    return this->msg.wParam;
}

/*static*/ LRESULT CALLBACK Demo::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    Demo* demo = (Demo*)GetWindowLongPtr(hWnd, 0);

    switch (message)
    {
        case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            switch (wmId)
            {
                case IDM_ABOUT:
                {
                    DialogBox(demo->hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                    break;
                }
                case IDM_EXIT:
                {
                    DestroyWindow(hWnd);
                    break;
                }
                default:
                {
                    return DefWindowProc(hWnd, message, wParam, lParam);
                }
            }
            break;
        }
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top,
                ps.rcPaint.right - ps.rcPaint.left, ps.rcPaint.bottom - ps.rcPaint.top,
                demo->frameDCHandle, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);

            EndPaint(hWnd, &ps);
            break;
        }
        case WM_SIZE:
        {
            demo->frameBitmapInfo.bmiHeader.biWidth = LOWORD(lParam);
            demo->frameBitmapInfo.bmiHeader.biHeight = HIWORD(lParam);

            if (demo->frameBitmapHandle != NULL)
            {
                DeleteObject(demo->frameBitmapHandle);
                demo->frameBitmapHandle = NULL;
            }

            demo->frameBitmapHandle = CreateDIBSection(NULL, &demo->frameBitmapInfo, DIB_RGB_COLORS, (void**)&demo->framePixelBuffer, NULL, 0);
            // TODO: Perform error handling.
            SelectObject(demo->frameDCHandle, demo->frameBitmapHandle);

            demo->FillFramebufferWithColor(0xFF, 0x00, 0x00);

            break;
        }
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            demo->exitProgram = true;
            break;
        }
        case WM_QUIT:   // Not sure why we don't get this.
        {
            demo->exitProgram = true;
            break;
        }
        default:
        {
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    return 0;
}

/*static*/ INT_PTR CALLBACK Demo::About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    switch (message)
    {
        case WM_INITDIALOG:
        {
            return (INT_PTR)TRUE;
        }
        case WM_COMMAND:
        {
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
            {
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
            break;
        }
    }

    return (INT_PTR)FALSE;
}