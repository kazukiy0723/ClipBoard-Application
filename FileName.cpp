#include <windows.h>

// Button identifier
#define BUTTON_ID 1

// Function for handling button press
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_COMMAND:
        if (LOWORD(wParam) == BUTTON_ID)
        {
            if (OpenClipboard(NULL))
            {
                HANDLE hClip = GetClipboardData(CF_TEXT);
                if (hClip)
                {
                    char* pszText = static_cast<char*>(GlobalLock(hClip));
                    if (pszText)
                    {
                        MessageBox(hwnd, pszText, "Clipboard Contents", MB_OK);
                        GlobalUnlock(hClip);
                    }
                }
                CloseClipboard();
            }
        }
        break;

    case WM_CREATE:
        CreateWindow("button", "Show Clipboard", WS_VISIBLE | WS_CHILD, 20, 20, 150, 25, hwnd, (HMENU)BUTTON_ID, NULL, NULL);
        break;

    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    const char CLASS_NAME[] = "Sample Window Class";

    WNDCLASS wc = {};

    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, CLASS_NAME, "Win32 API Clipboard Viewer", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 240, 120, NULL, NULL, hInstance, NULL);

    if (hwnd == NULL)
    {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
