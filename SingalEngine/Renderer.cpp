#include "Renderer.h"
#include "Scene.h"
#include "Utils.h"
#include <DirectXColors.h>
#include "Model.h"
Renderer::Renderer(int screenWidth,
                   int screenHeight)
{
    this->m_screenWidth = screenWidth;
    this->m_screenHeight = screenHeight;
    this->m_scissorRect = { 0, 0, screenWidth, screenHeight };
   
    //m_dsvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

   
}

void Renderer::Initialize(Scene& scene)
{
	scene.Initialize(m_device, m_commandList);
}

bool Renderer::InitDirect3D(HWND windowHandle, Scene& scene) {

    #if defined(DEBUG) || defined(_DEBUG) 
        {
            ComPtr<ID3D12Debug> debugController;
            ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
            debugController->EnableDebugLayer();
        }
    #endif

    ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&m_dxgiFactory)));

    // dx12 지원 device 생성
    HRESULT hardwareResult = D3D12CreateDevice(
        nullptr,             // default adapter
        D3D_FEATURE_LEVEL_12_0,
        IID_PPV_ARGS(&m_device));
    //GPU 명령 소진전 CPU 동작을 막기위한 펜스 추가
    ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE,
        IID_PPV_ARGS(&m_fence)));


    D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
    msQualityLevels.Format = m_backBufferFormat;
    msQualityLevels.SampleCount = 4;
    msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
    msQualityLevels.NumQualityLevels = 0;
    ThrowIfFailed(m_device->CheckFeatureSupport(
        D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
        &msQualityLevels,
        sizeof(msQualityLevels)));
    m_4xMsaaQuality = msQualityLevels.NumQualityLevels;

    m_rtvDescriptorSize =m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    InitCommandList();
    InitSwapChain(windowHandle, m_dxgiFactory, m_commandQueue);
    InitDSHeaps();

    DidResizeWindow();

    ThrowIfFailed(m_commandList->Reset(m_commandListAllocator.Get(), nullptr));

    BuildDescriptorHeaps(); //cbv heap 생성
    BuildConstantBuffers(); //cbv heap 사용
    BuildRootSignature(); //root signature 생성
    BuildShadersAndInputLayout(); // shader, inputLayout 생성
    Initialize(scene); //GPU 버퍼 생성
    BuildDefaultPSO(); // inputLayout, root signature 사용

    ThrowIfFailed(m_commandList->Close());
    ID3D12CommandList* cmdsLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    FlushCommandQueue();
    return true;
}

void Renderer::Update(Scene& scene, float DeltaTime) {
    scene.Update(DeltaTime);

    // Has the GPU finished processing the commands of the current frame resource?
    // If not, wait until the GPU has completed commands up to this fence point.
    if (scene.m_currentResource->Fence != 0 && m_fence->GetCompletedValue() < scene.m_currentResource->Fence)
    {
        HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
        ThrowIfFailed(m_fence->SetEventOnCompletion(scene.m_currentResource->Fence, eventHandle));
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }

    scene.UpdateConstantBuffers(DeltaTime);
}

void Renderer::Render(Scene& scene, float DeltaTime)
{
    ComPtr<ID3D12CommandAllocator> CurrentCommandListAlloc = scene.m_currentResource->CommandListAlloc;
    ThrowIfFailed(CurrentCommandListAlloc->Reset());
    ThrowIfFailed(m_commandList->Reset(CurrentCommandListAlloc.Get(), m_pso.Get()));
    
    SetViewport();

    m_commandList->ResourceBarrier(1,
        &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
            D3D12_RESOURCE_STATE_PRESENT,
            D3D12_RESOURCE_STATE_RENDER_TARGET));

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_currBackBufferIndex, m_rtvDescriptorSize);
    m_commandList->ClearRenderTargetView(rtvHandle, Colors::Black, 0, nullptr);
    //m_commandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    m_commandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, nullptr);
    m_commandList->SetPipelineState(m_pso.Get());
    
    ID3D12DescriptorHeap* descriptorHeaps[] = { m_cbvHeap.Get() };
    m_commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
    m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
    scene.Render(m_commandList);

    //m_commandList->SetGraphicsRootDescriptorTable(0, m_cbvHeap->GetGPUDescriptorHandleForHeapStart());
    m_commandList->ResourceBarrier(1,
        &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_PRESENT));

    HRESULT hr;
    hr = m_commandList->Close();
    std::cout << hr;;
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to close command list.");
    }

    //close 해놓은 뒤에 명령 리스트 GPU로 전달
    ID3D12CommandList* cmdsLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    PresentCurrentBuffer();
    FlushCommandQueue();
   
}

