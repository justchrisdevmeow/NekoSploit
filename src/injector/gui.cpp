#include <windows.h>
#include <commctrl.h>
#include <string>
#include <fstream>
#include "gui.h"
#include "utils.h"

#pragma comment(lib, "comctl32.lib")

#define ID_EDIT_SCRIPT   1001
#define ID_BTN_INJECT    1002
#define ID_BTN_LOAD      1003
#define ID_BTN_SAVE      1004
#define ID_BTN_CLEAR     1005
#define ID_COMBO_EXAMPLE 1006
#define ID_STATUS        1007

HWND hEditScript, hStatus, hComboExample;
HWND hMainWindow;

void SetStatus(const char* text) {
    SetWindowTextA(hStatus, text);
}

void AppendConsole(const char* text) {
    // Add to status bar for now; could be extended to a separate console
    SetStatus(text);
}

void LoadScriptFile() {
    OPENFILENAMEA ofn = { sizeof(ofn) };
    char file[MAX_PATH] = {0};
    ofn.hwndOwner = hMainWindow;
    ofn.lpstrFilter = "Lua Files\0*.lua\0All Files\0*.*\0";
    ofn.lpstrFile = file;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST;
    
    if (GetOpenFileNameA(&ofn)) {
        std::ifstream in(file);
        std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        SetWindowTextA(hEditScript, content.c_str());
        SetStatus(("Loaded: " + std::string(file)).c_str());
    }
}

void SaveScriptFile() {
    OPENFILENAMEA ofn = { sizeof(ofn) };
    char file[MAX_PATH] = {0};
    ofn.hwndOwner = hMainWindow;
    ofn.lpstrFilter = "Lua Files\0*.lua\0All Files\0*.*\0";
    ofn.lpstrFile = file;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_OVERWRITEPROMPT;
    
    if (GetSaveFileNameA(&ofn)) {
        int len = GetWindowTextLengthA(hEditScript);
        char* script = new char[len + 1];
        GetWindowTextA(hEditScript, script, len + 1);
        
        std::ofstream out(file);
        out << script;
        out.close();
        delete[] script;
        SetStatus(("Saved: " + std::string(file)).c_str());
    }
}

void InjectScript() {
    int len = GetWindowTextLengthA(hEditScript);
    if (len == 0) {
        SetStatus("No script to inject.");
        return;
    }
    
    char* script = new char[len + 1];
    GetWindowTextA(hEditScript, script, len + 1);
    
    // Save to temp file for DLL to read
    std::ofstream out("temp_script.lua");
    out << script;
    out.close();
    
    delete[] script;
    
    if (InjectIntoRoblox("nekosploit.dll")) {
        SetStatus("Injected! Script will run if DLL hooks Lua.");
    }
}

void ClearScript() {
    SetWindowTextA(hEditScript, "");
    SetStatus("Script cleared.");
}

void LoadExampleScript() {
    int idx = ComboBox_GetCurSel(hComboExample);
    if (idx == CB_ERR) return;
    
    const char* examples[] = {
        "-- Fly Script\nlocal plr = game.Players.LocalPlayer\nlocal char = plr.Character\nif char then\n    local b = Instance.new(\"BodyVelocity\")\n    b.MaxForce = Vector3.new(1,1,1)*1e5\n    b.Velocity = Vector3.new(0,50,0)\n    b.Parent = char.UpperTorso\n    wait(5)\n    b:Destroy()\nend",
        
        "-- Teleport to Mouse\nlocal plr = game.Players.LocalPlayer\nlocal mouse = plr:GetMouse()\nlocal hrp = plr.Character.HumanoidRootPart\nhrp.CFrame = mouse.Hit",
        
        "-- ESP (Player names)\nfor _, plr in pairs(game.Players:GetPlayers()) do\n    if plr ~= game.Players.LocalPlayer then\n        local highlight = Instance.new(\"Highlight\")\n        highlight.Adornee = plr.Character\n        highlight.FillColor = Color3.new(1,0,0)\n        highlight.Parent = plr.Character\n    end\nend",
        
        "-- Speed Boost\nlocal plr = game.Players.LocalPlayer\nif plr.Character then\n    local hrp = plr.Character.HumanoidRootPart\n    local bv = Instance.new(\"BodyVelocity\")\n    bv.MaxForce = Vector3.new(1,1,1)*1e5\n    bv.Velocity = Vector3.new(100,0,100)\n    bv.Parent = hrp\n    wait(3)\n    bv:Destroy()\nend"
    };
    
    SetWindowTextA(hEditScript, examples[idx]);
    SetStatus(("Loaded example: " + std::to_string(idx + 1)).c_str());
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            hMainWindow = hWnd;
            
            // Create edit control (script area)
            hEditScript = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "",
                WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
                10, 10, 480, 250, hWnd, (HMENU)ID_EDIT_SCRIPT, NULL, NULL);
            
            // Load button
            CreateWindowExA(0, "BUTTON", "Load", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                10, 270, 70, 30, hWnd, (HMENU)ID_BTN_LOAD, NULL, NULL);
            
            // Save button
            CreateWindowExA(0, "BUTTON", "Save", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                90, 270, 70, 30, hWnd, (HMENU)ID_BTN_SAVE, NULL, NULL);
            
            // Clear button
            CreateWindowExA(0, "BUTTON", "Clear", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                170, 270, 70, 30, hWnd, (HMENU)ID_BTN_CLEAR, NULL, NULL);
            
            // Example scripts dropdown
            hComboExample = CreateWindowExA(0, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
                250, 270, 120, 100, hWnd, (HMENU)ID_COMBO_EXAMPLE, NULL, NULL);
            ComboBox_AddString(hComboExample, "Fly");
            ComboBox_AddString(hComboExample, "Teleport");
            ComboBox_AddString(hComboExample, "ESP");
            ComboBox_AddString(hComboExample, "Speed");
            ComboBox_SetCurSel(hComboExample, 0);
            
            // Inject button
            CreateWindowExA(0, "BUTTON", "Inject", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                380, 270, 110, 30, hWnd, (HMENU)ID_BTN_INJECT, NULL, NULL);
            
            // Status bar
            hStatus = CreateWindowExA(0, "STATIC", "Ready", WS_CHILD | WS_VISIBLE | WS_BORDER,
                10, 310, 480, 25, hWnd, (HMENU)ID_STATUS, NULL, NULL);
            break;
            
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case ID_BTN_INJECT: InjectScript(); break;
                case ID_BTN_LOAD: LoadScriptFile(); break;
                case ID_BTN_SAVE: SaveScriptFile(); break;
                case ID_BTN_CLEAR: ClearScript(); break;
                case ID_COMBO_EXAMPLE: 
                    if (HIWORD(wParam) == CBN_SELCHANGE) LoadExampleScript();
                    break;
            }
            break;
            
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
            
        default:
            return DefWindowProcA(hWnd, msg, wParam, lParam);
    }
    return 0;
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
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 510, 390,
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
