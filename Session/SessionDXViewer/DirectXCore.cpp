//
// Created by white on 25. 11. 19..
//



#ifdef _WIN64
#include "DirectXCore.h"

#include <d3dcompiler.h>
#include <thread>

#include "Camera.h"
#include "DebugViewStatic.h"
#include "../../ServerStatics.h"
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
IDXGISwapChain* DirectXCore::swapChain = nullptr;
ID3D11Device* DirectXCore::device = nullptr;
ID3D11DeviceContext* DirectXCore::context = nullptr;
ID3D11RenderTargetView* DirectXCore::rtv = nullptr;

HWND DirectXCore::window = nullptr;
int DirectXCore::screenWidth = 0;
int DirectXCore::screenHeight = 0;
std::atomic<bool> DirectXCore::isRunningViewer(true);

ID3D11VertexShader* DirectXCore::vertexShader = nullptr;
ID3D11PixelShader* DirectXCore::pixelShader = nullptr;
ID3D11InputLayout* DirectXCore::inputLayout = nullptr;
ID3D11Buffer* DirectXCore::vertexBuffer = nullptr;
ID3D11Buffer* DirectXCore::indexBuffer = nullptr;
ID3D11Buffer* DirectXCore::constantBuffer = nullptr;
ID3D11DepthStencilView* DirectXCore::dsv = nullptr;
ID3D11Texture2D* DirectXCore::depthStencilBuffer = nullptr;
Camera camera = Camera();
struct SimpleVertex {
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT4 color;
};
void DirectXCore:: Cleanup()
{
    if (constantBuffer) constantBuffer->Release();
    if (indexBuffer) indexBuffer->Release();
    if (vertexBuffer) vertexBuffer->Release();
    if (inputLayout) inputLayout->Release();
    if (vertexShader) vertexShader->Release();
    if (pixelShader) pixelShader->Release();
    if (dsv) dsv->Release();
    if (depthStencilBuffer) depthStencilBuffer->Release();
    if (rtv) rtv->Release();
    if (swapChain) swapChain->Release();
    if (context) context->Release();
    if (device) device->Release();
    if (window) window = nullptr;
}
bool DirectXCore::InitPipeline() {

    ID3DBlob* vsBlob = nullptr;
    ID3DBlob* psBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;

    UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS;
    #if defined(_DEBUG)
        compileFlags |= D3DCOMPILE_DEBUG;
    #endif

    HRESULT hr = D3DCompileFromFile(
        L"Assets/DebugDXViewer/Shader.hlsl",
        nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "VS", "vs_5_0",
        compileFlags, 0,
        &vsBlob, &errorBlob
    );
    if (FAILED(hr)) {
        if (errorBlob) {
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
            errorBlob->Release();
        }
        return false;
    }
    device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vertexShader);

    hr = D3DCompileFromFile(
        L"Assets/DebugDXViewer/Shader.hlsl",
        nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "PS", "ps_5_0",
        compileFlags, 0,
        &psBlob, &errorBlob
    );

    if (FAILED(hr)) {
        if (errorBlob) {
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
            errorBlob->Release();
        }
        psBlob->Release();
        return false;
    }
    device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pixelShader);


    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    device->CreateInputLayout(layout, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayout);

    if (vsBlob) vsBlob->Release();
    if (psBlob) psBlob->Release();
    if (errorBlob) errorBlob->Release();

    SimpleVertex vertices[] = {
        { DirectX::XMFLOAT3(-1.0f,  1.0f, -1.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) }, // 0
        { DirectX::XMFLOAT3( 1.0f,  1.0f, -1.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) }, // 1
        { DirectX::XMFLOAT3( 1.0f,  1.0f,  1.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) }, // 2
        { DirectX::XMFLOAT3(-1.0f,  1.0f,  1.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) }, // 3
        { DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) }, // 4
        { DirectX::XMFLOAT3( 1.0f, -1.0f, -1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) }, // 5
        { DirectX::XMFLOAT3( 1.0f, -1.0f,  1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) }, // 6
        { DirectX::XMFLOAT3(-1.0f, -1.0f,  1.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) }, // 7
    };
    WORD indices[] = {
        3,1,0, 2,1,3, // 윗면
        0,5,4, 1,5,0, // 앞면
        3,4,7, 0,4,3, // 왼쪽
        1,6,5, 2,6,1, // 오른쪽
        2,7,6, 3,7,2, // 뒷면
        6,4,5, 7,4,6  // 아랫면
    };

    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SimpleVertex) * 8;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = vertices;

    hr = device->CreateBuffer(&bd, &initData, &vertexBuffer);
    if (FAILED(hr)) return false;

    bd.ByteWidth = sizeof(WORD) * 36;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    initData.pSysMem = indices;
    hr = device->CreateBuffer(&bd, &initData, &indexBuffer);
    if (FAILED(hr)) return false;

    D3D11_BUFFER_DESC cbd = {};
    cbd.Usage = D3D11_USAGE_DYNAMIC;            // 중요: 매 프레임 CPU가 값을 바꿀 거니까 DYNAMIC
    cbd.ByteWidth = sizeof(CBufferData);        // 크기는 구조체 크기만큼
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER; // 용도는 상수 버퍼
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;// CPU가 쓸(Write) 수 있어야 함
    hr = device->CreateBuffer(&cbd, nullptr, &constantBuffer);
    if (FAILED(hr)) return false;

    D3D11_RASTERIZER_DESC wfDesc = {};
    wfDesc.FillMode = D3D11_FILL_WIREFRAME;
    wfDesc.CullMode = D3D11_CULL_NONE;
    device->CreateRasterizerState(&wfDesc, &DebugViewStatic::wireframeState);

    // 원래대로 돌릴 솔리드 모드도 만들어둠
    wfDesc.FillMode = D3D11_FILL_SOLID;
    wfDesc.CullMode = D3D11_CULL_BACK; // 뒷면은 가려라 (기본값)
    device->CreateRasterizerState(&wfDesc, &DebugViewStatic::solidState);

    context->RSSetState(DebugViewStatic::wireframeState);
    return true;
}
bool DirectXCore::InitD3D(HWND hWnd, int width, int height){
    static bool registered = false;

    if (!registered) {
        atexit([]() { DirectXCore::Cleanup(); });
        registered = true;
    }
    Cleanup();

    screenWidth = width;
    screenHeight = height;

    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 1;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    if (FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, nullptr, 0, D3D11_SDK_VERSION, &sd, &swapChain, &device, nullptr, &context)))
        return false;
    ID3D11Texture2D* backBuffer = nullptr;
    swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
    device->CreateRenderTargetView(backBuffer, nullptr, &rtv);
    backBuffer->Release();

    D3D11_TEXTURE2D_DESC depthDesc = {};
    depthDesc.Width = width;
    depthDesc.Height = height;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // 24비트 깊이, 8비트 스텐실
    depthDesc.SampleDesc.Count = 1;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    device->CreateTexture2D(&depthDesc, nullptr, &depthStencilBuffer);
    device->CreateDepthStencilView(depthStencilBuffer, nullptr, &dsv);

    context->OMSetRenderTargets(1, &rtv, dsv);

    window = hWnd;
    D3D11_VIEWPORT vp;
    vp.Width = (float)width;
    vp.Height = (float)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    context->RSSetViewports(1, &vp);

    return InitPipeline();
}
void DirectXCore::RunDirectXLoop() {

    std::thread renderThread([&](){
        POINT clickPos = {0, 0};
        bool isRightPressed = false;
        while (isRunningViewer.load()) {
            bool currentRightDown = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
            if (currentRightDown) {
                POINT currentPos;
                GetCursorPos(&currentPos);

                if (!isRightPressed) {

                    isRightPressed = true;
                    clickPos = currentPos;
                    ShowCursor(FALSE);
                }
                else {

                    float dx = (float)(currentPos.x - clickPos.x);
                    float dy = (float)-(currentPos.y - clickPos.y);


                    if (dx != 0 || dy != 0) {
                        camera.OnMouseInput(dx, dy);

                        SetCursorPos(clickPos.x, clickPos.y);
                    }
                }
            }
            else {

                if (isRightPressed) {
                    isRightPressed = false;
                    ShowCursor(TRUE); // 커서 다시 보이기

                }
            }
            if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
                camera.ChangeToSprintSpeed();
            } else {
                camera.ChangeToDefaultSpeed();
            }
            if (GetAsyncKeyState('W') & 0x8000) camera.MoveForward();
            if (GetAsyncKeyState('S') & 0x8000) camera.MoveBackward();
            if (GetAsyncKeyState('A') & 0x8000) camera.MoveLeft();
            if (GetAsyncKeyState('D') & 0x8000) camera.MoveRight();
            if (GetAsyncKeyState('Q') & 0x8000) camera.MoveUp();
            if (GetAsyncKeyState('E') & 0x8000) camera.MoveDown();
            camera.UpdateView();



            constexpr float clearColor[4] = { 0.0f, 0.1f, 0.2f, 1.0f }; // 남색
            context->ClearRenderTargetView(rtv, clearColor);
            context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 0); // 깊이를 1.0(가장 뒤)으로 초기화

            DirectX::XMMATRIX world =  DirectX::XMMatrixRotationY(0) * DirectX::XMMatrixRotationX(0);

            DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV4, static_cast<float>(screenWidth) / static_cast<float>(screenHeight), 0.01f, 100.0f);
            DirectX::XMMATRIX wvp = world * camera.view * proj;
            wvp = DirectX::XMMatrixTranspose(wvp);

            D3D11_MAPPED_SUBRESOURCE mapped;
            context->Map(constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
            const auto dataPtr = static_cast<CBufferData *>(mapped.pData);
            dataPtr->wvpMatrix = wvp;
            context->Unmap(constantBuffer, 0);

            UINT stride = sizeof(SimpleVertex);
            UINT offset = 0;
            context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
            context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R16_UINT, 0);
            context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            context->IASetInputLayout(inputLayout);

            context->VSSetShader(vertexShader, nullptr, 0);
            context->VSSetConstantBuffers(0, 1, &constantBuffer);
            context->PSSetShader(pixelShader, nullptr, 0);
            context->DrawIndexed(36, 0, 0);

            swapChain->Present(1, 0);
        }
    });
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        if (msg.message == WM_QUIT) { // WM_QUIT 받으면 루프 종료
            isRunning.store(false);
            break;
        }
    }

    if (renderThread.joinable()) {renderThread.join();}

    DirectXCore::Cleanup();
}



#endif
