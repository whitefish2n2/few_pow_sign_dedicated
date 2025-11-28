//
// Created by white on 25. 11. 19..
//



#ifdef _WIN64
#include "DirectXCore.h"
#include <iostream>

#include "../../ServerStatics.h"
#pragma comment(lib, "d3d11.lib")
IDXGISwapChain* DirectXCore::swapChain = nullptr;
ID3D11Device* DirectXCore::device = nullptr;
ID3D11DeviceContext* DirectXCore::context = nullptr;
ID3D11RenderTargetView* DirectXCore::rtv = nullptr;
HWND DirectXCore::window = nullptr;

void DirectXCore:: Cleanup()
{
    if (rtv) rtv->Release();
    if (context) context->Release();
    if (device) device->Release();
    if (swapChain) swapChain->Release();
    if (window) window = nullptr;
}

bool DirectXCore::InitD3D(HWND hWnd, int width, int height)
{
    static bool registered = false;

    if (!registered) {
        atexit([]() { DirectXCore::Cleanup(); });
        registered = true;
    }
    Cleanup();

    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount = 1;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        nullptr, 0,
        D3D11_SDK_VERSION,
        &sd,
        &swapChain,
        &device,
        nullptr,
        &context
    );

    if (FAILED(hr))
        return false;

    ID3D11Texture2D* backBuffer = nullptr;
    swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);

    hr = device->CreateRenderTargetView(backBuffer, nullptr, &rtv);
    backBuffer->Release();

    if (FAILED(hr))
        return false;

    context->OMSetRenderTargets(1, &rtv, nullptr);
    window = hWnd;
    return true;
}
void DirectXCore::RunDirectXLoop() {
    MSG msg;
    while (isRunning.load()) {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                isRunning.store(false);
                break;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // 메시지 없을 때만 DirectX 렌더링
        constexpr float clearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
        context->ClearRenderTargetView(rtv, clearColor);

        context->Draw(2, 0);

        swapChain->Present(1, 0);
    }

    DirectXCore::Cleanup();
}



#endif
