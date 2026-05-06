#include <windows.h>
#include <commctrl.h>
#include <fstream>
#include <string>

#pragma comment(lib, "comctl32.lib")

#define ID_EDIT_SCRIPT   1001
#define ID_BTN_INJECT    1002
#define ID_BTN_LOAD      1003
#define ID_BTN_SAVE      1004
#define ID_BTN_CLEAR     1005
#define ID_COMBO_EXAMPLE 1006
#define ID_STATUS        1007
#define ID_BTN_SCAN      1008

HWND hEditScript, hStatus, hComboExample, hMainWindow;

// Forward declarations from other files
extern void SetStatus(const char* text);
extern void LoadScriptFile();
extern void SaveScriptFile();
extern void InjectScript();
extern void ClearScript();
extern void LoadExampleScript();
extern void ScanOffsets();

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            hMainWindow = hWnd;
            
            hEditScript = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "",
                WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
                10, 10, 480, 250, hWnd, (HMENU)ID_EDIT_SCRIPT, NULL, NULL);
            
            CreateWindowExA(0, "BUTTON", "Load", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                10, 270, 70, 30, hWnd, (HMENU)ID_BTN_LOAD, NULL, NULL);
            
            CreateWindowExA(0, "BUTTON", "Save", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                90, 270, 70, 30, hWnd, (HMENU)ID_BTN_SAVE, NULL, NULL);
            
            CreateWindowExA(0, "BUTTON", "Clear", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                170, 270, 70, 30, hWnd, (HMENU)ID_BTN_CLEAR, NULL, NULL);
            
            hComboExample = CreateWindowExA(0, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
                250, 270, 100, 100, hWnd, (HMENU)ID_COMBO_EXAMPLE, NULL, NULL);
            ComboBox_AddString(hComboExample, "Fly");
            ComboBox_AddString(hComboExample, "Teleport");
            ComboBox_AddString(hComboExample, "ESP");
            ComboBox_AddString(hComboExample, "Speed");
            ComboBox_SetCurSel(hComboExample, 0);
            
            CreateWindowExA(0, "BUTTON", "Scan Offsets", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                360, 270, 130, 30, hWnd, (HMENU)ID_BTN_SCAN, NULL, NULL);
            
            CreateWindowExA(0, "BUTTON", "Inject", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                10, 310, 480, 30, hWnd, (HMENU)ID_BTN_INJECT, NULL, NULL);
            
            hStatus = CreateWindowExA(0, "STATIC", "Ready", WS_CHILD | WS_VISIBLE | WS_BORDER,
                10, 350, 480, 25, hWnd, (HMENU)ID_STATUS, NULL, NULL);
            break;
            
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case ID_BTN_INJECT: InjectScript(); break;
                case ID_BTN_LOAD: LoadScriptFile(); break;
                case ID_BTN_SAVE: SaveScriptFile(); break;
                case ID_BTN_CLEAR: ClearScript(); break;
                case ID_BTN_SCAN: ScanOffsets(); break;
                case ID_COMBO_EXAMPLE:
                    if (HIWORD(wParam) == CBN_SELCHANGE) LoadExampleScript();
                    break;
            }
            break;
            
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
    }
    return DefWindowProcA(hWnd, msg, wParam, lParam);
}

int CreateMainWindow(HINSTANCE hInst, int nShow) {
    INITCOMMONCONTROLSEX icc = { sizeof(icc), ICC_WIN95_CLASSES };
    InitCommonControlsEx(&icc);
    
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = "NekoSploitClass";
    
    RegisterClassA(&wc);
    
    HWND hWnd = CreateWindowExA(0, "NekoSploitClass", "NekoSploit - Roblox Executor",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 510, 420,
        NULL, NULL, hInst, NULL);
    
    if (!hWnd) return 1;
    
    ShowWindow(hWnd, nShow);
    UpdateWindow(hWnd);
    
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}
