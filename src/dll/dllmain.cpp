#include <windows.h>
#include <fstream>
#include <string>
#include <sstream>

void InitializeHooks();
void ExecuteScript();

HANDLE hConsole = nullptr;
HANDLE hMainThread = nullptr;

void ConsolePrint(const char* msg) {
    if (hConsole) {
        WriteConsoleA(hConsole, msg, strlen(msg), NULL, NULL);
        WriteConsoleA(hConsole, "\n", 1, NULL, NULL);
    }
}

std::string ReadScriptFile() {
    std::ifstream file("temp_script.lua");
    if (!file.is_open()) return "";
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

DWORD WINAPI MainThread(LPVOID lpParam) {
    AllocConsole();
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    ConsolePrint("NekoSploit DLL loaded");
    
    Sleep(8000);
    
    ConsolePrint("[*] Initializing hooks...");
    InitializeHooks();
    
    std::string script = ReadScriptFile();
    if (!script.empty()) {
        ConsolePrint("[*] Executing script...");
        ExecuteScript();
    } else {
        ConsolePrint("[!] No script found (temp_script.lua missing)");
    }
    
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hModule);
            hMainThread = CreateThread(NULL, 0, MainThread, NULL, 0, NULL);
            break;
        case DLL_PROCESS_DETACH:
            if (hMainThread) CloseHandle(hMainThread);
            break;
    }
    return TRUE;
}
