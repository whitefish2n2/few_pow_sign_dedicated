// DirectXConfig.h
#pragma once
#define WIN32_LEAN_AND_MEAN
#ifdef _WIN64
#include <atomic>
#include <d3d11.h>
#include <DirectXMath.h>

struct CBufferData {
    DirectX::XMMATRIX wvpMatrix;
    DirectX::XMFLOAT4 color;
};
class DirectXCore {
public:
    static bool InitD3D(HWND hwnd, int width, int height);

    static void RunDirectXLoop();

    static void Cleanup();

    static bool InitPipeline();

    static ID3D11Device* Device() { return device; }
    static ID3D11DeviceContext* Context() { return context; }

    static std::atomic<bool> isRunningViewer;
    static std::atomic<bool> isViewerAlive;
    static ID3D11Device* device;
private:
    static IDXGISwapChain* swapChain;
    static ID3D11DeviceContext* context;
    static ID3D11RenderTargetView* rtv;
    static ID3D11DepthStencilView* dsv;
    static ID3D11Texture2D* depthStencilBuffer;

    static HWND window;
    static int screenWidth;
    static int screenHeight;

    static ID3D11VertexShader* vertexShader;
    static ID3D11PixelShader* pixelShader;
    static ID3D11InputLayout* inputLayout;

    static ID3D11Buffer* constantBuffer;

};

#endif
