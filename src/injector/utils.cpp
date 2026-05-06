#include <windows.h>
#include <tlhelp32.h>
#include <fstream>
#include <string>

#pragma comment(lib, "comctl32.lib")

// ComboBox macro
#ifndef ComboBox_GetCurSel
#define ComboBox_GetCurSel(hWnd) (int)(SNDMSG((hWnd), CB_GETCURSEL, 0, 0))
#endif

extern HWND hStatus, hComboExample, hMainWindow;

void SetStatus(const char* text) {
    SetWindowTextA(hStatus, text);
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
        SetWindowTextA(GetDlgItem(hMainWindow, 1001), content.c_str());
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
        int len = GetWindowTextLengthA(GetDlgItem(hMainWindow, 1001));
        char* script = new char[len + 1];
        GetWindowTextA(GetDlgItem(hMainWindow, 1001), script, len + 1);
        
        std::ofstream out(file);
        out << script;
        out.close();
        delete[] script;
        SetStatus(("Saved: " + std::string(file)).c_str());
    }
}

void ClearScript() {
    SetWindowTextA(GetDlgItem(hMainWindow, 1001), "");
    SetStatus("Cleared.");
}

void LoadExampleScript() {
    int idx = ComboBox_GetCurSel(GetDlgItem(hMainWindow, 1006));
    if (idx == CB_ERR) return;
    
    const char* examples[] = {
        "-- Fly Script\nlocal plr = game.Players.LocalPlayer\nlocal char = plr.Character\nif char then\n    local b = Instance.new(\"BodyVelocity\")\n    b.MaxForce = Vector3.new(1,1,1)*1e5\n    b.Velocity = Vector3.new(0,50,0)\n    b.Parent = char.UpperTorso\n    wait(5)\n    b:Destroy()\nend",
        "-- Teleport to Mouse\nlocal plr = game.Players.LocalPlayer\nlocal mouse = plr:GetMouse()\nlocal hrp = plr.Character.HumanoidRootPart\nhrp.CFrame = mouse.Hit",
        "-- ESP\nfor _, plr in pairs(game.Players:GetPlayers()) do\n    if plr ~= game.Players.LocalPlayer then\n        local highlight = Instance.new(\"Highlight\")\n        highlight.Adornee = plr.Character\n        highlight.FillColor = Color3.new(1,0,0)\n        highlight.Parent = plr.Character\n    end\nend",
        "-- Speed\nlocal plr = game.Players.LocalPlayer\nif plr.Character then\n    local hrp = plr.Character.HumanoidRootPart\n    local bv = Instance.new(\"BodyVelocity\")\n    bv.MaxForce = Vector3.new(1,1,1)*1e5\n    bv.Velocity = Vector3.new(100,0,100)\n    bv.Parent = hrp\n    wait(3)\n    bv:Destroy()\nend"
    };
    
    SetWindowTextA(GetDlgItem(hMainWindow, 1001), examples[idx]);
    SetStatus("Loaded example");
}

DWORD GetProcessIdByName(const char* procName) {
    PROCESSENTRY32 entry = { sizeof(entry) };
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return 0;
    
    if (Process32First(snapshot, &entry)) {
        do {
            if (_stricmp(entry.szExeFile, procName) == 0) {
                CloseHandle(snapshot);
                return entry.th32ProcessID;
            }
        } while (Process32Next(snapshot, &entry));
    }
    CloseHandle(snapshot);
    return 0;
}

bool InjectDLL(DWORD pid, const char* dllPath) {
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProcess) return false;
    
    size_t pathSize = strlen(dllPath) + 1;
    LPVOID remoteMem = VirtualAllocEx(hProcess, NULL, pathSize, MEM_COMMIT, PAGE_READWRITE);
    if (!remoteMem) {
        CloseHandle(hProcess);
        return false;
    }
    
    if (!WriteProcessMemory(hProcess, remoteMem, dllPath, pathSize, NULL)) {
        VirtualFreeEx(hProcess, remoteMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }
    
    LPTHREAD_START_ROUTINE loadLib = (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
    if (!loadLib) {
        VirtualFreeEx(hProcess, remoteMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }
    
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, loadLib, remoteMem, 0, NULL);
    if (!hThread) {
        VirtualFreeEx(hProcess, remoteMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }
    
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    VirtualFreeEx(hProcess, remoteMem, 0, MEM_RELEASE);
    CloseHandle(hProcess);
    return true;
}

void InjectScript() {
    int len = GetWindowTextLengthA(GetDlgItem(hMainWindow, 1001));
    if (len == 0) {
        SetStatus("No script to inject.");
        return;
    }
    
    char* script = new char[len + 1];
    GetWindowTextA(GetDlgItem(hMainWindow, 1001), script, len + 1);
    
    std::ofstream out("temp_script.lua");
    out << script;
    out.close();
    delete[] script;
    
    DWORD pid = GetProcessIdByName("RobloxPlayerBeta.exe");
    if (pid == 0) pid = GetProcessIdByName("RobloxApp.exe");
    if (pid == 0) {
        SetStatus("Roblox not running.");
        return;
    }
    
    char dllPath[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, dllPath);
    strcat(dllPath, "\\nekosploit.dll");
    
    if (InjectDLL(pid, dllPath)) {
        SetStatus("Injected! Script sent to Roblox.");
    } else {
        SetStatus("Injection failed.");
    }
}
