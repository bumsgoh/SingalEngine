#pragma once

#include "resource.h"
#include "GlobalFunctions.h"
#include "ShaderCommon.h"
#include "CommonMath.h"
#include "Scene.h"
#include "Renderer.h"
//vcpkg install imgui[core,dx12-binding,win32-binding]:x64-windows

#include <D3Dcompiler.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#include "d3dx12.h"
#include <d3dcompiler.h>
#include <DirectXMath.h>

#include <iostream>
#include <vector>
#include <directxtk/SimpleMath.h>
#include <windows.h>
#include <wrl/client.h> 

#include "Timer.h"

// Link necessary d3d12 libraries.
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

using Microsoft::WRL::ComPtr;
using std::shared_ptr;
using std::vector;
using std::wstring;

class SingalEngine {

public:
	static const int SwapChainBufferCount = 2;
	SingalEngine();
	virtual ~SingalEngine();
	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	bool Initialize();
	bool InitWindow(HINSTANCE hInstance, int nCmdShow);

	int Run();

	void UpdateEnginFrame(float dt);
	void UpdateGUI();
	void DidResizeWindow();

	void Update(float deltaTime);
	void Render(float DeltaTime);


public:

	HWND m_windowHandle = nullptr;

private:
	Timer m_timer;
	shared_ptr<Scene> m_scene = nullptr;
	shared_ptr<Renderer> m_renderer = nullptr;
};