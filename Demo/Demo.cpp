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
#include "LightSources/SpotLight.h"
#include "LightSources/DirectionalLight.h"
#include "LightSources/AmbientLight.h"
#include "ConvexHull.h"
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
    this->frameBuffer = nullptr;
    this->depthBuffer = nullptr;
    this->shadowBuffer = nullptr;
    this->renderer = nullptr;
    this->assetManager = nullptr;
    this->lastMouseMove = -1;
    this->rotateObjects = false;
    this->rotateCamera = false;
    this->rotateLight = false;
    this->objectRotationAngle = 0.0;
    this->cameraRotationAngle = 0.0;
    this->lightRotationAngle = 0.0;
    this->objectRotationRate = 20.0;
    this->cameraRotationRate = -18.0;
    this->lightRotationRate = 15.0;
}

/*virtual*/ Demo::~Demo()
{
}

bool Demo::Setup(HINSTANCE hInstance, int nCmdShow)
{
    this->hInst = hInstance;
    this->exitProgram = false;

    this->assetManager = new Frumpy::AssetManager();
    this->assetManager->LoadAssets("Images/texture.ppm");
    this->assetManager->LoadAssets("Meshes/Teapot.obj");
    this->assetManager->LoadAssets("Meshes/Cube.obj");
    this->assetManager->LoadAssets("Meshes/Torus.obj");
    this->assetManager->LoadAssets("Meshes/GroundPlane.obj");

    Frumpy::ConvexHull convexHull;
    Frumpy::Mesh* mesh = nullptr;

    convexHull.Generate(Frumpy::ConvexHull::Polyhedron::REGULAR_TETRAHEDRON, 10.0);
    mesh = convexHull.Generate();
    strcpy_s(mesh->name, sizeof(mesh->name), "Tetrahedron");
    this->assetManager->AddAsset(mesh);

#if 0
    convexHull.Generate(Frumpy::ConvexHull::Polyhedron::REGULAR_HEXADRON);
    mesh = convexHull.Generate();
    strcpy_s(mesh->name, sizeof(mesh->name), "Hexadron");
    this->assetManager->AddAsset(mesh);
#endif

    this->renderer = new Frumpy::Renderer();

    this->frameDCHandle = CreateCompatibleDC(NULL);

    this->ResizeFramebuffer(512);

    this->shadowBuffer = new Frumpy::Image(512, 512);
    this->renderer->SetShadowBuffer(this->shadowBuffer);

    Frumpy::SpotLight* spotLight = new Frumpy::SpotLight();
    spotLight->worldSpaceLocation.SetComponents(70.0, 100.0, 0.0);
    spotLight->worldSpaceDirection = spotLight->worldSpaceLocation * -1.0;
    spotLight->worldSpaceDirection.Normalize();
    spotLight->innerConeAngle = 5.0;
    spotLight->outerConeAngle = 20.0;
    spotLight->ambientIntensity = 0.1;
    this->renderer->SetLightSource(spotLight);

    this->scene = new Frumpy::Scene();

    this->camera = new Frumpy::Camera();
    this->camera->LookAt(Frumpy::Vector(100.0, 100.0, 0.0), Frumpy::Vector(0.0, 20.0, 0.0), Frumpy::Vector(0.0, 1.0, 0.0));

    Frumpy::MeshObject* object = nullptr;

    object = new Frumpy::MeshObject();
    object->SetMesh(dynamic_cast<Frumpy::Mesh*>(this->assetManager->FindAssetByName("Teapot001")));
    object->GetMesh()->SetColor(Frumpy::Vector(1.0, 0.0, 0.0));
    object->SetTexture(dynamic_cast<Frumpy::Image*>(this->assetManager->FindAssetByName("Images/texture.ppm")));
    object->SetRenderFlag(Frumpy::MeshObject::CASTS_SHADOW, true);
    object->SetRenderFlag(Frumpy::MeshObject::CAN_BE_SHADOWED, false);
    object->SetRenderFlag(Frumpy::MeshObject::VISIBLE, false);
    object->childToParent.SetTranslation(Frumpy::Vector(0.0, 20.0, 0.0));
    strcpy_s(object->name, "teapot");
    this->scene->objectList.AddTail(object);

    object = new Frumpy::MeshObject();
    object->SetMesh(dynamic_cast<Frumpy::Mesh*>(this->assetManager->FindAssetByName("Torus001")));
    object->GetMesh()->SetColor(Frumpy::Vector(0.0, 1.0, 0.0));
    object->SetTexture(dynamic_cast<Frumpy::Image*>(this->assetManager->FindAssetByName("Images/texture.ppm")));
    object->childToParent.SetTranslation(Frumpy::Vector(0.0, 20.0, -10.0));
    object->SetRenderFlag(Frumpy::MeshObject::CASTS_SHADOW, true);
    object->SetRenderFlag(Frumpy::MeshObject::CAN_BE_SHADOWED, false);
    object->SetRenderFlag(Frumpy::MeshObject::VISIBLE, false);
    strcpy_s(object->name, "torus");
    this->scene->objectList.AddTail(object);

    object = new Frumpy::MeshObject();
    object->SetMesh(dynamic_cast<Frumpy::Mesh*>(this->assetManager->FindAssetByName("Box001")));
    object->GetMesh()->SetColor(Frumpy::Vector(0.0, 0.0, 1.0));
    object->SetTexture(dynamic_cast<Frumpy::Image*>(this->assetManager->FindAssetByName("Images/texture.ppm")));
    object->childToParent.SetTranslation(Frumpy::Vector(0.0, 20.0, 10.0));
    object->SetRenderFlag(Frumpy::MeshObject::CASTS_SHADOW, true);
    object->SetRenderFlag(Frumpy::MeshObject::CAN_BE_SHADOWED, false);
    object->SetRenderFlag(Frumpy::MeshObject::VISIBLE, false);
    strcpy_s(object->name, "cube");
    this->scene->objectList.AddTail(object);

    object = new Frumpy::MeshObject();
    object->SetMesh(dynamic_cast<Frumpy::Mesh*>(this->assetManager->FindAssetByName("Plane001")));
    object->GetMesh()->SetColor(Frumpy::Vector(1.0, 1.0, 1.0));
    object->SetRenderFlag(Frumpy::MeshObject::CASTS_SHADOW, false);
    object->SetRenderFlag(Frumpy::MeshObject::CAN_BE_SHADOWED, true);
    object->SetRenderFlag(Frumpy::MeshObject::VISIBLE, true);
    strcpy_s(object->name, "ground_plane");
    this->scene->objectList.AddTail(object);

    object = new Frumpy::MeshObject();
    object->SetMesh(dynamic_cast<Frumpy::Mesh*>(this->assetManager->FindAssetByName("Tetrahedron")));
    object->GetMesh()->SetColor(Frumpy::Vector(1.0, 1.0, 0.0));
    object->SetRenderFlag(Frumpy::MeshObject::CASTS_SHADOW, true);
    object->SetRenderFlag(Frumpy::MeshObject::CAN_BE_SHADOWED, false);
    object->SetRenderFlag(Frumpy::MeshObject::VISIBLE, true);
    object->childToParent.SetTranslation(Frumpy::Vector(0.0, 20.0, 0.0));
    strcpy_s(object->name, "tetrahedron");
    this->scene->objectList.AddTail(object);

    this->renderer->Startup(8);

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

        // Rotate the lright if configured to do so.
        if (this->rotateLight)
        {
            Frumpy::SpotLight* spotLight = dynamic_cast<Frumpy::SpotLight*>(this->renderer->GetLightSource());
            if (spotLight)
            {
                this->lightRotationAngle += this->lightRotationRate * deltaTimeSeconds;
                static double radius = 70.0;
                spotLight->worldSpaceLocation.SetComponents(radius * cos(FRUMPY_DEGS_TO_RADS(this->lightRotationAngle)), 100.0, radius * sin(FRUMPY_DEGS_TO_RADS(this->lightRotationAngle)));
                spotLight->worldSpaceDirection = spotLight->worldSpaceLocation * -1.0;
                spotLight->worldSpaceDirection.Normalize();
            }

            Frumpy::DirectionalLight* directionalLight = dynamic_cast<Frumpy::DirectionalLight*>(this->renderer->GetLightSource());
            if (directionalLight)
            {
                //...
            }
        }

        // Rotate the objects around if configured to do so.
        if (this->rotateObjects)
        {
            this->objectRotationAngle += this->objectRotationRate * deltaTimeSeconds;
            int i = 0;
            this->scene->ForAllObjects([=, &i](Frumpy::Scene::Object* object) -> bool {
                if (0 != strcmp(object->name, "ground_plane"))
                {
                    Frumpy::Vector translation;
                    Frumpy::Vector axis;
                    if (i == 0)
                        axis.SetComponents(1.0, 0.0, 0.0);
                    else if (i == 1)
                        axis.SetComponents(0.0, 1.0, 0.0);
                    else
                        axis.SetComponents(0.0, 0.0, 1.0);
                    i++;
                    object->childToParent.GetCol(3, translation);
                    object->childToParent.RigidBodyMotion(axis, FRUMPY_DEGS_TO_RADS(this->objectRotationAngle), translation);
                }
                return true;
            });
        }

        // Rotate the camera if configured to do so.
        if (this->rotateCamera)
        {
            this->cameraRotationAngle += this->cameraRotationRate * deltaTimeSeconds;
            static double radius = 100.0;
            Frumpy::Vector upDirection(0.0, 1.0, 0.0);
            Frumpy::Vector eyePoint(radius * cos(FRUMPY_DEGS_TO_RADS(this->cameraRotationAngle)), 100.0, radius * sin(FRUMPY_DEGS_TO_RADS(this->cameraRotationAngle)));
            Frumpy::Vector eyeTarget(0.0, 0.0, 0.0);
            this->camera->LookAt(eyePoint, eyeTarget, upDirection);
        }
        else
        {
            // Let the user control the camera.
            this->HandleKeyboardInput(deltaTimeSeconds);
        }

        static bool debug = false;
        if (debug)
        {
            this->shadowBuffer->ConvertDepthToGreyScale(10000.0);
            this->assetManager->SaveAsset("shadow_buffer.ppm", this->shadowBuffer);
        }

        // Render a frame directly into the windows BMP memory.
        if (this->renderer && this->camera && this->scene)
        {
            ProfileBlock profileBlock(&this->frumpyRenderTime);
            this->renderer->RenderScene(this->scene, this->camera);
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
            sprintf_s(fpsMessage, sizeof(fpsMessage), "Image Size: %d x %d; FPS: %2.4f; Render: %2.4f; Blit: %2.4f; Msg: %2.4f; Jobs: %d",
                this->frameBuffer->GetWidth(),
                this->frameBuffer->GetHeight(),
                fps,
                this->frumpyRenderTime.GetAverageMilliseconds(),
                this->demoBlitTime.GetAverageMilliseconds(),
                this->demoMessageTime.GetAverageMilliseconds(),
                this->renderer->totalRenderJobsPerformedPerFrame);
            SendMessage(this->hWndStatusBar, SB_SETTEXT, 0, (LPARAM)fpsMessage);
            totalElapsedTimeSeconds = 0.0;
        }
    }
}

