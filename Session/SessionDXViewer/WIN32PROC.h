//
// Created by white on 25. 11. 19..
//
#ifndef FPSPROJECTSERVER_WIN32PROC_H
#define FPSPROJECTSERVER_WIN32PROC_H
#ifdef _WIN64
#include <windows.h>

inline LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        default:
            std::cout<<msg<< std::endl;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

inline HWND CreateDebugWindow(const HINSTANCE& hInst, const int width, const int height)
{
    const auto CLASS_NAME = reinterpret_cast<LPCSTR>(L"DebugDXWindow");

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = reinterpret_cast<LPCSTR>(CLASS_NAME);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        reinterpret_cast<LPCSTR>(L"DX Debug Window"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        width, height,
        nullptr,
        nullptr,
        hInst,
        nullptr
    );

    ShowWindow(hwnd, SW_SHOW);

    return hwnd;
}
#endif
#endif
