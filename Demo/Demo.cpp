// Demo.cpp : Defines the entry point for the application.
//

// Helpful resources:
// - https://www.youtube.com/watch?v=q1fMa8Hufmg
// - http://www.winprog.org/tutorial/

#include "framework.h"
#include "Demo.h"
#include "Vertex.h"
#include "AssetManager.h"
#include "SceneObjects/MeshObject.h"
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
    this->renderer = nullptr;
    this->directionLight = nullptr;
    this->assetManager = nullptr;
    this->rotationAngle = 0.0;
    this->rotationRate = 20.0;
    this->lastMouseMove = -1;
}

/*virtual*/ Demo::~Demo()
{
}

bool Demo::Setup(HINSTANCE hInstance, int nCmdShow)
{
    this->hInst = hInstance;
    this->exitProgram = false;

    this->assetManager = new Frumpy::AssetManager();
    this->assetManager->LoadAssets("Images/enterprise1701d.ppm");
    this->assetManager->LoadAssets("Meshes/Cube.obj");

    this->frameBitmapInfo.bmiHeader.biSize = sizeof(frameBitmapInfo.bmiHeader);
    this->frameBitmapInfo.bmiHeader.biPlanes = 1;
    this->frameBitmapInfo.bmiHeader.biBitCount = 32;
    this->frameBitmapInfo.bmiHeader.biCompression = BI_RGB;
    this->frameBitmapInfo.bmiHeader.biWidth = 400;
    this->frameBitmapInfo.bmiHeader.biHeight = 400;

    this->frameDCHandle = CreateCompatibleDC(NULL);
    // TODO: Error handling.

    this->frameBitmapHandle = CreateDIBSection(NULL, &this->frameBitmapInfo, DIB_RGB_COLORS, (void**)&this->framePixelBuffer, NULL, 0);
    // TODO: Perform error handling.

    SelectObject(this->frameDCHandle, this->frameBitmapHandle);

    Frumpy::Image::Format format;
    format.bShift = 0;
    format.gShift = 8;
    format.rShift = 16;
    format.aShift = 24;

    this->image = new Frumpy::Image();
    this->image->SetRawPixelBuffer(this->framePixelBuffer, this->frameBitmapInfo.bmiHeader.biWidth, this->frameBitmapInfo.bmiHeader.biHeight);
    this->image->SetFormat(format);

    this->depthBuffer = new Frumpy::Image(this->image->GetWidth(), this->image->GetHeight());

    this->directionLight = new Frumpy::DirectionalLight();
    this->directionLight->directionWorldSpace.SetComponents(-0.5, -1.0, 0.0);
    this->directionLight->directionWorldSpace.Normalize();
    this->directionLight->ambientIntensity = 0.1;

    this->renderer = new Frumpy::Renderer();
    this->renderer->SetFramebuffer(this->image);
    this->renderer->SetDepthBuffer(this->depthBuffer);
    this->renderer->SetLightSource(this->directionLight);
    this->renderer->Startup(10);

    this->scene = new Frumpy::Scene();
    this->scene->clearPixel.color = 0;

    this->camera = new Frumpy::Camera();
    this->camera->LookAt(Frumpy::Vector(0.0, 0.0, 100.0), Frumpy::Vector(0.0, 0.0, 0.0), Frumpy::Vector(0.0, 1.0, 0.0));

    Frumpy::MeshObject* teapotObject = new Frumpy::MeshObject();
    teapotObject->SetMesh(dynamic_cast<Frumpy::Mesh*>(this->assetManager->FindAssetByName("Box001")));
    teapotObject->GetMesh()->SetColor(Frumpy::Vector(1.0, 0.0, 0.0));
    strcpy_s(teapotObject->name, "object");
    this->scene->objectList.AddTail(teapotObject);

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

    //SetWindowPos(this->hWnd, HWND_TOP, 0, 0, 512, 512, SWP_NOMOVE);

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

        // Keep track of time taken per loop iteration.
        clock_t currentTime = clock();
        clock_t deltaTime = currentTime - lastTime;
        double deltaTimeSeconds = double(deltaTime) / double(CLOCKS_PER_SEC);
        lastTime = currentTime;
        totalElapsedTimeSeconds += deltaTimeSeconds;

        // Let the user control the camera.
        this->HandleKeyboardInput(deltaTimeSeconds);

        // Animate our mesh by rotating it at a desired rate.
        Frumpy::MeshObject* object = (Frumpy::MeshObject*)this->scene->FindObjectByName("object");
        if (object)
        {
            this->rotationAngle += this->rotationRate * deltaTimeSeconds;
            Frumpy::Vector axis(0.0, 1.0, 0.0);
            object->childToParent.Rotation(axis, FRUMPY_DEGS_TO_RADS(this->rotationAngle));
        }

        // Render a frame directly into the windows BMP memory.
        if (this->renderer && this->camera && this->scene)
        {
            ProfileBlock profileBlock(&this->frumpyRenderTime);
            this->scene->Render(*this->camera, *this->renderer);
        }

        // Ask windows to have us repaint our window.
        InvalidateRect(this->hWnd, NULL, FALSE);
        UpdateWindow(this->hWnd);

        // Periodically show are framerate.
        frameCount++;
        if (frameCount % frameFPSCalcFrequency == 0)
        {
            double fps = double(frameFPSCalcFrequency) / totalElapsedTimeSeconds;
            static char fpsMessage[256];
            sprintf_s(fpsMessage, sizeof(fpsMessage), "Image Size: %d x %d; FPS: %2.4f; Render: %2.4f; Blit: %2.4f; Msg: %2.4f",
                this->image->GetWidth(),
                this->image->GetHeight(),
                fps,
                this->frumpyRenderTime.GetAverageMilliseconds(),
                this->demoBlitTime.GetAverageMilliseconds(),
                this->demoMessageTime.GetAverageMilliseconds());
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

    if (this->directionLight)
    {
        delete this->directionLight;
        this->directionLight = nullptr;
    }

    if (this->assetManager)
    {
        delete this->assetManager;
        this->assetManager = nullptr;
    }

    return this->msg.wParam;
}

void Demo::HandleKeyboardInput(double deltaTimeSeconds)
{
    Frumpy::Vector cameraRight, cameraForward, cameraUp;
    this->camera->worldTransform.GetAxes(cameraRight, cameraUp, cameraForward);
    cameraForward *= -1.0;

    Frumpy::Vector cameraVelocity;
    double cameraSpeed = 20.0;
    
    if ((GetAsyncKeyState('E') & 0x8000) != 0x0000)
        cameraVelocity += cameraForward * cameraSpeed;
    if ((GetAsyncKeyState('D') & 0x8000) != 0x0000)
        cameraVelocity -= cameraForward * cameraSpeed;
    if ((GetAsyncKeyState('S') & 0x8000) != 0x0000)
        cameraVelocity -= cameraRight * cameraSpeed;
    if ((GetAsyncKeyState('F') & 0x8000) != 0x0000)
        cameraVelocity += cameraRight * cameraSpeed;

    Frumpy::Vector cameraPosition;
    this->camera->worldTransform.GetCol(3, cameraPosition);
    cameraPosition += cameraVelocity * deltaTimeSeconds;
    this->camera->worldTransform.SetCol(3, cameraPosition);
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

            RECT statusBarRect;
            GetWindowRect(this->hWndStatusBar, &statusBarRect);
            DWORD statBarHeight = statusBarRect.bottom - statusBarRect.top;

            int result = StretchDIBits(hdc, ps.rcPaint.left, ps.rcPaint.top,
                ps.rcPaint.right - ps.rcPaint.left, ps.rcPaint.bottom - ps.rcPaint.top - statBarHeight,
                0, 0, this->frameBitmapInfo.bmiHeader.biWidth, this->frameBitmapInfo.bmiHeader.biHeight,
                this->framePixelBuffer, &this->frameBitmapInfo, DIB_RGB_COLORS, SRCCOPY);

            EndPaint(hWnd, &ps);
            break;
        }
        case WM_SIZE:
        {
            DWORD width = LOWORD(lParam);
            DWORD height = HIWORD(lParam);

            double aspectRatio = double(width) / double(height);
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
        case WM_LBUTTONUP:
        {
            this->lastMouseMove = -1;
            break;
        }
        case WM_MOUSEMOVE:
        {
            if ((wParam & MK_LBUTTON) != 0)
            {
                DWORD mouseMove = lParam;
                
                if (this->lastMouseMove != -1)
                {
                    int currentMouseX = (mouseMove & 0x0000FFFF) >> 0;
                    int currentMouseY = (mouseMove & 0xFFFF0000) >> 16;

                    int lastMouseX = (this->lastMouseMove & 0x0000FFFF) >> 0;
                    int lastMouseY = (this->lastMouseMove & 0xFFFF0000) >> 16;

                    double deltaX = currentMouseX - lastMouseX;
                    double deltaY = currentMouseY - lastMouseY;

                    double mouseSensativity = 0.002;

                    double yawDelta = -deltaX * mouseSensativity;
                    double pitchDelta = -deltaY * mouseSensativity;

                    Frumpy::Vector cameraRight, cameraUp, cameraForward;
                    this->camera->worldTransform.GetAxes(cameraRight, cameraUp, cameraForward);

                    Frumpy::Vector upVector(0.0, 1.0, 0.0);
                    cameraRight.Rotation(cameraRight, upVector, yawDelta);
                    cameraUp.Rotation(cameraUp, upVector, yawDelta);
                    cameraForward.Rotation(cameraForward, upVector, yawDelta);

                    cameraUp.Rotation(cameraUp, cameraRight, pitchDelta);
                    cameraForward.Rotation(cameraForward, cameraRight, pitchDelta);

                    this->camera->worldTransform.SetAxes(cameraRight, cameraUp, cameraForward);
                    this->camera->worldTransform.OrthonormalizeOrientation();
                }

                this->lastMouseMove = mouseMove;
            }

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