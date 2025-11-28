// DirectXConfig.h
#pragma once

#ifdef _WIN64
#include <d3d11.h>
#include <windows.h>

class DirectXCore {
public:
    static bool InitD3D(HWND hwnd, int width, int height);

    static void RunDirectXLoop();

    static void Cleanup();

    static ID3D11Device* Device() { return device; }
    static ID3D11DeviceContext* Context() { return context; }

private:
    static IDXGISwapChain* swapChain;
    static ID3D11Device* device;
    static ID3D11DeviceContext* context;
    static ID3D11RenderTargetView* rtv;
    static HWND window;
};

#endif
