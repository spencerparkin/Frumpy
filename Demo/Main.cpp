#include "framework.h"
#include "Demo.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    Demo demo;

    if (!demo.Setup(hInstance, nCmdShow))
        return FALSE;

    demo.Run();

    return demo.Shutdown();
}