// --- xRcon\main.cpp ---
// Main entry point for the XRcon application.
// Sets up the Windows application, handles window messages, and manages UI page navigation.

#include <windows.h>
#include <commctrl.h>
#include "UIComponents.h"
#include "UIServers.h"
#include "UIRcon.h"
#include "ServerPage.h"
#include "RconPage.h"
#include "ServerManager.h"
#include "resource.h"

static HBRUSH g_hOutput = nullptr;        // Brush for output box background
static HBRUSH g_hFormBackground = nullptr; // Brush for form background

// Hides all controls for server and RCON pages.
void hideAllPageControls(HWND hwnd, bool showingServerPage) {
    // Hide server page controls
    ShowWindow(GetDlgItem(hwnd, 200), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, 300), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, 301), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, 302), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, 303), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, 304), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, 305), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, 306), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, 307), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, 310), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, 312), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, 313), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, 314), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, 315), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, 316), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, 317), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, 318), SW_HIDE);

    // Hide RCON page controls
    ShowWindow(GetDlgItem(hwnd, 500), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, 501), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, 502), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, 503), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, 504), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, 510), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, 511), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, 512), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, 513), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, 514), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, 515), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, 516), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, 517), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, 518), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, 519), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, 520), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, 521), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, 522), SW_HIDE);
    ShowWindow(GetDlgItem(hwnd, 523), SW_HIDE);

    // Hide shared output box
    ShowWindow(GetDlgItem(hwnd, 400), SW_HIDE);
}

