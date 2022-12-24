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
#include "FileFormats/OBJFormat.h"
#include "ProfileBlock.h"
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
    this->depthBuffer = nullptr;
    this->mesh = nullptr;
    this->renderer = nullptr;
    this->rotationAngle = 0.0;
    this->rotationRate = 20.0;
}

/*virtual*/ Demo::~Demo()
{
}

void Demo::UpdateFramebuffer()
{
    Frumpy::Vector texCoords(0.0, 0.0, 0.0);

    // Up-sample from the Frumpy image to the BMP image.
    for (unsigned int i = 0; i < (unsigned)this->frameBitmapInfo.bmiHeader.biHeight; i++)
    {
        texCoords.y = double(i) / double(this->frameBitmapInfo.bmiHeader.biHeight);

        for (unsigned int j = 0; j < (unsigned)this->frameBitmapInfo.bmiHeader.biWidth; j++)
        {
            texCoords.x = double(j) / double(this->frameBitmapInfo.bmiHeader.biWidth);

            unsigned char* dstPixel = (unsigned char*)&this->framePixelBuffer[i * this->frameBitmapInfo.bmiHeader.biWidth + j];
            const Frumpy::Image::Pixel* srcPixel = this->image->GetPixel(texCoords);
                
            dstPixel[0] = srcPixel->color.blue;
            dstPixel[1] = srcPixel->color.green;
            dstPixel[2] = srcPixel->color.red;
            dstPixel[4] = 0x00;        // alpha?
        }
    }
}

bool Demo::Setup(HINSTANCE hInstance, int nCmdShow)
{
    this->image = new Frumpy::Image(300, 300);
    this->depthBuffer = new Frumpy::Image(this->image->GetWidth(), this->image->GetHeight());

    this->renderer = new Frumpy::Renderer();
    this->renderer->SetImage(this->image);
    this->renderer->SetDepthBuffer(this->depthBuffer);
    this->renderer->Startup(10);

    this->scene = new Frumpy::Scene();
    this->scene->clearPixel.color.SetColor(0, 0, 0, 0);

    this->camera = new Frumpy::Camera();
    this->camera->LookAt(Frumpy::Vector(0.0, 0.0, 60.0), Frumpy::Vector(0.0, 0.0, 0.0), Frumpy::Vector(0.0, 1.0, 0.0));

    Frumpy::List<Frumpy::FileFormat::Asset*> assetList;
    Frumpy::OBJFormat objFormat;
    if (!objFormat.LoadAssets("Meshes/Teapot.obj", assetList) || assetList.GetCount() != 1)
        return false;

    this->mesh = dynamic_cast<Frumpy::Mesh*>(assetList.GetHead()->value);
    this->scene->objectList.AddTail(this->mesh);

    srand(0);
    for (unsigned int i = 0; i < this->mesh->GetVertexBufferSize(); i++)
    {
        Frumpy::Vertex* vertex = this->mesh->GetVertex(i);
        vertex->color.x = FRUMPY_CLAMP(double(rand()) / double(RAND_MAX), 0.0, 1.0);
        vertex->color.y = FRUMPY_CLAMP(double(rand()) / double(RAND_MAX), 0.0, 1.0);
        vertex->color.z = FRUMPY_CLAMP(double(rand()) / double(RAND_MAX), 0.0, 1.0);
    }

    assetList.Clear();

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

    SetWindowPos(this->hWnd, HWND_TOP, 0, 0, 512, 512, SWP_NOMOVE);

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
        {
            ProfileBlock profileBlock(&this->demoMessageTime);
            while (PeekMessage(&this->msg, NULL, 0, 0, PM_REMOVE))
                DispatchMessage(&this->msg);
        }

        // Render a frame.
        if (this->renderer && this->camera && this->scene)
        {
            // Have frumpy render into our image.
            {
                ProfileBlock profileBlock(&this->frumpyRenderTime);
                this->scene->Render(*this->camera, *this->renderer);
            }

            // Up-sample the frumpy image into the windows BMP.
            {
                ProfileBlock profileBlock(&this->demoUpSampleTime);
                this->UpdateFramebuffer();
            }
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
        Frumpy::Vector axis(0.0, 1.0, 0.0);
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
            sprintf_s(fpsMessage, sizeof(fpsMessage), "Image Size: %d x %d; FPS: %2.4f; Render: %2.4f; Blit: %2.4f; Msg: %2.4f; Up-sample: %2.4f",
                this->image->GetWidth(),
                this->image->GetHeight(),
                fps,
                this->frumpyRenderTime.GetAverageMilliseconds(),
                this->demoBlitTime.GetAverageMilliseconds(),
                this->demoMessageTime.GetAverageMilliseconds(),
                this->demoUpSampleTime.GetAverageMilliseconds());
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

    if (this->renderer)
    {
        this->renderer->Shutdown();
        delete this->renderer;
        this->renderer = nullptr;
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

    if (this->depthBuffer)
    {
        delete this->depthBuffer;
        this->depthBuffer = nullptr;
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
            ProfileBlock profileBlock(&this->demoBlitTime);

            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            // TODO: Render directly from Frompy into windows BMP and then replace this with StretchDIBits?
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

            double aspectRatio = double(this->frameBitmapInfo.bmiHeader.biWidth) / double(this->frameBitmapInfo.bmiHeader.biHeight);
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