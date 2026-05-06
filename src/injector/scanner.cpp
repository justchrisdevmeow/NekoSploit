#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <fstream>
#include <string>

#pragma comment(lib, "psapi.lib")

extern HWND hStatus;
extern void SetStatus(const char* text);

struct OffsetInfo {
    const char* name;
    const char* pattern;
    const char* mask;
    uintptr_t offset;
};

uintptr_t PatternScan(HMODULE module, const char* pattern, const char* mask) {
    MODULEINFO info = {0};
    GetModuleInformation(GetCurrentProcess(), module, &info, sizeof(info));
    
    uintptr_t start = (uintptr_t)module;
    uintptr_t end = start + info.SizeOfImage;
    size_t patternLen = strlen(mask);
    
    for (uintptr_t i = start; i < end - patternLen; i++) {
        bool found = true;
        for (size_t j = 0; j < patternLen; j++) {
            if (mask[j] == 'x' && ((uint8_t*)i)[j] != (uint8_t)pattern[j]) {
                found = false;
                break;
            }
        }
        if (found) return i;
    }
    return 0;
}

DWORD GetProcessIdForScan(const char* procName) {
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

void ScanOffsets() {
    SetStatus("Scanning Roblox for Lua offsets...");
    
    DWORD pid = GetProcessIdForScan("RobloxPlayerBeta.exe");
    if (pid == 0) pid = GetProcessIdForScan("RobloxApp.exe");
    if (pid == 0) {
        SetStatus("Roblox not running.");
        return;
    }
    
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (!hProcess) {
        SetStatus("Failed to open Roblox process.");
        return;
    }
    
    HMODULE hMods[1024];
    DWORD cbNeeded;
    
    if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
        for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
            char modName[MAX_PATH];
            GetModuleBaseNameA(hProcess, hMods[i], modName, sizeof(modName));
            if (_stricmp(modName, "RobloxPlayerBeta.exe") == 0 || _stricmp(modName, "RobloxApp.exe") == 0) {
                CloseHandle(hProcess);
                
                OffsetInfo offsets[] = {
                    {"luaL_loadstring", "\x55\x8B\xEC\x83\xE4\xF8\x83\xEC\x0C\x53\x56\x57\x8B\x75\x08\x85\xF6", "xxxxxxxxxxxxxxxxx", 0},
                    {"lua_pcall", "\x55\x8B\xEC\x83\xEC\x14\x53\x56\x57\x8B\x7D\x08", "xxxxxxxxxxxx", 0},
                    {"lua_getglobal", "\x55\x8B\xEC\x83\xEC\x0C\x53\x56\x57", "xxxxxxxxx", 0},
                };
                
                HMODULE roblox = hMods[i];
                uintptr_t base = (uintptr_t)roblox;
                bool allFound = true;
                
                for (auto& off : offsets) {
                    uintptr_t addr = PatternScan(roblox, off.pattern, off.mask);
                    if (addr) {
                        off.offset = addr - base;
                    } else {
                        allFound = false;
                    }
                }
                
                if (allFound) {
                    std::ofstream out("offsets_generated.h");
                    out << "// Generated automatically by NekoSploit scanner\n";
                    out << "// Copy these values to src/dll/offsets.cpp\n\n";
                    out << "#pragma once\n\n";
                    for (auto& off : offsets) {
                        out << "#define OFFSET_" << off.name << " 0x" << std::hex << off.offset << "\n";
                    }
                    out << "\n// Lua state offset (may need manual adjustment)\n";
                    out << "#define OFFSET_LuaState 0x00000000\n";
                    out.close();
                    SetStatus("Offsets saved to offsets_generated.h");
                    return;
                } else {
                    SetStatus("Some patterns not found. Roblox may have updated.");
                    return;
                }
            }
        }
    }
    
    CloseHandle(hProcess);
    SetStatus("Scan failed.");
}
