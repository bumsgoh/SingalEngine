#include "Renderer.h"
#include "Scene.h"
#include "Utils.h"


Renderer::Renderer(const ComPtr<ID3D12Device>& device,
				   const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
	this->device = device;
	this->commandList = commandList;
}

void Renderer::Initialize(Scene& scene)
{
	scene.Initialize(device, commandList);
}

void Renderer::Render(Scene& scene)
{
    ID3D12DescriptorHeap* descriptorHeaps[] = { m_cbvHeap.Get() };
    commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
    commandList->SetGraphicsRootSignature(m_rootSignature.Get());
    scene.Render(commandList);
    commandList->SetGraphicsRootDescriptorTable(0, m_cbvHeap->GetGPUDescriptorHandleForHeapStart());
   
}


void Renderer::BuildConstantBuffers()
{
    mObjectCB = std::make_unique<UploadBuffer<ConstantBuffer>>(device.Get(), 1, true);

    UINT objCBByteSize = CalcConstantBufferByteSize(sizeof(ConstantBuffer));

    D3D12_GPU_VIRTUAL_ADDRESS cbAddress = mObjectCB->Resource()->GetGPUVirtualAddress();
    // Offset to the ith object constant buffer in the buffer.
    int boxCBufIndex = 0;
    cbAddress += boxCBufIndex * objCBByteSize;

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
    cbvDesc.BufferLocation = cbAddress;
    cbvDesc.SizeInBytes = CalcConstantBufferByteSize(sizeof(ConstantBuffer));

    device->CreateConstantBufferView(
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
    ThrowIfFailed(device->CreateDescriptorHeap(&cbvHeapDesc,
        IID_PPV_ARGS(&m_cbvHeap)));
}


void Renderer::BuildRootSignature()
{

	CD3DX12_ROOT_PARAMETER slotRootParameter[1];

	CD3DX12_DESCRIPTOR_RANGE cbvTable;
	cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

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

	ThrowIfFailed(device->CreateRootSignature(
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
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = m_backBufferFormat;
    psoDesc.SampleDesc.Count = 1; //TODO:: 분기 추후 적용
    psoDesc.SampleDesc.Quality = 0;
    //psoDesc.DSVFormat = mDepthStencilFormat;
    ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pso)));
}
