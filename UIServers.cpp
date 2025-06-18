// --- xRcon\UIServers.cpp ---
// Implementation of the server management UI page.
// Provides functionality to create and manage a UI for adding, editing, and displaying game servers.

#include "UIServers.h"
#include "UIComponents.h"
#include <commctrl.h>
#include <vector>
#include <string>

#pragma comment(lib, "comctl32.lib") // Link Common Controls library

// Static UI control handles
static HWND serverTable = nullptr;   // List view for server list
static HWND gameCombo = nullptr;     // Combo box for game selection
static bool serverPageCreated = false; // Flag to track if server page is created

// Creates the server table (list view) to display server information.
void UIServers::createServerTable(HWND hwnd, HINSTANCE hInstance) {
    InitCommonControls(); // Initialize common controls for list view

    // Create server table with report style and single selection
    serverTable = CreateWindow(WC_LISTVIEW, L"", WS_CHILD | LVS_REPORT | LVS_SINGLESEL,
        160, 0, 600, 400, hwnd, (HMENU)200, hInstance, nullptr);

    // Enable full row selection, grid lines, and header drag-drop
    ListView_SetExtendedListViewStyle(serverTable, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP);

    // Initialize table columns
    LVCOLUMN col = { 0 };
    col.mask = LVCF_TEXT | LVCF_WIDTH;

    WCHAR name[] = L"Name";
    WCHAR ip[] = L"IP/Hostname";
    WCHAR port[] = L"Port";
    WCHAR game[] = L"Game";
    WCHAR edit[] = L"Edit";
    WCHAR del[] = L"Delete";

    col.cx = 100; col.pszText = name; ListView_InsertColumn(serverTable, 0, &col);
    col.cx = 150; col.pszText = ip; ListView_InsertColumn(serverTable, 1, &col);
    col.cx = 60; col.pszText = port; ListView_InsertColumn(serverTable, 2, &col);
    col.cx = 150; col.pszText = game; ListView_InsertColumn(serverTable, 3, &col);
    col.cx = 70; col.pszText = edit; ListView_InsertColumn(serverTable, 4, &col);
    col.cx = 70; col.pszText = del; ListView_InsertColumn(serverTable, 5, &col);

    updateServerTable(hwnd); // Populate table with server data
}

// Creates the form for adding or editing server details.
void UIServers::createServerForm(HWND hwnd, HINSTANCE hInstance) {
    // Layout coordinates for form controls
    int x = 770, y = 0, w = 390, h = 25;

    // Create group box for server form
    CreateWindow(L"BUTTON", L"Add Server", WS_CHILD | BS_GROUPBOX,
        x + 20, y + 10, w + 40, 440, hwnd, (HMENU)310, hInstance, nullptr);

    x += 30;
    y += 50;

    // Create server name input
    CreateWindow(L"STATIC", L"Server Name:", WS_CHILD, x, y + 4, 180, h, hwnd, (HMENU)312, hInstance, nullptr);
    CreateWindow(L"EDIT", L"", WS_CHILD | WS_BORDER, x + 130, y, 280, h, hwnd, (HMENU)300, hInstance, nullptr);
    y += 30;

    // Create IP/hostname input
    CreateWindow(L"STATIC", L"IP/Hostname:", WS_CHILD, x, y + 4, 180, h, hwnd, (HMENU)313, hInstance, nullptr);
    CreateWindow(L"EDIT", L"", WS_CHILD | WS_BORDER, x + 130, y, 280, h, hwnd, (HMENU)301, hInstance, nullptr);
    y += 30;

    // Create port input
    CreateWindow(L"STATIC", L"Port:", WS_CHILD, x, y + 4, 180, h, hwnd, (HMENU)314, hInstance, nullptr);
    CreateWindow(L"EDIT", L"", WS_CHILD | WS_BORDER, x + 130, y, 280, h, hwnd, (HMENU)302, hInstance, nullptr);
    y += 30;

    // Create game selection combo box
    CreateWindow(L"STATIC", L"Game:", WS_CHILD, x, y + 4, 180, h, hwnd, (HMENU)315, hInstance, nullptr);
    gameCombo = CreateWindow(WC_COMBOBOX, L"", WS_CHILD | CBS_DROPDOWNLIST | WS_VSCROLL,
        x + 130, y, 280, 200, hwnd, (HMENU)303, hInstance, nullptr);
    y += 30;

    // Create RCON password input
    CreateWindow(L"STATIC", L"RCON Password:", WS_CHILD, x, y + 4, 180, h, hwnd, (HMENU)316, hInstance, nullptr);
    CreateWindow(L"EDIT", L"", WS_CHILD | WS_BORDER, x + 130, y, 280, h, hwnd, (HMENU)304, hInstance, nullptr);
    y += 30;

    // Create gametypes input (multiline)
    CreateWindow(L"STATIC", L"Gametypes:", WS_CHILD, x, y + 4, 180, h, hwnd, (HMENU)317, hInstance, nullptr);
    CreateWindow(L"EDIT", L"", WS_CHILD | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN | WS_VSCROLL,
        x + 130, y, 280, h * 3, hwnd, (HMENU)305, hInstance, nullptr);
    HWND hwndGametypes = GetDlgItem(hwnd, 305);
    SendMessage(hwndGametypes, EM_SETLIMITTEXT, 4096, 0); // Limit to 4096 characters
    y += 30 * 3;

    // Create maps input (multiline)
    CreateWindow(L"STATIC", L"Maps:", WS_CHILD, x, y + 4, 180, h, hwnd, (HMENU)318, hInstance, nullptr);
    CreateWindow(L"EDIT", L"", WS_CHILD | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN | WS_VSCROLL,
        x + 130, y, 280, h * 3, hwnd, (HMENU)306, hInstance, nullptr);
    HWND hwndMaps = GetDlgItem(hwnd, 306);
    SendMessage(hwndMaps, EM_SETLIMITTEXT, 4096, 0); // Limit to 4096 characters
    y += 30 * 3;

    // Create save server button
    CreateWindow(L"BUTTON", L"Save Server", WS_CHILD | BS_PUSHBUTTON, x + 310, y, 100, h, hwnd, (HMENU)307, hInstance, nullptr);

    // Populate game combo box with available games
    std::vector<std::wstring> gameStrings;
    auto games = ServerManager::getGameOptions();
    for (const auto& game : games) {
        gameStrings.emplace_back(game.first.begin(), game.first.end());
        SendMessage(gameCombo, CB_ADDSTRING, 0, (LPARAM)gameStrings.back().c_str());
    }
    SendMessage(gameCombo, CB_SETCURSEL, 0, 0); // Select first game

    // Refresh multiline input controls
    UpdateWindow(hwndGametypes);
    UpdateWindow(hwndMaps);
}