// Window procedure to handle messages for the main window.
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static bool isServerPage = true; // Tracks current page (true for server, false for RCON)

    switch (msg) {
    case WM_LBUTTONDOWN: {
        POINT pt = { LOWORD(lParam), HIWORD(lParam) }; // Get click coordinates
        HWND hitWnd = ChildWindowFromPoint(hwnd, pt);  // Find clicked control
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    case WM_CREATE: {
        // Initialize common controls for list views
        INITCOMMONCONTROLSEX icex = { sizeof(INITCOMMONCONTROLSEX), ICC_LISTVIEW_CLASSES | ICC_STANDARD_CLASSES };
        InitCommonControlsEx(&icex);

        // Create brushes for UI elements
        g_hOutput = CreateSolidBrush(RGB(0, 0, 0));
        g_hFormBackground = CreateSolidBrush(GetSysColor(COLOR_3DFACE));

        // Create UI components
        UIComponents::createSidebar(hwnd, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE));
        UIServers::createServerPage(hwnd, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE));
        UIRcon::createRconPage(hwnd, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE));
        UIComponents::createOutputBox(hwnd, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE));

        // Fallback: Set icons in WM_CREATE
        HICON hIcon = LoadIcon((HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), MAKEINTRESOURCE(IDI_ICON1));
        if (hIcon) {
            SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
            SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
        }
        else {
            char error[256];
            sprintf_s(error, "Failed to load icon in WM_CREATE: %lu", GetLastError());
            ServerManager::logDebug(error);
        }

        // Show server page by default
        hideAllPageControls(hwnd, true);
        UIServers::showServerPage(hwnd);
        InvalidateRect(hwnd, nullptr, TRUE);
        UpdateWindow(hwnd);
        break;
    }

    case WM_ERASEBKGND: {
        HDC hdc = (HDC)wParam;
        RECT rect;
        GetClientRect(hwnd, &rect);
        FillRect(hdc, &rect, (HBRUSH)(COLOR_3DFACE + 1)); // Fill background
        return 1;
    }

    case WM_CTLCOLORSTATIC: {
        HDC hdc = (HDC)wParam;
        HWND hControl = (HWND)lParam;
        int ctrlId = GetDlgCtrlID(hControl);

        // Style output box (ID 400)
        if (ctrlId == 400) {
            SetBkColor(hdc, RGB(0, 0, 0));
            SetTextColor(hdc, RGB(255, 255, 255));
            return (LRESULT)g_hOutput;
        }

        // Style server form labels (IDs 312–318)
        if (ctrlId >= 312 && ctrlId <= 318) {
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(0, 0, 0));
            return (LRESULT)g_hFormBackground;
        }

        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    case WM_COMMAND: {
        // Handle sidebar navigation
        if (LOWORD(wParam) == 100) { // Servers button
            if (!isServerPage) {
                isServerPage = true;
                hideAllPageControls(hwnd, true);
                UIServers::showServerPage(hwnd);
                // Ensure server table and form are on top
                SetWindowPos(GetDlgItem(hwnd, 200), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                SetWindowPos(GetDlgItem(hwnd, 310), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                InvalidateRect(hwnd, nullptr, TRUE);
                UpdateWindow(hwnd);
            }
        }
        else if (LOWORD(wParam) == 101) { // RCON button
            if (isServerPage) {
                isServerPage = false;
                hideAllPageControls(hwnd, false);
                UIRcon::showRconPage(hwnd);
                // Ensure player table and settings are on top
                SetWindowPos(GetDlgItem(hwnd, 501), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                SetWindowPos(GetDlgItem(hwnd, 504), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                InvalidateRect(hwnd, nullptr, TRUE);
                UpdateWindow(hwnd);
            }
        }

        // Delegate to page-specific handlers
        if (isServerPage) {
            ServerPage::handleServerPage(hwnd, msg, wParam, lParam);
        }
        else {
            RconPage::handleRconPage(hwnd, msg, wParam, lParam);
        }
        break;
    }

    case WM_NOTIFY: {
        LPNMHDR nmhdr = (LPNMHDR)lParam;
        // Handle notifications for server table (ID 200)
        if (nmhdr->idFrom == 200 && isServerPage) {
            ServerPage::handleServerPage(hwnd, msg, wParam, lParam);
        }
        // Handle notifications for player table (ID 501)
        else if (nmhdr->idFrom == 501 && !isServerPage) {
            RconPage::handleRconPage(hwnd, msg, wParam, lParam);
        }
        break;
    }

    case WM_TIMER: {
        // Handle timers for RCON page
        if (!isServerPage) {
            RconPage::handleRconPage(hwnd, msg, wParam, lParam);
        }
        break;
    }

    case WM_DESTROY: {
        // Clean up brushes
        if (g_hOutput) {
            DeleteObject(g_hOutput);
            g_hOutput = nullptr;
        }
        if (g_hFormBackground) {
            DeleteObject(g_hFormBackground);
            g_hFormBackground = nullptr;
        }
        PostQuitMessage(0); // Signal application exit
        break;
    }

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// Application entry point.
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
    // Register window class using WNDCLASSEX
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"RconApp";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1)); // Load large icon (title bar)
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
    wc.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1)); // Load small icon (taskbar, Alt+Tab)

    // Log icon loading errors
    if (!wc.hIcon) {
        char error[256];
        sprintf_s(error, "Failed to load large icon: %lu", GetLastError());
        ServerManager::logDebug(error);
    }
    if (!wc.hIconSm) {
        char error[256];
        sprintf_s(error, "Failed to load small icon: %lu", GetLastError());
        ServerManager::logDebug(error);
    }

    if (!RegisterClassEx(&wc)) {
        char error[256];
        sprintf_s(error, "Failed to register window class: %lu", GetLastError());
        ServerManager::logDebug(error);
        return 0;
    }

    // Create main window
    HWND hwnd = CreateWindowEx(0, L"RconApp", L"XRcon", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT, 1250, 650, nullptr, nullptr, hInstance, nullptr);

    if (!hwnd) {
        char error[256];
        sprintf_s(error, "Failed to create window: %lu", GetLastError());
        ServerManager::logDebug(error);
        return 0;
    }

    // Fallback: Set icons directly on the window
    HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    if (hIcon) {
        SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);   // Large icon for title bar
        SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon); // Small icon for taskbar
    }
    else {
        char error[256];
        sprintf_s(error, "Failed to load icon for WM_SETICON: %lu", GetLastError());
        ServerManager::logDebug(error);
    }

    // Show and update window
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // Message loop
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (!IsDialogMessage(hwnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return (int)msg.wParam;
}