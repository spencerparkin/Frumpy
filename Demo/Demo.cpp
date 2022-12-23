// Demo.cpp : Defines the entry point for the application.
//

// Helpful resources:
// - https://www.youtube.com/watch?v=q1fMa8Hufmg
// - http://www.winprog.org/tutorial/

#include "framework.h"
#include "Demo.h"
#include "Image.h"
#include "Mesh.h"
#include "Vertex.h"
#include <time.h>

Demo::Demo()
{
    this->szTitle[0] = '\0';
    this->szWindowClass[0] = '\0';
    this->hWnd = NULL;
    this->hWndStatusBar = NULL;
    this->frameBitmapHandle = nullptr;
    this->frameDCHandle = nullptr;
    this->framePixelBuffer = nullptr;
    this->exitProgram = false;
    this->scene = nullptr;
    this->camera = nullptr;
    this->image = nullptr;
    this->mesh = nullptr;
    this->rotationAngle = 0.0;
    this->rotationRate = 20.0;
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
    this->scene->clearPixel.color.SetColor(0, 0, 0, 0);

    this->camera = new Frumpy::Camera();
    this->camera->LookAt(Frumpy::Vector(0.0, 0.0, 10.0), Frumpy::Vector(0.0, 0.0, 0.0), Frumpy::Vector(0.0, 1.0, 0.0));

    this->mesh = new Frumpy::Mesh();
    
    this->mesh->SetVertexBufferSize(4);
    
    this->mesh->GetVertex(0)->objectSpacePoint.SetComponents(-1.0, -1.0, 0.0);
    this->mesh->GetVertex(1)->objectSpacePoint.SetComponents(1.0, -1.0, 0.0);
    this->mesh->GetVertex(2)->objectSpacePoint.SetComponents(1.0, 1.0, 0.0);
    this->mesh->GetVertex(3)->objectSpacePoint.SetComponents(-1.0, 1.0, 0.0);

    this->mesh->GetVertex(0)->color.SetComponents(1.0, 0.0, 0.0);
    this->mesh->GetVertex(1)->color.SetComponents(0.0, 1.0, 0.0);
    this->mesh->GetVertex(2)->color.SetComponents(0.0, 0.0, 1.0);
    this->mesh->GetVertex(3)->color.SetComponents(1.0, 1.0, 0.0);

    this->mesh->SetIndexBufferSize(6);

    this->mesh->SetIndex(0, 0);
    this->mesh->SetIndex(1, 1);
    this->mesh->SetIndex(2, 2);
    this->mesh->SetIndex(3, 0);
    this->mesh->SetIndex(4, 2);
    this->mesh->SetIndex(5, 3);

    this->scene->objectList.AddTail(this->mesh);

    this->hInst = hInstance;
    this->exitProgram = false;

    this->frameBitmapInfo.bmiHeader.biSize = sizeof(frameBitmapInfo.bmiHeader);
    this->frameBitmapInfo.bmiHeader.biPlanes = 1;
    this->frameBitmapInfo.bmiHeader.biBitCount = 32;
    this->frameBitmapInfo.bmiHeader.biCompression = BI_RGB;

    this->frameDCHandle = CreateCompatibleDC(NULL);
    // TODO: Error handling.

    LoadString(this->hInst, IDS_APP_TITLE, this->szTitle, MAX_LOADSTRING);
    LoadString(this->hInst, IDC_DEMO, this->szWindowClass, MAX_LOADSTRING);

    WNDCLASSEX winClass;
    winClass.cbSize = sizeof(WNDCLASSEX);
    winClass.style = CS_HREDRAW | CS_VREDRAW;
    winClass.lpfnWndProc = &Demo::WndProc;
    winClass.cbClsExtra = 0;
    winClass.cbWndExtra = sizeof(Demo*);
    winClass.hInstance = this->hInst;
    winClass.hIcon = LoadIcon(this->hInst, MAKEINTRESOURCE(IDI_DEMO));
    winClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    winClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    winClass.lpszMenuName = MAKEINTRESOURCE(IDC_DEMO);
    winClass.lpszClassName = szWindowClass;
    winClass.hIconSm = LoadIcon(this->hInst, MAKEINTRESOURCE(IDI_SMALL));
    ATOM atom = RegisterClassEx(&winClass);
    // TODO: Error handling.

    this->hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, this->hInst, nullptr);
    if (!this->hWnd)
        return false;

    SetWindowLongPtr(this->hWnd, 0, (LONG)this);

    InitCommonControls();

    HMENU hMenu = LoadMenu(this->hInst, winClass.lpszMenuName);
    this->hWndStatusBar = CreateWindowEx(0, STATUSCLASSNAME, (PCTSTR)NULL, SBARS_SIZEGRIP | WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, this->hWnd, hMenu, this->hInst, NULL);

    ShowWindow(this->hWnd, nCmdShow);
    UpdateWindow(this->hWnd);

    HACCEL hAccelTable = LoadAccelerators(this->hInst, MAKEINTRESOURCE(IDC_DEMO));

    return true;
}