int Demo::Shutdown()
{
    this->ResizeFramebuffer(0);

    if (this->renderer)
    {
        delete this->renderer->GetLightSource();
        this->renderer->SetLightSource(nullptr);
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

    if (this->shadowBuffer)
    {
        delete this->shadowBuffer;
        this->shadowBuffer = nullptr;
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
    if (this->hWnd == GetFocus())
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
}

/*static*/ LRESULT CALLBACK Demo::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    Demo* demo = (Demo*)GetWindowLongPtr(hWnd, 0);
    if (demo)
        return demo->HandleMessage(hWnd, message, wParam, lParam);

    return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT Demo::HandleCommandMessage(WPARAM wParam, LPARAM lParam)
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
            DestroyWindow(this->hWnd);
            break;
        }
        case ID_SCENE_TEAPOT:
        {
            Frumpy::Scene::Object* object = this->scene->FindObjectByName("teapot");
            if (object)
                object->SetRenderFlag(Frumpy::Scene::Object::VISIBLE, !object->GetRenderFlag(Frumpy::Scene::Object::VISIBLE));
            break;
        }
        case ID_SCENE_CUBE:
        {
            Frumpy::Scene::Object* object = this->scene->FindObjectByName("cube");
            if (object)
                object->SetRenderFlag(Frumpy::Scene::Object::VISIBLE, !object->GetRenderFlag(Frumpy::Scene::Object::VISIBLE));
            break;
        }
        case ID_SCENE_TORUS:
        {
            Frumpy::Scene::Object* object = this->scene->FindObjectByName("torus");
            if (object)
                object->SetRenderFlag(Frumpy::Scene::Object::VISIBLE, !object->GetRenderFlag(Frumpy::Scene::Object::VISIBLE));
            break;
        }
        case ID_SCENE_TETRAHEDRON:
        {
            Frumpy::Scene::Object* object = this->scene->FindObjectByName("tetrahedron");
            if (object)
                object->SetRenderFlag(Frumpy::Scene::Object::VISIBLE, !object->GetRenderFlag(Frumpy::Scene::Object::VISIBLE));
            break;
        }
        case ID_LIGHTING_CASTSHADOWS:
        {
            Frumpy::Scene::Object* torusObject = this->scene->FindObjectByName("torus");
            if (torusObject)
            {
                bool castsShadow = !torusObject->GetRenderFlag(Frumpy::Scene::Object::CASTS_SHADOW);
                this->scene->ForAllObjects([=](Frumpy::Scene::Object* object) -> bool {
                    if (0 != strcmp(object->name, "ground_plane"))
                        object->SetRenderFlag(Frumpy::Scene::Object::CASTS_SHADOW, castsShadow);
                    return true;
                });

                this->renderer->SetShadowBuffer(castsShadow ? this->shadowBuffer : nullptr);
            }

            break;
        }
        case ID_LIGHTING_SELF_SHADOW:
        {
            Frumpy::Scene::Object* torusObject = this->scene->FindObjectByName("torus");
            if (torusObject)
            {
                bool canBeShadowed = !torusObject->GetRenderFlag(Frumpy::Scene::Object::CAN_BE_SHADOWED);
                this->scene->ForAllObjects([=](Frumpy::Scene::Object* object) -> bool {
                    if (0 != strcmp(object->name, "ground_plane"))
                        object->SetRenderFlag(Frumpy::Scene::Object::CAN_BE_SHADOWED, canBeShadowed);
                    return true;
                });
            }

            break;
        }
        case ID_ANIMATION_ROTATECAMERA:
        {
            this->rotateCamera = !this->rotateCamera;
            break;
        }
        case ID_ANIMATION_ROTATELIGHT:
        {
            this->rotateLight = !this->rotateLight;
            break;
        }
        case ID_ANIMATION_ROTATEOBJECT:
        {
            this->rotateObjects = !this->rotateObjects;
            break;
        }
        case ID_FRAMEBUFFER_128X128:
        {
            this->ResizeFramebuffer(128);
            break;
        }
        case ID_FRAMEBUFFER_256X256:
        {
            this->ResizeFramebuffer(256);
            break;
        }
        case ID_FRAMEBUFFER_512X512:
        {
            this->ResizeFramebuffer(512);
            break;
        }
        case ID_FRAMEBUFFER_1024X1024:
        {
            this->ResizeFramebuffer(1024);
            break;
        }
        case ID_SHADOWBUFFER_128X128:
        {
            this->shadowBuffer->SetWidthAndHeight(128, 128);
            break;
        }
        case ID_SHADOWBUFFER_256X256:
        {
            this->shadowBuffer->SetWidthAndHeight(256, 256);
            break;
        }
        case ID_SHADOWBUFFER_512X512:
        {
            this->shadowBuffer->SetWidthAndHeight(512, 512);
            break;
        }
        case ID_SHADOWBUFFER_1024X1024:
        {
            this->shadowBuffer->SetWidthAndHeight(1024, 1024);
            break;
        }
        case ID_LIGHTING_AMBIENTLIGHTONLY:
        {
            Frumpy::AmbientLight* ambientLight = dynamic_cast<Frumpy::AmbientLight*>(this->renderer->GetLightSource());
            if (!ambientLight)
            {
                ambientLight = new Frumpy::AmbientLight();
                ambientLight->ambientIntensity = 0.6;
                delete this->renderer->GetLightSource();
                this->renderer->SetLightSource(ambientLight);
            }
            break;
        }
        case ID_LIGHTING_DIRECTIONALLIGHT:
        {
            Frumpy::DirectionalLight* directionalLight = dynamic_cast<Frumpy::DirectionalLight*>(this->renderer->GetLightSource());
            if (!directionalLight)
            {
                directionalLight = new Frumpy::DirectionalLight();
                directionalLight->worldSpaceDirection.SetComponents(-0.5, -1.0, 0.0);
                directionalLight->worldSpaceDirection.Normalize();
                directionalLight->ambientIntensity = 0.1;
                delete this->renderer->GetLightSource();
                this->renderer->SetLightSource(directionalLight);
            }
            break;
        }
        case ID_LIGHTING_SPOTLIGHT:
        {
            Frumpy::SpotLight* spotLight = dynamic_cast<Frumpy::SpotLight*>(this->renderer->GetLightSource());
            if (!spotLight)
            {
                spotLight = new Frumpy::SpotLight();
                spotLight->worldSpaceLocation.SetComponents(70.0, 100.0, 0.0);
                spotLight->worldSpaceDirection = spotLight->worldSpaceLocation * -1.0;
                spotLight->worldSpaceDirection.Normalize();
                spotLight->innerConeAngle = 5.0;
                spotLight->outerConeAngle = 20.0;
                spotLight->ambientIntensity = 0.1;
                delete this->renderer->GetLightSource();
                this->renderer->SetLightSource(spotLight);
            }
            break;
        }
        case ID_MULTITHREADING_1_THREAD:
        {
            if (this->renderer->GetThreadCount() != 1)
            {
                this->renderer->Shutdown();
                this->renderer->Startup(1);
            }
            break;
        }
        case ID_MULTITHREADING_8_THREADS:
        {
            if (this->renderer->GetThreadCount() != 8)
            {
                this->renderer->Shutdown();
                this->renderer->Startup(8);
            }
            break;
        }
        case ID_MULTITHREADING_16_THREADS:
        {
            if (this->renderer->GetThreadCount() != 16)
            {
                this->renderer->Shutdown();
                this->renderer->Startup(16);
            }
            break;
        }
        case ID_TEXTURING_NEARESTFILTERING:
        {
            Frumpy::MeshObject* object = dynamic_cast<Frumpy::MeshObject*>(this->scene->FindObjectByName("teapot"));
            if (object)
                object->SetSampleMethod(Frumpy::Image::NEAREST);
            break;
        }
        case ID_TEXTURING_BILINEARFILTERING:
        {
            Frumpy::MeshObject* object = dynamic_cast<Frumpy::MeshObject*>(this->scene->FindObjectByName("teapot"));
            if (object)
                object->SetSampleMethod(Frumpy::Image::BILINEAR);
            break;
        }
        case ID_TEXTURING_USETEXTURE:
        {
            this->scene->ForAllObjects([=](Frumpy::Scene::Object* object) -> bool {
                if (0 != strcmp(object->name, "ground_plane"))
                {
                    Frumpy::MeshObject* meshObject = dynamic_cast<Frumpy::MeshObject*>(object);
                    if (meshObject)
                        meshObject->SetTexture(dynamic_cast<Frumpy::Image*>(this->assetManager->FindAssetByName("Images/texture.ppm")));
                }
                return true;
            });
            break;
        }
        case ID_TEXTURING_USEVERTEXCOLORS:
        {
            this->scene->ForAllObjects([=](Frumpy::Scene::Object* object) -> bool {
                if (0 != strcmp(object->name, "ground_plane"))
                {
                    Frumpy::MeshObject* meshObject = dynamic_cast<Frumpy::MeshObject*>(object);
                    if (meshObject)
                        meshObject->SetTexture(nullptr);
                }
                return true;
            });
            break;
        }
        default:
        {
            return DefWindowProc(this->hWnd, WM_COMMAND, wParam, lParam);
        }
    }

    return 0;
}

