/*
TinySpacer

A Minimal, no‑bullshit workspace grouper for Windows.

Max 5 groups
- Alt + 1..5: switch to group
- Alt+Shift+1..5: move current window to a group
- Tray icon shows current workspace number
- Single .cpp file, no config, no GUI


Build (Developer Command Prompt for VS):
  cl.exe /nologo /EHsc /O2 tinyspacer.cpp user32.lib shell32.lib gdi32.lib /link /SUBSYSTEM:WINDOWS

*/

#include <windows.h>
#include <shellapi.h>
#include <vector>
#include <algorithm>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "gdi32.lib")

#define MAX_GROUPS 5
#define HOTKEY_SWITCH_BASE  100
#define HOTKEY_MOVE_BASE    200
#define TRAY_ICON_ID        1
#define WM_TRAYICON         (WM_USER + 1)
#define ID_TRAY_EXIT        9001
#define TIMER_ID_CATCH      1

struct Group {
    std::vector<HWND> windows;
};

static Group           g_groups[MAX_GROUPS];
static int             g_currentGroup = 0;
static HWND            g_hwnd         = nullptr;
static NOTIFYICONDATAA g_nid          = { 0 };
static HWINEVENTHOOK   g_eventHook    = NULL;

// ----------------- Helpers -----------------

bool IsRealWindow(HWND hwnd) {
    if (!IsWindow(hwnd) || !IsWindowVisible(hwnd))
        return false;

    LONG style = GetWindowLong(hwnd, GWL_STYLE);
    LONG ex    = GetWindowLong(hwnd, GWL_EXSTYLE);

    if (ex & WS_EX_TOOLWINDOW) return false;
    if (!(style & WS_OVERLAPPEDWINDOW)) return false;

    char title[256];
    if (GetWindowTextA(hwnd, title, sizeof(title)) == 0) return false;

    char cls[256];
    GetClassNameA(hwnd, cls, sizeof(cls));
    if (strcmp(cls, "Progman") == 0 || strcmp(cls, "WorkerW") == 0 || strcmp(cls, "Shell_TrayWnd") == 0)
        return false;

    return true;
}

bool IsTracked(HWND hwnd) {
    for (int i = 0; i < MAX_GROUPS; i++) {
        for (HWND w : g_groups[i].windows) {
            if (w == hwnd) return true;
        }
    }
    return false;
}

void RemoveFromAllGroups(HWND hwnd) {
    for (int i = 0; i < MAX_GROUPS; ++i) {
        auto& v = g_groups[i].windows;
        v.erase(std::remove(v.begin(), v.end(), hwnd), v.end());
    }
}

void AddToGroup(int group, HWND hwnd) {
    if (group < 0 || group >= MAX_GROUPS) return;
    if (!IsRealWindow(hwnd)) return;

    RemoveFromAllGroups(hwnd);
    g_groups[group].windows.push_back(hwnd);
}

void CatchUnassignedWindows() {
    EnumWindows([](HWND hwnd, LPARAM) -> BOOL {
        if (IsRealWindow(hwnd) && !IsTracked(hwnd)) {
            g_groups[g_currentGroup].windows.push_back(hwnd);
        }
        return TRUE;
    }, 0);
}

// ----------------- Icon Generation -----------------

