#include <windows.h>
#include <string>
#include "offsets.cpp"

typedef int (*luaL_loadstring_t)(void* L, const char* s);
typedef int (*lua_pcall_t)(void* L, int nargs, int nresults, int errfunc);
typedef void (*lua_getglobal_t)(void* L, const char* name);

luaL_loadstring_t luaL_loadstring = nullptr;
lua_pcall_t lua_pcall = nullptr;
lua_getglobal_t lua_getglobal = nullptr;
void* lua_state = nullptr;

extern void ConsolePrint(const char* msg);
extern std::string ReadScriptFile();

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

void FindLuaFunctions() {
    HMODULE roblox = GetModuleHandleA("RobloxPlayerBeta.exe");
    if (!roblox) roblox = GetModuleHandleA("RobloxApp.exe");
    if (!roblox) {
        ConsolePrint("[!] Roblox module not found");
        return;
    }
    
    ConsolePrint("[*] Scanning for Lua functions...");
    
    // Use offsets from offsets.cpp
    uintptr_t base = (uintptr_t)roblox;
    
    if (OFFSET_luaL_loadstring != 0) {
        luaL_loadstring = (luaL_loadstring_t)(base + OFFSET_luaL_loadstring);
        ConsolePrint("[+] luaL_loadstring found at offset");
    } else {
        // Fallback to pattern scan
        const char* pattern = "\x55\x8B\xEC\x83\xE4\xF8\x83\xEC\x0C\x53\x56\x57\x8B\x75\x08\x85\xF6";
        const char* mask = "xxxxxxxxxxxxxxxxx";
        uintptr_t addr = PatternScan(roblox, pattern, mask);
        if (addr) {
            luaL_loadstring = (luaL_loadstring_t)addr;
            ConsolePrint("[+] luaL_loadstring found via pattern scan");
        }
    }
    
    if (OFFSET_lua_pcall != 0) {
        lua_pcall = (lua_pcall_t)(base + OFFSET_lua_pcall);
        ConsolePrint("[+] lua_pcall found at offset");
    } else {
        const char* pattern = "\x55\x8B\xEC\x83\xEC\x14\x53\x56\x57\x8B\x7D\x08";
        const char* mask = "xxxxxxxxxxxx";
        uintptr_t addr = PatternScan(roblox, pattern, mask);
        if (addr) {
            lua_pcall = (lua_pcall_t)addr;
            ConsolePrint("[+] lua_pcall found via pattern scan");
        }
    }
    
    if (OFFSET_lua_getglobal != 0) {
        lua_getglobal = (lua_getglobal_t)(base + OFFSET_lua_getglobal);
        ConsolePrint("[+] lua_getglobal found at offset");
    } else {
        const char* pattern = "\x55\x8B\xEC\x83\xEC\x0C\x53\x56\x57";
        const char* mask = "xxxxxxxxx";
        uintptr_t addr = PatternScan(roblox, pattern, mask);
        if (addr) {
            lua_getglobal = (lua_getglobal_t)addr;
            ConsolePrint("[+] lua_getglobal found via pattern scan");
        }
    }
}

void* FindLuaState() {
    HMODULE roblox = GetModuleHandleA("RobloxPlayerBeta.exe");
    if (!roblox) return nullptr;
    
    // Try offset method first
    if (OFFSET_LuaState != 0) {
        uintptr_t base = (uintptr_t)roblox;
        void** luaStatePtr = (void**)(base + OFFSET_LuaState);
        if (!IsBadReadPtr(luaStatePtr, sizeof(void*))) {
            return *luaStatePtr;
        }
    }
    
    // Alternative: get from luaL_loadstring first parameter when called
    // This is more complex and requires hooking
    
    ConsolePrint("[!] Could not find Lua state");
    return nullptr;
}

void ExecuteScript() {
    std::string script = ReadScriptFile();
    if (script.empty()) {
        ConsolePrint("[!] No script to execute");
        return;
    }
    
    if (!luaL_loadstring || !lua_pcall) {
        ConsolePrint("[!] Lua functions not found. Run FindLuaFunctions() first.");
        return;
    }
    
    if (!lua_state) {
        lua_state = FindLuaState();
        if (!lua_state) return;
        ConsolePrint("[+] Lua state found");
    }
    
    // Load the string
    if (luaL_loadstring(lua_state, script.c_str()) == 0) {
        // Execute it
        if (lua_pcall(lua_state, 0, 0, 0) == 0) {
            ConsolePrint("[+] Script executed successfully");
        } else {
            ConsolePrint("[-] Script execution failed (pcall error)");
        }
    } else {
        ConsolePrint("[-] Failed to load script (syntax error?)");
    }
}

void InitializeHooks() {
    FindLuaFunctions();
    
    if (luaL_loadstring && lua_pcall) {
        ConsolePrint("[+] NekoSploit ready");
    } else {
        ConsolePrint("[!] Offsets may be outdated. Run scanner again.");
    }
}