void Demo::ResizeFramebuffer(unsigned int newSize)
{
    if (this->frameBitmapHandle != NULL)
    {
        DeleteObject(this->frameBitmapHandle);
        this->frameBitmapHandle = NULL;
        this->framePixelBuffer = NULL;
    }

    if (this->frameBuffer)
    {
        delete this->frameBuffer;
        this->frameBuffer = nullptr;
    }

    if (this->depthBuffer)
    {
        delete this->depthBuffer;
        this->depthBuffer = nullptr;
    }

    if (newSize > 0)
    {
        this->frameBitmapInfo.bmiHeader.biSize = sizeof(frameBitmapInfo.bmiHeader);
        this->frameBitmapInfo.bmiHeader.biPlanes = 1;
        this->frameBitmapInfo.bmiHeader.biBitCount = 32;
        this->frameBitmapInfo.bmiHeader.biCompression = BI_RGB;
        this->frameBitmapInfo.bmiHeader.biWidth = newSize;
        this->frameBitmapInfo.bmiHeader.biHeight = newSize;

        this->frameBitmapHandle = CreateDIBSection(NULL, &this->frameBitmapInfo, DIB_RGB_COLORS, (void**)&this->framePixelBuffer, NULL, 0);
        
        SelectObject(this->frameDCHandle, this->frameBitmapHandle);

        Frumpy::Image::Format format;
        format.bShift = 0;
        format.gShift = 8;
        format.rShift = 16;
        format.aShift = 24;

        this->frameBuffer = new Frumpy::Image();
        this->frameBuffer->SetRawPixelBuffer(this->framePixelBuffer, this->frameBitmapInfo.bmiHeader.biWidth, this->frameBitmapInfo.bmiHeader.biHeight);
        this->frameBuffer->SetFormat(format);

        this->depthBuffer = new Frumpy::Image(this->frameBuffer->GetWidth(), this->frameBuffer->GetHeight());
    }

    this->renderer->SetFramebuffer(this->frameBuffer);
    this->renderer->SetDepthBuffer(this->depthBuffer);
}