void Renderer::PresentCurrentBuffer()
{
    ThrowIfFailed(m_swapChain->Present(0, 0));
    //backbuffer 인덱스 바꿔줌
    m_currBackBufferIndex = (m_currBackBufferIndex + 1) % 2;
}

void Renderer::SetViewport() {

    // Set the viewport
    ZeroMemory(&m_screenViewport, sizeof(D3D12_VIEWPORT));
    m_screenViewport.TopLeftX = 0;
    m_screenViewport.TopLeftY = 0;
    m_screenViewport.Width = float(m_screenWidth);
    m_screenViewport.Height = float(m_screenHeight);
    m_screenViewport.MinDepth = 0.0f;
    m_screenViewport.MaxDepth = 1.0f;

    m_commandList->RSSetViewports(1, &m_screenViewport);
    m_commandList->RSSetScissorRects(1, &m_scissorRect);
}

float Renderer::GetScreenRatio() const {
    return float(m_screenWidth) / m_screenHeight;
}



void Renderer::FlushCommandQueue()
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

//RTV, DSV 등을 저장하고있는 heap 을 생성한다
void Renderer::InitDSHeaps() {

    //Render Target
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
    rtvHeapDesc.NumDescriptors = 2;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;
    ThrowIfFailed(m_device->CreateDescriptorHeap(
        &rtvHeapDesc, IID_PPV_ARGS(m_rtvHeap.GetAddressOf())));

    //Depth Stencil
    /*D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    dsvHeapDesc.NodeMask = 0;
    ThrowIfFailed(m_device->CreateDescriptorHeap(
        &dsvHeapDesc, IID_PPV_ARGS(m_dsvHeap.GetAddressOf())));*/
}



void Renderer::InitCommandList() {

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

    ThrowIfFailed(m_device->CreateCommandQueue(
        &queueDesc,
        IID_PPV_ARGS(&m_commandQueue)));

    m_device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(m_commandListAllocator.GetAddressOf()));

    m_device->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        m_commandListAllocator.Get(),
        nullptr,
        IID_PPV_ARGS(m_commandList.GetAddressOf()));

    m_commandList->Close();
}

//2개의 버퍼를 가지는 swap chain을 형성한다
void Renderer::InitSwapChain(HWND windowHandle,
                             ComPtr<IDXGIFactory>& dxgiFactory,
                             ComPtr<ID3D12CommandQueue>& commandQueue) {
    m_swapChain.Reset();

    DXGI_SWAP_CHAIN_DESC sd;
    sd.BufferDesc.Width = m_screenWidth;
    sd.BufferDesc.Height = m_screenHeight;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferDesc.Format = m_backBufferFormat;
    sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED; //TODO:: 어떤 기능인지?
    sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED; //TODO:: 어떤 기능인지?
    //TODO:: msaa 추후에 설정
    sd.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
    sd.SampleDesc.Quality = m_4xMsaaState ? (m_4xMsaaQuality - 1) : 0;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = 2;
    sd.OutputWindow = windowHandle;
    sd.Windowed = true;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; //TODO:: 어떤 효과인지?
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    // Note: Swap chain uses queue to perform flush.
    ThrowIfFailed(dxgiFactory->CreateSwapChain(
        commandQueue.Get(),
        &sd,
        m_swapChain.GetAddressOf()));
}