// Updates the server table with current server data.
void UIServers::updateServerTable(HWND hwnd) {
    ListView_DeleteAllItems(serverTable); // Clear existing items
    auto servers = ServerManager::loadServers(); // Load server list
    int index = 0;

    std::vector<std::wstring> strings;
    for (const auto& s : servers) {
        WCHAR buffer[4096];
        // Convert server name to wide string
        MultiByteToWideChar(CP_UTF8, 0, s.name.c_str(), -1, buffer, sizeof(buffer) / sizeof(WCHAR));
        strings.emplace_back(buffer);
        // Convert IP/hostname to wide string
        MultiByteToWideChar(CP_UTF8, 0, s.ipOrHostname.c_str(), -1, buffer, sizeof(buffer) / sizeof(WCHAR));
        strings.emplace_back(buffer);
        // Convert port to wide string
        strings.emplace_back(std::to_wstring(s.port));
        // Convert game name to wide string
        MultiByteToWideChar(CP_UTF8, 0, s.game.c_str(), -1, buffer, sizeof(buffer) / sizeof(WCHAR));
        strings.emplace_back(buffer);
        strings.emplace_back(L"Edit");
        strings.emplace_back(L"Delete");

        // Add server to table
        LVITEM item = { 0 };
        item.mask = LVIF_TEXT;
        item.iItem = index++;
        item.pszText = (LPWSTR)strings[strings.size() - 6].c_str();
        ListView_InsertItem(serverTable, &item);
        ListView_SetItemText(serverTable, index - 1, 1, (LPWSTR)strings[strings.size() - 5].c_str());
        ListView_SetItemText(serverTable, index - 1, 2, (LPWSTR)strings[strings.size() - 4].c_str());
        ListView_SetItemText(serverTable, index - 1, 3, (LPWSTR)strings[strings.size() - 3].c_str());
        ListView_SetItemText(serverTable, index - 1, 4, (LPWSTR)strings[strings.size() - 2].c_str());
        ListView_SetItemText(serverTable, index - 1, 5, (LPWSTR)strings[strings.size() - 1].c_str());
    }
}

// Creates the server management page by initializing the table and form.
void UIServers::createServerPage(HWND hwnd, HINSTANCE hInstance) {
    if (serverPageCreated) {
        return; // Prevent multiple creations
    }

    createServerTable(hwnd, hInstance); // Create server table
    createServerForm(hwnd, hInstance);  // Create server form
    serverPageCreated = true;
}

// Shows the server page by making all controls visible.
void UIServers::showServerPage(HWND hwnd) {
    if (!serverPageCreated) {
        createServerPage(hwnd, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE)); // Create page if not exists
    }

    // Show all controls
    ShowWindow(GetDlgItem(hwnd, 200), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, 300), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, 301), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, 302), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, 303), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, 304), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, 305), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, 306), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, 307), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, 310), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, 312), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, 313), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, 314), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, 315), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, 316), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, 317), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, 318), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, 400), SW_SHOW); 
    InvalidateRect(hwnd, nullptr, TRUE); // Redraw window
    UpdateWindow(hwnd);
}

// Hides the server page (currently a no-op if page is created).
void UIServers::hideServerPage(HWND hwnd) {
    if (serverPageCreated) {
        // No controls are hidden (placeholder for future use)
    }
}