LRESULT Demo::HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_COMMAND:
        {
            return this->HandleCommandMessage(wParam, lParam);
        }
        case WM_MENUSELECT:
        {
            if (LOWORD(wParam) == 0x0001 /* options menu */)
            {
                HMENU optionsMenuHandle = (HMENU)lParam;
                this->UpdateOptionsMenuItemChecks(optionsMenuHandle);
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

void Demo::UpdateOptionsMenuItemChecks(HMENU menuHandle)
{
    int menuItemCount = GetMenuItemCount(menuHandle);
    for (int i = 0; i < menuItemCount; i++)
    {
        UINT menuItemID = GetMenuItemID(menuHandle, i);

        switch (menuItemID)
        {
            case ID_SCENE_TEAPOT:
            {
                bool checked = false;
                Frumpy::Scene::Object* object = this->scene->FindObjectByName("teapot");
                if (object)
                    checked = object->GetRenderFlag(Frumpy::Scene::Object::VISIBLE);
                CheckMenuItem(menuHandle, menuItemID, (checked ? MF_CHECKED : MF_UNCHECKED));
                break;
            }
            case ID_SCENE_CUBE:
            {
                bool checked = false;
                Frumpy::Scene::Object* object = this->scene->FindObjectByName("cube");
                if (object)
                    checked = object->GetRenderFlag(Frumpy::Scene::Object::VISIBLE);
                CheckMenuItem(menuHandle, menuItemID, (checked ? MF_CHECKED : MF_UNCHECKED));
                break;
            }
            case ID_SCENE_TORUS:
            {
                bool checked = false;
                Frumpy::Scene::Object* object = this->scene->FindObjectByName("torus");
                if (object)
                    checked = object->GetRenderFlag(Frumpy::Scene::Object::VISIBLE);
                CheckMenuItem(menuHandle, menuItemID, (checked ? MF_CHECKED : MF_UNCHECKED));
                break;
            }
            case ID_SCENE_TETRAHEDRON:
            {
                bool checked = false;
                Frumpy::Scene::Object* object = this->scene->FindObjectByName("tetrahedron");
                if (object)
                    checked = object->GetRenderFlag(Frumpy::Scene::Object::VISIBLE);
                CheckMenuItem(menuHandle, menuItemID, (checked ? MF_CHECKED : MF_UNCHECKED));
                break;
            }
            case ID_LIGHTING_CASTSHADOWS:
            {
                bool checked = false;
                Frumpy::Scene::Object* object = this->scene->FindObjectByName("teapot");
                if (object)
                    checked = object->GetRenderFlag(Frumpy::Scene::Object::CASTS_SHADOW);
                CheckMenuItem(menuHandle, menuItemID, (checked ? MF_CHECKED : MF_UNCHECKED));
                break;
            }
            case ID_LIGHTING_SELF_SHADOW:
            {
                bool checked = false;
                Frumpy::Scene::Object* object = this->scene->FindObjectByName("teapot");
                if (object)
                    checked = object->GetRenderFlag(Frumpy::Scene::Object::CAN_BE_SHADOWED);
                CheckMenuItem(menuHandle, menuItemID, (checked ? MF_CHECKED : MF_UNCHECKED));
                break;
            }
            case ID_ANIMATION_ROTATECAMERA:
            {
                CheckMenuItem(menuHandle, menuItemID, (this->rotateCamera ? MF_CHECKED : MF_UNCHECKED));
                break;
            }
            case ID_ANIMATION_ROTATELIGHT:
            {
                CheckMenuItem(menuHandle, menuItemID, (this->rotateLight ? MF_CHECKED : MF_UNCHECKED));
                break;
            }
            case ID_ANIMATION_ROTATEOBJECT:
            {
                CheckMenuItem(menuHandle, menuItemID, (this->rotateObjects ? MF_CHECKED : MF_UNCHECKED));
                break;
            }
            case ID_FRAMEBUFFER_128X128:
            {
                CheckMenuItem(menuHandle, menuItemID, (this->frameBuffer->GetWidth() == 128 ? MF_CHECKED : MF_UNCHECKED));
                break;
            }
            case ID_FRAMEBUFFER_256X256:
            {
                CheckMenuItem(menuHandle, menuItemID, (this->frameBuffer->GetWidth() == 256 ? MF_CHECKED : MF_UNCHECKED));
                break;
            }
            case ID_FRAMEBUFFER_512X512:
            {
                CheckMenuItem(menuHandle, menuItemID, (this->frameBuffer->GetWidth() == 512 ? MF_CHECKED : MF_UNCHECKED));
                break;
            }
            case ID_FRAMEBUFFER_1024X1024:
            {
                CheckMenuItem(menuHandle, menuItemID, (this->frameBuffer->GetWidth() == 1024 ? MF_CHECKED : MF_UNCHECKED));
                break;
            }
            case ID_SHADOWBUFFER_128X128:
            {
                CheckMenuItem(menuHandle, menuItemID, (this->shadowBuffer->GetWidth() == 128 ? MF_CHECKED : MF_UNCHECKED));
                break;
            }
            case ID_SHADOWBUFFER_256X256:
            {
                CheckMenuItem(menuHandle, menuItemID, (this->shadowBuffer->GetWidth() == 256 ? MF_CHECKED : MF_UNCHECKED));
                break;
            }
            case ID_SHADOWBUFFER_512X512:
            {
                CheckMenuItem(menuHandle, menuItemID, (this->shadowBuffer->GetWidth() == 512 ? MF_CHECKED : MF_UNCHECKED));
                break;
            }
            case ID_SHADOWBUFFER_1024X1024:
            {
                CheckMenuItem(menuHandle, menuItemID, (this->shadowBuffer->GetWidth() == 1024 ? MF_CHECKED : MF_UNCHECKED));
                break;
            }
            case ID_LIGHTING_AMBIENTLIGHTONLY:
            {
                CheckMenuItem(menuHandle, menuItemID, (dynamic_cast<Frumpy::AmbientLight*>(this->renderer->GetLightSource()) ? MF_CHECKED : MF_UNCHECKED));
                break;
            }
            case ID_LIGHTING_DIRECTIONALLIGHT:
            {
                CheckMenuItem(menuHandle, menuItemID, (dynamic_cast<Frumpy::DirectionalLight*>(this->renderer->GetLightSource()) ? MF_CHECKED : MF_UNCHECKED));
                break;
            }
            case ID_LIGHTING_SPOTLIGHT:
            {
                CheckMenuItem(menuHandle, menuItemID, (dynamic_cast<Frumpy::SpotLight*>(this->renderer->GetLightSource()) ? MF_CHECKED : MF_UNCHECKED));
                break;
            }
            case ID_MULTITHREADING_1_THREAD:
            {
                CheckMenuItem(menuHandle, menuItemID, (this->renderer->GetThreadCount() == 1) ? MF_CHECKED : MF_UNCHECKED);
                break;
            }
            case ID_MULTITHREADING_8_THREADS:
            {
                CheckMenuItem(menuHandle, menuItemID, (this->renderer->GetThreadCount() == 8) ? MF_CHECKED : MF_UNCHECKED);
                break;
            }
            case ID_MULTITHREADING_16_THREADS:
            {
                CheckMenuItem(menuHandle, menuItemID, (this->renderer->GetThreadCount() == 16) ? MF_CHECKED : MF_UNCHECKED);
                break;
            }
            case ID_TEXTURING_NEARESTFILTERING:
            {
                bool checked = false;
                Frumpy::MeshObject* object = dynamic_cast<Frumpy::MeshObject*>(this->scene->FindObjectByName("teapot"));
                if (object)
                    checked = (object->GetSampleMethod() == Frumpy::Image::NEAREST) ? MF_CHECKED : MF_UNCHECKED;
                CheckMenuItem(menuHandle, menuItemID, (checked ? MF_CHECKED : MF_UNCHECKED));
                break;
            }
            case ID_TEXTURING_BILINEARFILTERING:
            {
                bool checked = false;
                Frumpy::MeshObject* object = dynamic_cast<Frumpy::MeshObject*>(this->scene->FindObjectByName("teapot"));
                if (object)
                    checked = (object->GetSampleMethod() == Frumpy::Image::BILINEAR) ? MF_CHECKED : MF_UNCHECKED;
                CheckMenuItem(menuHandle, menuItemID, (checked ? MF_CHECKED : MF_UNCHECKED));
                break;
            }
            case ID_TEXTURING_USETEXTURE:
            {
                bool checked = false;
                Frumpy::MeshObject* object = dynamic_cast<Frumpy::MeshObject*>(this->scene->FindObjectByName("teapot"));
                if (object)
                    checked = object->GetTexture() ? true : false;
                CheckMenuItem(menuHandle, menuItemID, (checked ? MF_CHECKED : MF_UNCHECKED));
                break;
            }
            case ID_TEXTURING_USEVERTEXCOLORS:
            {
                bool checked = false;
                Frumpy::MeshObject* object = dynamic_cast<Frumpy::MeshObject*>(this->scene->FindObjectByName("teapot"));
                if (object)
                    checked = object->GetTexture() ? false : true;
                CheckMenuItem(menuHandle, menuItemID, (checked ? MF_CHECKED : MF_UNCHECKED));
                break;
            }
            default:
            {
                HMENU subMenuHandle = GetSubMenu(menuHandle, i);
                if (subMenuHandle != NULL)
                {
                    this->UpdateOptionsMenuItemChecks(subMenuHandle);
                }
                break;
            }
        }
    }
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