void Demo::Run()
{
    clock_t lastTime = clock();

    int frameCount = 0;
    int frameFPSCalcFrequency = 10;
    double totalElapsedTimeSeconds = 0.0;

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

        // Keep track of time taken per loop iteration.
        clock_t currentTime = clock();
        clock_t deltaTime = currentTime - lastTime;
        double deltaTimeSeconds = double(deltaTime) / double(CLOCKS_PER_SEC);
        lastTime = currentTime;
        totalElapsedTimeSeconds += deltaTimeSeconds;
        
        // Animate our mesh by rotating it at a desired rate.
        static bool rotate = true;
        if (rotate)
            this->rotationAngle += this->rotationRate * deltaTimeSeconds;
        //Frumpy::Vector axis(1.0, 0.0, 0.0);
        Frumpy::Vector axis(0.0, 0.0, 1.0);
        this->mesh->childToParent.Rotation(axis, FRUMPY_DEGS_TO_RADS(this->rotationAngle));

        // Ask windows to have us repaint our window.
        InvalidateRect(this->hWnd, NULL, FALSE);
        UpdateWindow(this->hWnd);

        // Periodically show are framerate.
        frameCount++;
        if (frameCount % frameFPSCalcFrequency == 0)
        {
            double fps = double(frameFPSCalcFrequency) / totalElapsedTimeSeconds;
            static char fpsMessage[256];
            sprintf_s(fpsMessage, sizeof(fpsMessage), "Image Size: %d x %d; FPS: %f", this->image->GetWidth(), this->image->GetHeight(), fps);
            SendMessage(this->hWndStatusBar, SB_SETTEXT, 0, (LPARAM)fpsMessage);
            totalElapsedTimeSeconds = 0.0;
        }
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
    if (demo)
        return demo->HandleMessage(hWnd, message, wParam, lParam);

    return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT Demo::HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            switch (wmId)
            {
                case IDM_ABOUT:
                {
                    DialogBox(this->hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
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
                this->frameDCHandle, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);

            EndPaint(hWnd, &ps);
            break;
        }
        case WM_SIZE:
        {
            this->frameBitmapInfo.bmiHeader.biWidth = LOWORD(lParam);
            this->frameBitmapInfo.bmiHeader.biHeight = HIWORD(lParam);

            if (this->frameBitmapHandle != NULL)
            {
                DeleteObject(this->frameBitmapHandle);
                this->frameBitmapHandle = NULL;
            }

            this->frameBitmapHandle = CreateDIBSection(NULL, &this->frameBitmapInfo, DIB_RGB_COLORS, (void**)&this->framePixelBuffer, NULL, 0);
            // TODO: Perform error handling.
            SelectObject(this->frameDCHandle, this->frameBitmapHandle);

            if (!this->image || this->image->GetWidth() != this->frameBitmapInfo.bmiHeader.biWidth || this->image->GetHeight() != this->frameBitmapInfo.bmiHeader.biHeight)
            {
                delete this->image;
                this->image = new Frumpy::Image(this->frameBitmapInfo.bmiHeader.biWidth, this->frameBitmapInfo.bmiHeader.biHeight);
            }

            double aspectRatio = this->image->GetAspectRatio();
            this->camera->frustum.AdjustVFoviForAspectRatio(aspectRatio);

            SendMessage(this->hWndStatusBar, WM_SIZE, 0, 0);

            break;
        }
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            this->exitProgram = true;
            break;
        }
        case WM_QUIT:   // Not sure why we don't get this.
        {
            this->exitProgram = true;
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