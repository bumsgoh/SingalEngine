
#include "framework.h"
#include "SingalEngine.h"

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
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

    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SINGALENGINE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    std::unique_ptr<SingalEngine> app = std::make_unique<SingalEngine>();

    // window 초기화
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

    if (!InitDirect3D())
        return false;


    SetForegroundWindow(m_windowHandle);

    /*
    if (!InitMainWindow())
        return false;

    if (!InitDirect3D())
        return false;

    if (!InitGUI())
        return false;

    if (!InitScene())
        return false;

    // PostEffect에 사용
    m_screenSquare = make_shared<Model>(
        m_device, m_context, vector{ GeometryGenerator::MakeSquare() });

    // 환경 박스 초기화
    MeshData skyboxMesh = GeometryGenerator::MakeBox(40.0f);
    std::reverse(skyboxMesh.indices.begin(), skyboxMesh.indices.end());
    m_skybox = make_shared<Model>(m_device, m_context, vector{ skyboxMesh });
    m_skybox->m_name = "SkyBox";

    // 콘솔창이 렌더링 창을 덮는 것을 방지
    SetForegroundWindow(m_mainWindow);
    */
    return true;
}

bool SingalEngine::InitDirect3D() {

    ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&m_dxgiFactory)));

    // dx12 지원 device 생성
     D3D12CreateDevice(
        nullptr,             // default adapter
        D3D_FEATURE_LEVEL_12_0,
        IID_PPV_ARGS(&m_device));
    
    //GPU 명령 소진전 CPU 동작을 막기위한 펜스 추가
    ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE,
        IID_PPV_ARGS(&m_fence)));

    InitCommandList();
    InitDSHeaps();
    return true;
}

void SingalEngine::InitCommandList() {

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    ThrowIfFailed(m_device->CreateCommandQueue(
        &queueDesc,
        IID_PPV_ARGS(m_commandQueue.GetAddressOf())));
    m_device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(m_commandListAllocator.GetAddressOf()));
    m_device->CreateCommandList(0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        m_commandListAllocator.Get(),
        nullptr,
        IID_PPV_ARGS(&m_commandList));

    m_commandList->Close();

}

//RTV, DSV 등을 저장하고있는 heap 을 생성한다
void SingalEngine::InitDSHeaps() {

    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
    rtvHeapDesc.NumDescriptors = 2;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;
    ThrowIfFailed(m_device->CreateDescriptorHeap(
        &rtvHeapDesc, IID_PPV_ARGS(m_rtvHeap.GetAddressOf())));


    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    dsvHeapDesc.NodeMask = 0;
    ThrowIfFailed(m_device->CreateDescriptorHeap(
        &dsvHeapDesc, IID_PPV_ARGS(m_dsvHeap.GetAddressOf())));

    m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    m_dsvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

//2개의 버퍼를 가지는 swap chain을 형성한다
void SingalEngine::InitSwapChain() {
    m_swapChain.Reset();

    DXGI_SWAP_CHAIN_DESC sd;
    sd.BufferDesc.Width = m_screenWidth;
    sd.BufferDesc.Height = m_screenHeight;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferDesc.Format = m_backBufferFormat;
    sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    //TODO:: msaa 추후에 설정
    //sd.SampleDesc.Count = m4xMsaaState ? 4 : 1;
    //sd.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = 2;
    sd.OutputWindow = m_windowHandle;
    sd.Windowed = true;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    // Note: Swap chain uses queue to perform flush.
    ThrowIfFailed(m_dxgiFactory->CreateSwapChain(
        m_commandQueue.Get(),
        &sd,
        m_swapChain.GetAddressOf()));
}

int SingalEngine::Run() {
    MSG msg = { 0 };
    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            ImGui_ImplDX12_NewFrame();
            ImGui_ImplWin32_NewFrame();

            ImGui::NewFrame();
            ImGui::Begin("Scene Control");

            //Draw, update

            ImGui::Text("Average %.3f ms/frame (%.1f FPS)",
                1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);

            UpdateGUI(); // 추가적으로 사용할 GUI

            ImGui::End();
            ImGui::Render();

            UpdateEnginFrame(ImGui::GetIO().DeltaTime);

            Render();
            SetViewport();

        }
    }
    return 0;
}

SingalEngine::SingalEngine()
    : m_screenWidth(1280),
    m_screenHeight(720),
    m_screenViewport(D3D12_VIEWPORT()) {

    g_engine = this;
}