HICON CreateNumberIcon(int number) {
    const int W = 16, H = 16;
    HDC hdc = GetDC(NULL);
    HDC mem = CreateCompatibleDC(hdc);
    BITMAPV5HEADER bi = { 0 };
    bi.bV5Size = sizeof(BITMAPV5HEADER);
    bi.bV5Width = W; bi.bV5Height = -H; bi.bV5Planes = 1; bi.bV5BitCount = 32;
    bi.bV5Compression = BI_BITFIELDS;
    bi.bV5RedMask = 0x00FF0000; bi.bV5GreenMask = 0x0000FF00; bi.bV5BlueMask = 0x000000FF; bi.bV5AlphaMask = 0xFF000000;
    void* bits = nullptr;
    HBITMAP bmpColor = CreateDIBSection(hdc, (BITMAPINFO*)&bi, DIB_RGB_COLORS, &bits, NULL, 0);
    HBITMAP oldColor = (HBITMAP)SelectObject(mem, bmpColor);
    memset(bits, 0, W * H * 4);
    SetBkMode(mem, TRANSPARENT);
    SetTextColor(mem, RGB(255, 255, 255));
    HFONT font = CreateFontA(15, 0, 0, 0, FW_BLACK, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, "Arial");
    HFONT oldFont = (HFONT)SelectObject(mem, font);
    char txt[2] = { char('1' + number), 0 };
    TextOutA(mem, 4, 0, txt, 1);
    SelectObject(mem, oldFont); DeleteObject(font); SelectObject(mem, oldColor);
    HBITMAP bmpMask = CreateBitmap(W, H, 1, 1, NULL);
    ICONINFO ii = { TRUE, 0, 0, bmpMask, bmpColor };
    HICON icon = CreateIconIndirect(&ii);
    DeleteObject(bmpColor); DeleteObject(bmpMask); DeleteDC(mem); ReleaseDC(NULL, hdc);
    return icon;
}

void UpdateTrayIcon() {
    char tip[64];
    wsprintfA(tip, "TinySpacer Group: %d", g_currentGroup + 1);
    lstrcpynA(g_nid.szTip, tip, sizeof(g_nid.szTip));

    HICON icon = CreateNumberIcon(g_currentGroup);
    g_nid.hIcon = icon;
    g_nid.uFlags = NIF_TIP | NIF_ICON;
    Shell_NotifyIconA(NIM_MODIFY, &g_nid);

    // if explorer shell restarted add  tray icon back (this is only triggered when one of the tinyspacer shortcuts are pressed)
    if (!Shell_NotifyIconA(NIM_MODIFY, &g_nid)) {
        Shell_NotifyIconA(NIM_ADD, &g_nid);
    }
    static HICON prev = NULL;
    if (prev) DestroyIcon(prev);
    prev = icon;
}

// ----------------- Core Logic -----------------

void SwitchToGroup(int group) {
    if (group < 0 || group >= MAX_GROUPS || group == g_currentGroup) return;

    CatchUnassignedWindows();

    for (HWND hwnd : g_groups[g_currentGroup].windows) {
        if (IsWindow(hwnd)) ShowWindow(hwnd, SW_HIDE);
    }

    g_currentGroup = group;

    for (HWND hwnd : g_groups[g_currentGroup].windows) {
        if (IsWindow(hwnd)) ShowWindow(hwnd, SW_SHOW);
    }

    if (!g_groups[g_currentGroup].windows.empty()) {
        SetForegroundWindow(g_groups[g_currentGroup].windows.back());
    }

    UpdateTrayIcon();
}

// ----------------- Callbacks -----------------