ID3D12Resource* Renderer::CurrentBackBuffer() const
{
    return m_swapChainBuffer[m_currBackBufferIndex].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE Renderer::CurrentBackBufferView() const
{
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(
        m_rtvHeap->GetCPUDescriptorHandleForHeapStart(),
        m_currBackBufferIndex,
        m_rtvDescriptorSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE Renderer::DepthStencilView() const
{
    return m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
}

void Renderer::DidResizeWindow()
{
    FlushCommandQueue();
    ThrowIfFailed(m_commandList->Reset(m_commandListAllocator.Get(), nullptr));

    // Wait until resize is complete.
    FlushCommandQueue();

    ResetSwapChain();

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
    for (UINT i = 0; i < SwapChainBufferCount; i++)
    {
        ThrowIfFailed(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_swapChainBuffer[i])));
        m_device->CreateRenderTargetView(m_swapChainBuffer[i].Get(), nullptr, rtvHeapHandle);
        rtvHeapHandle.Offset(1, m_rtvDescriptorSize);
    }

    /*D3D12_RESOURCE_DESC depthStencilDesc;
    depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthStencilDesc.Alignment = 0;
    depthStencilDesc.Width = m_screenWidth;
    depthStencilDesc.Height = m_screenHeight;
    depthStencilDesc.DepthOrArraySize = 1;
    depthStencilDesc.MipLevels = 1;

    depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;

    depthStencilDesc.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
    depthStencilDesc.SampleDesc.Quality = m_4xMsaaState ? (m_4xMsaaQuality - 1) : 0;
    depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE optClear;
    optClear.Format = m_depthStencilFormat;
    optClear.DepthStencil.Depth = 1.0f;
    optClear.DepthStencil.Stencil = 0;
    ThrowIfFailed(m_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &depthStencilDesc,
        D3D12_RESOURCE_STATE_COMMON,
        &optClear,
        IID_PPV_ARGS(m_depthStencilBuffer.GetAddressOf())));

    // Create descriptor to mip level 0 of entire resource using the format of the resource.
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Format = m_depthStencilFormat;
    dsvDesc.Texture2D.MipSlice = 0;
    m_device->CreateDepthStencilView(m_depthStencilBuffer.Get(), &dsvDesc, DepthStencilView());

    // Transition the resource from its initial state to be used as a depth buffer.
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_depthStencilBuffer.Get(),
        D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));
        */
        // Execute the resize commands.
    // Execute the resize commands.
    SetViewport();

    ThrowIfFailed(m_commandList->Close());
    ID3D12CommandList* cmdsLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
}

void  Renderer::ResetSwapChain() {
    for (int i = 0; i < SwapChainBufferCount; ++i)
        m_swapChainBuffer[i].Reset();

    ThrowIfFailed(m_swapChain->ResizeBuffers(
        SwapChainBufferCount,
        m_screenWidth, m_screenHeight,
        m_backBufferFormat,
        DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

    m_currBackBufferIndex = 0;
}


void Renderer::BuildConstantBuffers()
{
    gpuConstantBuffer = std::make_unique<UploadBuffer<ConstantBuffer>>(m_device.Get(), 1, true);

    UINT objCBByteSize = CalcConstantBufferByteSize(sizeof(ConstantBuffer));

    D3D12_GPU_VIRTUAL_ADDRESS cbAddress = gpuConstantBuffer->Resource()->GetGPUVirtualAddress();
    // Offset to the ith object constant buffer in the buffer.
    int boxCBufIndex = 0;
    cbAddress += boxCBufIndex * objCBByteSize;

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
    cbvDesc.BufferLocation = cbAddress;
    cbvDesc.SizeInBytes = CalcConstantBufferByteSize(sizeof(ConstantBuffer));

    m_device->CreateConstantBufferView(
        &cbvDesc,
        m_cbvHeap->GetCPUDescriptorHandleForHeapStart());
}

void Renderer::BuildDescriptorHeaps()
{
    D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
    cbvHeapDesc.NumDescriptors = 1;
    cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    cbvHeapDesc.NodeMask = 0;
    ThrowIfFailed(m_device->CreateDescriptorHeap(&cbvHeapDesc,
        IID_PPV_ARGS(&m_cbvHeap)));
}


void Renderer::BuildRootSignature()
{

	CD3DX12_ROOT_PARAMETER slotRootParameter[1];

	CD3DX12_DESCRIPTOR_RANGE cbvTable;
	cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	slotRootParameter[0].InitAsConstantBufferView(0);

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(1, slotRootParameter, 0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(m_device->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&m_rootSignature)));
}

void Renderer::BuildShadersAndInputLayout()
{
    HRESULT hr = S_OK;

    m_vsByteCode = CompileShader(L"DefaultVertexShader.hlsl", nullptr, "main", "vs_5_0");
    m_psByteCode = CompileShader(L"DefaultPixelShader.hlsl", nullptr, "main", "ps_5_0");

    m_inputLayout =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

}

void Renderer::BuildDefaultPSO()
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
    ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    psoDesc.InputLayout = { m_inputLayout.data(), (UINT)m_inputLayout.size() };
    psoDesc.pRootSignature = m_rootSignature.Get();
    psoDesc.VS =
    {
        reinterpret_cast<BYTE*>(m_vsByteCode->GetBufferPointer()),
        m_vsByteCode->GetBufferSize()
    };
    psoDesc.PS =
    {
        reinterpret_cast<BYTE*>(m_psByteCode->GetBufferPointer()),
        m_psByteCode->GetBufferSize()
    };
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    //psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = m_backBufferFormat;
    psoDesc.SampleDesc.Count = 1; //TODO:: 분기 추후 적용
    psoDesc.SampleDesc.Quality = 0;
    //psoDesc.DSVFormat = mDepthStencilFormat;
    ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pso)));
}
