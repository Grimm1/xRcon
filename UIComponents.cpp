// --- xRcon\UIComponents.cpp ---
// Implementation of common UI components for the application.
// Provides functionality to create a sidebar with navigation buttons and an output box for messages.

#include "UIComponents.h"
#include <commctrl.h>

#pragma comment(lib, "comctl32.lib") // Link Common Controls library

// Static UI control handle
static HWND outputBox = nullptr; // Edit box for displaying output messages

// Creates the sidebar with navigation buttons and version label.
void UIComponents::createSidebar(HWND hwnd, HINSTANCE hInstance) {
    // Create "Servers" navigation button
    CreateWindow(L"BUTTON", L"Servers", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        15, 15, 120, 120, hwnd, (HMENU)100, hInstance, nullptr);

    // Create "RCON" navigation button
    CreateWindow(L"BUTTON", L"RCON", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        15, 145, 120, 120, hwnd, (HMENU)101, hInstance, nullptr);

    CreateWindow(L"STATIC", L"Medal of Honor and Spearhead require a custom server side script to use the Rename and Unbind functions, these are included in the program directory ", WS_CHILD | WS_VISIBLE,
        800, 490, 400, 80, hwnd, (HMENU)104, hInstance, nullptr);



    // Create version label
    CreateWindow(L"STATIC", L"Version:0.05", WS_CHILD | WS_VISIBLE,
        1145, 590, 100, 20, hwnd, (HMENU)103, hInstance, nullptr);
}

// Creates the output box for displaying messages.
void UIComponents::createOutputBox(HWND hwnd, HINSTANCE hInstance) {
    // Create multiline, read-only edit box with scrollbars
    outputBox = CreateWindow(L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL,
        160, 495, 600, 100, hwnd, (HMENU)400, hInstance, nullptr);

    if (outputBox) {
        SendMessage(outputBox, EM_SETLIMITTEXT, 32768, 0); // Set text limit to 32KB
    }
}

// Sets the message displayed in the output box.
void UIComponents::setOutputMessage(HWND hwnd, const char* message) {
    HWND hwndOutputBox = GetDlgItem(hwnd, 400); // Get output box handle
    if (hwndOutputBox) {
        WCHAR buffer[4096];
        // Convert UTF-8 message to wide string
        MultiByteToWideChar(CP_UTF8, 0, message, -1, buffer, sizeof(buffer) / sizeof(WCHAR));
        SendMessage(hwndOutputBox, WM_SETTEXT, 0, (LPARAM)buffer); // Set message text
    }
}