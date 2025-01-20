
#include "framework.h"
#include "SingalEngine.h"
#include <DirectXColors.h>

#define MAX_LOADSTRING 100
using namespace DirectX;
// ���� ����:
HINSTANCE hInst;                                // ���� �ν��Ͻ��Դϴ�.
WCHAR szTitle[MAX_LOADSTRING];                  // ���� ǥ���� �ؽ�Ʈ�Դϴ�.
WCHAR szWindowClass[MAX_LOADSTRING];            // �⺻ â Ŭ���� �̸��Դϴ�.

// �� �ڵ� ��⿡ ���Ե� �Լ��� ������ �����մϴ�:
ATOM                MyRegisterClass(HINSTANCE hInstance);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

SingalEngine* g_engine = nullptr;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: ���⿡ �ڵ带 �Է��մϴ�.

    // ���� ���ڿ��� �ʱ�ȭ�մϴ�.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SINGALENGINE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    std::unique_ptr<SingalEngine> app = std::make_unique<SingalEngine>();

    // window �ʱ�ȭ
    if (!app->InitWindow(hInstance, nCmdShow))
    {
        return FALSE;
    }

    if (!app->Initialize()) {
        return -1;
    }

    return app->Run();
}

bool SingalEngine::Initialize() {

    if (!m_renderer->InitDirect3D(m_windowHandle, *m_scene))
        return false;


    SetForegroundWindow(m_windowHandle);
    return true;
}

int SingalEngine::Run() {
    MSG msg = { 0 };
    m_timer.Reset();

    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            m_timer.Tick();
            UpdateGUI(); // �߰������� ����� GUI

            Update(m_timer.DeltaTime());
            Render(m_timer.DeltaTime());

        }
    }
    return 0;
}

SingalEngine::SingalEngine() {
    g_engine = this;
    m_renderer = std::make_shared<Renderer>(1280, 720);
    m_scene = std::make_shared<Scene>(720.f / 1280.0f);
}

SingalEngine::~SingalEngine() {
    g_engine = nullptr;
    m_scene = nullptr;
    m_renderer = nullptr;
}

void SingalEngine::UpdateEnginFrame(float dt) {

}

void SingalEngine::UpdateGUI() {

}

void SingalEngine::Update(float DeltaTime) {
    m_renderer->Update(*m_scene, DeltaTime);
}

void SingalEngine::Render(float DeltaTime) {

    m_renderer->Render(*m_scene, DeltaTime);
}

void SingalEngine::DidResizeWindow()
{
    m_renderer->DidResizeWindow();
}

//
//  �Լ�: MyRegisterClass()
//
//  �뵵: â Ŭ������ ����մϴ�.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SINGALENGINE));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_SINGALENGINE);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   �Լ�: InitInstance(HINSTANCE, int)
//
//   �뵵: �ν��Ͻ� �ڵ��� �����ϰ� �� â�� ����ϴ�.
//
//   �ּ�:
//
//        �� �Լ��� ���� �ν��Ͻ� �ڵ��� ���� ������ �����ϰ�
//        �� ���α׷� â�� ���� ���� ǥ���մϴ�.
//
bool SingalEngine::InitWindow(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // �ν��Ͻ� �ڵ��� ���� ������ �����մϴ�.

    m_windowHandle = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!m_windowHandle)
    {
        return FALSE;
    }

    ShowWindow(m_windowHandle, nCmdShow);
    UpdateWindow(m_windowHandle);

    return TRUE;
}



LRESULT SingalEngine::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    switch (msg) {
    case WM_SIZE:
        // ȭ�� �ػ󵵰� �ٲ�� SwapChain�� �ٽ� ����
        /*if (m_swapChain) {

            m_screenWidth = int(LOWORD(lParam));
            m_screenHeight = int(HIWORD(lParam));

          
        }*/
        break;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_MOUSEMOVE:
        break;
    case WM_LBUTTONDOWN:
        break;
    case WM_LBUTTONUP:
        break;
    case WM_RBUTTONDOWN:
        break;
    case WM_RBUTTONUP:
        break;
    case WM_KEYDOWN:
        break;
    case WM_KEYUP:
        break;
    case WM_MOUSEWHEEL:
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }

    return ::DefWindowProc(hwnd, msg, wParam, lParam);
}


//
//  �Լ�: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  �뵵: �� â�� �޽����� ó���մϴ�.
//
//  WM_COMMAND  - ���ø����̼� �޴��� ó���մϴ�.
//  WM_PAINT    - �� â�� �׸��ϴ�.
//  WM_DESTROY  - ���� �޽����� �Խ��ϰ� ��ȯ�մϴ�.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    return g_engine->MsgProc(hWnd, message, wParam, lParam);
}

// ���� ��ȭ ������ �޽��� ó�����Դϴ�.
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