SingalEngine::~SingalEngine() {
    g_engine = nullptr;

    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

float SingalEngine::GetScreenRatio() const {
    return float(m_screenWidth) / m_screenHeight;
}

void SingalEngine::UpdateEnginFrame(float dt) {


}

void SingalEngine::UpdateGUI() {

}

void SingalEngine::SetViewport() {

    // Set the viewport
    ZeroMemory(&m_screenViewport, sizeof(D3D12_VIEWPORT));
    m_screenViewport.TopLeftX = 0;
    m_screenViewport.TopLeftY = 0;
    m_screenViewport.Width = float(m_screenWidth);
    m_screenViewport.Height = float(m_screenHeight);
    m_screenViewport.MinDepth = 0.0f;
    m_screenViewport.MaxDepth = 1.0f;

    m_commandList->RSSetViewports(1, &m_screenViewport);
}

void SingalEngine::Render() {
    //command 들을 가르키고있는 allocator를 초기화한다
    ThrowIfFailed(m_commandListAllocator->Reset());
    ThrowIfFailed(m_commandList->Reset(m_commandListAllocator.Get(), nullptr));

    D3D12_CPU_DESCRIPTOR_HANDLE dsView = DepthStencilView();
    D3D12_CPU_DESCRIPTOR_HANDLE backbufferView = CurrentBackBufferView();
    ID3D12Resource* curBackbuffer = CurrentBackBuffer();
    D3D12_RESOURCE_BARRIER beforeBarrier = CD3DX12_RESOURCE_BARRIER::Transition(curBackbuffer,
        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    D3D12_RESOURCE_BARRIER afterBarrier = CD3DX12_RESOURCE_BARRIER::Transition(curBackbuffer,
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    //resource: present > renderTarget
    m_commandList->ResourceBarrier(1, &beforeBarrier);
    m_commandList->RSSetViewports(1, &m_screenViewport);
    //m_commandList->RSSetScissorRects(1, &mScissorRect);
    FLOAT color = 1.0f;

    m_commandList->ClearRenderTargetView(backbufferView, &color, 0, nullptr);
    m_commandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    m_commandList->OMSetRenderTargets(1, &backbufferView, true, &dsView);
    //resource: renderTarget > present
    m_commandList->ResourceBarrier(1, &afterBarrier);

    //명령 끝났다면 close
    ThrowIfFailed(m_commandList->Close());

    //close 해놓은 뒤에 명령 리스트 GPU로 전달
    ID3D12CommandList* cmdsLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    //스왑 체인 렌더링
    ThrowIfFailed(m_swapChain->Present(0, 0));
    //backbuffer 인덱스 바꿔줌
    m_currBackBufferIndex = (m_currBackBufferIndex + 1) % 2;

    FlushCommandQueue();
}

ID3D12Resource* SingalEngine::CurrentBackBuffer() const
{
    return m_swapChainBuffer[m_currBackBufferIndex].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE SingalEngine::CurrentBackBufferView() const
{
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(
        m_rtvHeap->GetCPUDescriptorHandleForHeapStart(),
        m_currBackBufferIndex,
        m_rtvDescriptorSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE SingalEngine::DepthStencilView() const
{
    return m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
}

void SingalEngine::FlushCommandQueue()
{
    m_currentFenceValue++;

    // Add an instruction to the command queue to set a new fence point.  Because we 
    // are on the GPU timeline, the new fence point won't be set until the GPU finishes
    // processing all the commands prior to this Signal().
    ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_currentFenceValue));

    // Wait until the GPU has completed commands up to this fence point.
    if (m_fence->GetCompletedValue() < m_currentFenceValue)
    {
        HANDLE eventHandle = CreateEventEx(nullptr, 0, false, EVENT_ALL_ACCESS);

        // Fire event when GPU hits current fence.  
        ThrowIfFailed(m_fence->SetEventOnCompletion(m_currentFenceValue, eventHandle));

        // Wait until the GPU hits current fence event is fired.
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }
}

//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
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
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
bool SingalEngine::InitWindow(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

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
        // 화면 해상도가 바뀌면 SwapChain을 다시 생성
        if (m_swapChain) {

            m_screenWidth = int(LOWORD(lParam));
            m_screenHeight = int(HIWORD(lParam));

            // 윈도우가 Minimize 모드에서는 screenWidth/Height가 0
            if (m_screenWidth && m_screenHeight) {

                std::cout << "Resize SwapChain to " << m_screenWidth << " "
                    << m_screenHeight << std::endl;

                //m_backBufferRTV.Reset();
                m_swapChain->ResizeBuffers(
                    0,                    // 현재 개수 유지
                    (UINT)LOWORD(lParam), // 해상도 변경
                    (UINT)HIWORD(lParam),
                    DXGI_FORMAT_UNKNOWN, // 현재 포맷 유지
                    0);
                //CreateBuffers();
                SetViewport();
            }
        }
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
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    return g_engine->MsgProc(hWnd, message, wParam, lParam);
}

// 정보 대화 상자의 메시지 처리기입니다.
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
