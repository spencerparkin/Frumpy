// Demo.cpp : Defines the entry point for the application.
//

// Helpful resource used here: https://www.youtube.com/watch?v=q1fMa8Hufmg

#include "framework.h"
#include "Demo.h"
#include "Image.h"

Demo::Demo()
{
    this->szTitle[0] = '\0';
    this->szWindowClass[0] = '\0';
    this->hWnd = NULL;
    this->frameBitmapHandle = nullptr;
    this->frameDCHandle = nullptr;
    this->framePixelBuffer = nullptr;
    this->exitProgram = false;
    this->scene = nullptr;
    this->camera = nullptr;
    this->image = nullptr;
}

/*virtual*/ Demo::~Demo()
{
}

void Demo::UpdateFramebuffer()
{
    if (this->image && this->image->GetWidth() == this->frameBitmapInfo.bmiHeader.biWidth && this->image->GetHeight() == this->frameBitmapInfo.bmiHeader.biHeight)
    {
        for (unsigned int i = 0; i < (unsigned)this->frameBitmapInfo.bmiHeader.biHeight; i++)
        {
            for (unsigned int j = 0; j < (unsigned)this->frameBitmapInfo.bmiHeader.biWidth; j++)
            {
                unsigned char* dstPixel = (unsigned char*)&this->framePixelBuffer[i * this->frameBitmapInfo.bmiHeader.biWidth + j];
                const Frumpy::Image::Pixel* srcPixel = this->image->GetPixel(Frumpy::Image::Location{ i, j });
                
                dstPixel[0] = srcPixel->color.blue;
                dstPixel[1] = srcPixel->color.green;
                dstPixel[2] = srcPixel->color.red;
                dstPixel[4] = 0x00;        // alpha?
            }
        }
    }
}

bool Demo::Setup(HINSTANCE hInstance, int nCmdShow)
{
    this->scene = new Frumpy::Scene();
    this->scene->clearPixel.color.SetColor(255, 255, 255, 0);

    this->camera = new Frumpy::Camera();

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
        if (this->image && this->camera && this->scene)
        {
            // TODO: We could save a step by rendering directly into the windows bitmap.
            this->scene->Render(*this->camera, *this->image);
            this->UpdateFramebuffer();
        }

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

    if (this->scene)
    {
        delete this->scene;
        this->scene = nullptr;
    }

    if (this->camera)
    {
        delete this->camera;
        this->camera = nullptr;
    }

    if (this->image)
    {
        delete this->image;
        this->image = nullptr;
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

            if (!demo->image || demo->image->GetWidth() != demo->frameBitmapInfo.bmiHeader.biWidth || demo->image->GetHeight() != demo->frameBitmapInfo.bmiHeader.biHeight)
            {
                delete demo->image;
                demo->image = new Frumpy::Image(demo->frameBitmapInfo.bmiHeader.biWidth, demo->frameBitmapInfo.bmiHeader.biHeight);
            }

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