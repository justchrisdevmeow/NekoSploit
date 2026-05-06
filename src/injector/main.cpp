#include <windows.h>

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmdLine, int nShow) {
    // Call gui.cpp's main function
    extern int CreateMainWindow(HINSTANCE, int);
    return CreateMainWindow(hInst, nShow);
}