void CALLBACK HandleWinEvent(HWINEVENTHOOK, DWORD event, HWND hwnd, LONG, LONG, DWORD, DWORD) {
    if (event != EVENT_SYSTEM_FOREGROUND) return;
    if (!IsRealWindow(hwnd)) return;

    AddToGroup(g_currentGroup, hwnd);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_TIMER:
        if (wParam == TIMER_ID_CATCH) CatchUnassignedWindows();
        break;
    case WM_TRAYICON:
        if (lParam == WM_RBUTTONUP) {
            POINT pt; GetCursorPos(&pt);
            HMENU hMenu = CreatePopupMenu();
            AppendMenuA(hMenu, MF_STRING, ID_TRAY_EXIT, "Exit");
            SetForegroundWindow(hwnd);
            TrackPopupMenu(hMenu, TPM_BOTTOMALIGN, pt.x, pt.y, 0, hwnd, NULL);
            DestroyMenu(hMenu);
        }
        break;
    case WM_COMMAND:
        if (LOWORD(wParam) == ID_TRAY_EXIT) PostQuitMessage(0);
        break;
    case WM_HOTKEY: {
        int id = (int)wParam;
        if (id >= HOTKEY_SWITCH_BASE && id < HOTKEY_SWITCH_BASE + MAX_GROUPS)
            SwitchToGroup(id - HOTKEY_SWITCH_BASE);
        else if (id >= HOTKEY_MOVE_BASE && id < HOTKEY_MOVE_BASE + MAX_GROUPS) {
            HWND fg = GetForegroundWindow();
            if (IsRealWindow(fg)) {
                AddToGroup(id - HOTKEY_MOVE_BASE, fg);
                SwitchToGroup(id - HOTKEY_MOVE_BASE);
            }
        }
        break;
    }
    case WM_DESTROY: PostQuitMessage(0); break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// ----------------- WinMain helpers -----------------

void TinySpacer_WaitForShellTray() {
    while (!FindWindowA("Shell_TrayWnd", NULL)) {
        Sleep(500);
    }
}

void TinySpacer_InitWindowClass(HINSTANCE hInstance) {
    WNDCLASSA wc = { 0 };
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = "WorkspaceManagerClass";
    RegisterClassA(&wc);
}

void TinySpacer_CreateMessageWindow(HINSTANCE hInstance) {
    g_hwnd = CreateWindowExA(
        0,
        "WorkspaceManagerClass",
        "workspace",
        0,
        0, 0, 0, 0,
        HWND_MESSAGE,
        NULL,
        hInstance,
        NULL);
}

void TinySpacer_InitTrayIcon() {
    g_nid.cbSize           = sizeof(g_nid);
    g_nid.hWnd             = g_hwnd;
    g_nid.uID              = TRAY_ICON_ID;
    g_nid.uFlags           = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    g_nid.uCallbackMessage = WM_TRAYICON;
    g_nid.hIcon            = LoadIcon(NULL, IDI_APPLICATION);
    lstrcpynA(g_nid.szTip, "TinySpacer Group: 1", sizeof(g_nid.szTip));
    Shell_NotifyIconA(NIM_ADD, &g_nid);
    UpdateTrayIcon();
}

void TinySpacer_RegisterHotkeys() {
    for (int i = 0; i < MAX_GROUPS; i++) {
        RegisterHotKey(g_hwnd, HOTKEY_SWITCH_BASE + i, MOD_ALT, '1' + i);
        RegisterHotKey(g_hwnd, HOTKEY_MOVE_BASE + i, MOD_ALT | MOD_SHIFT, '1' + i);
    }
}

void TinySpacer_InitHooksAndTimers() {
    g_eventHook = SetWinEventHook(
        EVENT_SYSTEM_FOREGROUND,
        EVENT_SYSTEM_FOREGROUND,
        NULL,
        HandleWinEvent,
        0, 0,
        WINEVENT_OUTOFCONTEXT);
    SetTimer(g_hwnd, TIMER_ID_CATCH, 500, NULL);
}

void TinySpacer_CleanupAndShowAll() {
    for (int i = 0; i < MAX_GROUPS; i++) {
        for (HWND hw : g_groups[i].windows) {
            if (IsWindow(hw)) ShowWindow(hw, SW_SHOW);
        }
    }

    if (g_eventHook) UnhookWinEvent(g_eventHook);
    Shell_NotifyIconA(NIM_DELETE, &g_nid);
}

// ----------------- Main -----------------

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    TinySpacer_WaitForShellTray();

    TinySpacer_InitWindowClass(hInstance);
    TinySpacer_CreateMessageWindow(hInstance);

    TinySpacer_InitTrayIcon();

    TinySpacer_RegisterHotkeys();

    CatchUnassignedWindows();

    TinySpacer_InitHooksAndTimers();

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    TinySpacer_CleanupAndShowAll();
    return 0;
}