// --- xRcon\UIRcon.cpp ---
// Implementation of the RCON (Remote Console) UI for managing game servers.
// Provides functionality to create and manage a UI page for server selection, player information, and server settings.

#include "UIRcon.h"
#include "UIComponents.h"
#include "GameServerQuery.h"
#include "ServerManager.h"
#include <commctrl.h>
#include <vector>
#include <sstream>
#include <string>

#pragma comment(lib, "comctl32.lib") // Link Common Controls library
#pragma comment(lib, "Ws2_32.lib")   // Link Winsock library
#pragma comment(lib, "GameServerQuery.lib") // Link GameServerQuery library

// Static UI control handles
static HWND playerTable = nullptr;       // List view for player information
static HWND serverCombo = nullptr;       // Combo box for server selection
static HWND commandInput = nullptr;      // Edit box for RCON commands
static HWND serverInfoStatic = nullptr;  // Group box for server settings
static HWND sendButton = nullptr;        // Button to send RCON commands
static HWND hostnameInput = nullptr;     // Edit box for server hostname
static HWND hostnameApply = nullptr;     // Button to apply hostname changes
static HWND mapSelector = nullptr;       // Combo box for map selection
static HWND mapApply = nullptr;          // Button to apply map changes
static HWND gametypeSelector = nullptr;  // Combo box for gametype selection
static HWND gametypeApply = nullptr;     // Button to apply gametype changes
static HWND playersLabel = nullptr;      // Label for player count
static HWND restartButton = nullptr;     // Button for server restart
static HWND fastRestartButton = nullptr; // Button for fast server restart
static HWND mapRotateButton = nullptr;   // Button for map rotation
static bool rconPageCreated = false;     // Flag to track if RCON page is created
static const UINT_PTR REFRESH_TIMER_ID = 1001; // Timer ID for periodic refresh
static std::vector<WCHAR*> mapData;      // Stores map names for combo box
static std::vector<WCHAR*> gametypeData; // Stores gametype names for combo box
static std::string lastGame;             // Tracks last game type for column preservation

// Creates the RCON UI page with controls for server management.
void UIRcon::createRconPage(HWND hwnd, HINSTANCE hInstance) {
    if (rconPageCreated) {
        return; // Prevent multiple creations
    }

    // Create server selection combo box
    serverCombo = CreateWindow(WC_COMBOBOX, L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        160, 10, 600, 200, hwnd, (HMENU)500, hInstance, nullptr);

    // Create player table (list view)
    playerTable = CreateWindow(WC_LISTVIEW, L"", WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | WS_TABSTOP,
        160, 50, 600, 400, hwnd, (HMENU)501, hInstance, nullptr);
    if (playerTable) {
        // Enable full row selection, grid lines, and header drag-drop
        ListView_SetExtendedListViewStyle(playerTable, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP);
        EnableWindow(playerTable, TRUE);
    }

    // Initialize player table columns
    LVCOLUMN col = { 0 };
    col.mask = LVCF_TEXT | LVCF_WIDTH;
    WCHAR headers[][20] = { L"Num", L"Name", L"IP", L"Score", L"Ping" };
    int widths[] = { 40, 140, 140, 45, 45 };
    for (int i = 0; i < 5; ++i) {
        col.cx = widths[i];
        col.pszText = headers[i];
        if (playerTable) {
            ListView_InsertColumn(playerTable, i, &col);
        }
    }

    // Create server settings group box
    serverInfoStatic = CreateWindow(L"BUTTON", L"Server Settings", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        790, 10, 440, 440, hwnd, (HMENU)504, hInstance, nullptr);

    // Layout coordinates for server settings controls
    int x = 820, y = 40, w = 380, h = 25;

    // Create hostname controls
    CreateWindow(L"STATIC", L"Hostname:", WS_CHILD, x, y + 4, 100, h, hwnd, (HMENU)510, hInstance, nullptr);
    hostnameInput = CreateWindow(L"EDIT", L"", WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
        x + 110, y, w - 190, h, hwnd, (HMENU)511, hInstance, nullptr);
    hostnameApply = CreateWindow(L"BUTTON", L"Apply", WS_CHILD | BS_PUSHBUTTON,
        x + w - 70, y, 70, h, hwnd, (HMENU)512, hInstance, nullptr);
    y += 40;

    // Create map selection controls
    CreateWindow(L"STATIC", L"Map:", WS_CHILD, x, y + 4, 100, h, hwnd, (HMENU)513, hInstance, nullptr);
    mapSelector = CreateWindow(WC_COMBOBOX, L"", WS_CHILD | CBS_DROPDOWNLIST | WS_VSCROLL,
        x + 110, y, w - 190, 200, hwnd, (HMENU)514, hInstance, nullptr);
    mapApply = CreateWindow(L"BUTTON", L"Apply", WS_CHILD | BS_PUSHBUTTON,
        x + w - 70, y, 70, h, hwnd, (HMENU)515, hInstance, nullptr);
    y += 40;

    // Create gametype selection controls
    CreateWindow(L"STATIC", L"Gametype:", WS_CHILD, x, y + 4, 100, h, hwnd, (HMENU)516, hInstance, nullptr);
    gametypeSelector = CreateWindow(WC_COMBOBOX, L"", WS_CHILD | CBS_DROPDOWNLIST | WS_VSCROLL,
        x + 110, y, w - 190, 200, hwnd, (HMENU)517, hInstance, nullptr);
    gametypeApply = CreateWindow(L"BUTTON", L"Apply", WS_CHILD | BS_PUSHBUTTON,
        x + w - 70, y, 70, h, hwnd, (HMENU)518, hInstance, nullptr);
    y += 40;

    // Create player count label
    CreateWindow(L"STATIC", L"Players:", WS_CHILD, x, y + 4, 100, h, hwnd, (HMENU)519, hInstance, nullptr);
    playersLabel = CreateWindow(L"STATIC", L"0/0", WS_CHILD, x + 110, y + 4, 100, h, hwnd, (HMENU)520, hInstance, nullptr);
    y += 40;

    // Create server action buttons
    restartButton = CreateWindow(L"BUTTON", L"Restart", WS_CHILD | BS_PUSHBUTTON,
        x, y, 100, h, hwnd, (HMENU)521, hInstance, nullptr);
    fastRestartButton = CreateWindow(L"BUTTON", L"Fast Restart", WS_CHILD | BS_PUSHBUTTON,
        x + 110, y, 100, h, hwnd, (HMENU)522, hInstance, nullptr);
    mapRotateButton = CreateWindow(L"BUTTON", L"Map Rotate", WS_CHILD | BS_PUSHBUTTON,
        x + 220, y, 100, h, hwnd, (HMENU)523, hInstance, nullptr);

    // Create RCON command input
    commandInput = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        160, 460, 475, 25, hwnd, (HMENU)502, hInstance, nullptr);
    if (commandInput) {
        SendMessage(commandInput, EM_SETLIMITTEXT, 1024, 0); // Limit input to 1024 characters
    }

    // Create send command button
    sendButton = CreateWindow(L"BUTTON", L"Send Command", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        640, 460, 120, 25, hwnd, (HMENU)503, hInstance, nullptr);

    rconPageCreated = true;
    SetTimer(hwnd, REFRESH_TIMER_ID, 60000, nullptr); // Start 60-second refresh timer
    updateServerSelector(hwnd); // Populate server list
}

// Shows the RCON page by making all controls visible.
void UIRcon::showRconPage(HWND hwnd) {
    if (!rconPageCreated) {
        createRconPage(hwnd, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE)); // Create page if not exists
    }

    // Show all controls
    ShowWindow(GetDlgItem(hwnd, 500), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, 501), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, 502), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, 503), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, 504), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, 510), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, 511), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, 512), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, 513), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, 514), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, 515), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, 516), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, 517), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, 518), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, 519), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, 520), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, 521), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, 522), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, 523), SW_SHOW);
    ShowWindow(GetDlgItem(hwnd, 400), SW_SHOW); // Unknown control (possibly parent or output)

    // Ensure player table is focused and on top
    SetWindowPos(GetDlgItem(hwnd, 501), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    EnableWindow(GetDlgItem(hwnd, 501), TRUE);
    SetFocus(GetDlgItem(hwnd, 501));

    updateServerSelector(hwnd); // Refresh server list
    InvalidateRect(hwnd, nullptr, TRUE); // Redraw window

    // Update selected server's player table and settings
    HWND hwndServerCombo = GetDlgItem(hwnd, 500);
    if (hwndServerCombo) {
        int index = static_cast<int>(SendMessage(hwndServerCombo, CB_GETCURSEL, 0, 0));
        if (index != CB_ERR) {
            auto servers = ServerManager::loadServers();
            if (index < static_cast<int>(servers.size())) {
                UIRcon::updatePlayerTable(hwnd, servers[index]);
                UIRcon::updateServerSettings(hwnd, servers[index]);
            }
        }
    }
}

// Hides the RCON page by stopping the refresh timer.
void UIRcon::hideRconPage(HWND hwnd) {
    if (rconPageCreated) {
        KillTimer(hwnd, REFRESH_TIMER_ID); // Stop refresh timer
    }
}

// Updates the server selection combo box with available servers.
void UIRcon::updateServerSelector(HWND hwnd) {
    HWND hwndServerCombo = GetDlgItem(hwnd, 500);
    if (!hwndServerCombo) {
        return; // Combo box not found
    }

    SendMessage(hwndServerCombo, CB_RESETCONTENT, 0, 0); // Clear existing items
    auto servers = ServerManager::loadServers(); // Load server list

    std::vector<std::wstring> serverNames;
    for (const auto& server : servers) {
        if (server.name.empty()) {
            continue; // Skip servers with empty names
        }
        WCHAR buffer[4096];
        int len = MultiByteToWideChar(CP_UTF8, 0, server.name.c_str(), -1, buffer, sizeof(buffer) / sizeof(WCHAR));
        if (len == 0) {
            continue; // Skip if conversion fails
        }
        serverNames.emplace_back(buffer);
        SendMessage(hwndServerCombo, CB_ADDSTRING, 0, (LPARAM)serverNames.back().c_str());
    }

    if (!servers.empty()) {
        SendMessage(hwndServerCombo, CB_SETCURSEL, 0, 0); // Select first server
        updatePlayerTable(hwnd, servers[0]); // Update player table
        updateServerSettings(hwnd, servers[0]); // Update server settings
        EnableWindow(GetDlgItem(hwnd, 503), TRUE); // Enable send button
    }
    else {
        ListView_DeleteAllItems(GetDlgItem(hwnd, 501)); // Clear player table
        EnableWindow(GetDlgItem(hwnd, 503), TRUE); // Disable send button
        for (int id = 510; id <= 523; ++id) {
            EnableWindow(GetDlgItem(hwnd, id), TRUE); // Disable settings controls
        }
    }
    InvalidateRect(hwndServerCombo, nullptr, TRUE); // Redraw combo box
    UpdateWindow(hwndServerCombo);
}

// Schedules a refresh of server data using a timer.
void UIRcon::scheduleRefresh(HWND hwnd, const Server& server) {
    SetTimer(hwnd, REFRESH_TIMER_ID, 60000, nullptr); // Set 60-second timer
}

// Updates server settings controls (hostname, map, gametype, players).
void UIRcon::updateServerSettings(HWND hwnd, const Server& server) {
    // Get control handles
    HWND hwndHostnameInput = GetDlgItem(hwnd, 511);
    HWND hwndMapSelector = GetDlgItem(hwnd, 514);
    HWND hwndGametypeSelector = GetDlgItem(hwnd, 517);
    HWND hwndPlayersLabel = GetDlgItem(hwnd, 520);

    if (!hwndHostnameInput || !hwndMapSelector || !hwndGametypeSelector || !hwndPlayersLabel) {
        scheduleRefresh(hwnd, server); // Retry later if controls are missing
        return;
    }

    // Determine game type for conditional logic
    bool isMOHAA = server.game == "Medal of Honor: Allied Assault";
    bool isMOHSHBT = server.game == "Medal of Honor: AA Spearhead" || server.game == "Medal of Honor: AA Breakthrough";
    bool isCOD = server.game.find("Call of Duty") != std::string::npos;
    bool isCOD2Plus = server.game == "Call of Duty 2" || server.game == "Call of Duty 4: Modern Warfare" || server.game == "Call of Duty: World at War";

    // Show/hide action buttons based on game type
    ShowWindow(GetDlgItem(hwnd, 521), isMOHAA || isMOHSHBT || isCOD ? SW_SHOW : SW_HIDE); // Restart
    ShowWindow(GetDlgItem(hwnd, 522), isCOD2Plus ? SW_SHOW : SW_HIDE); // Fast Restart
    ShowWindow(GetDlgItem(hwnd, 523), isCOD && !isMOHAA && !isMOHSHBT ? SW_SHOW : SW_HIDE); // Map Rotate

    // Query server status
    const char* response = ProcessGameServerCommand(
        server.protocolId,
        false,
        server.ipOrHostname.c_str(),
        server.port,
        "getstatus",
        ""
    );

    if (!response || strncmp(response, "error=", 6) == 0) {
        std::string error = response ? response : "No response from server";
        UIComponents::setOutputMessage(hwnd, ("Failed to fetch server status: " + error).c_str());
        if (response) FreeGameServerResponse(response);
        scheduleRefresh(hwnd, server);
        return;
    }

    std::string json = response;
    FreeGameServerResponse(response);

    // Helper to extract JSON fields
    auto extractField = [](const std::string& json, const std::string& field) {
        size_t pos = json.find("\"" + field + "\":");
        if (pos == std::string::npos) return std::string();
        pos += field.length() + 3;
        if (json[pos] == '"') {
            pos++;
            size_t end = json.find('"', pos);
            return json.substr(pos, end - pos);
        }
        else {
            size_t end = json.find_first_of(",}", pos);
            return json.substr(pos, end - pos);
        }
        };

    // Extract server information
    std::string hostname = extractField(json, "sv_hostname");
    std::string mapname = extractField(json, "mapname");
    std::string gametype = extractField(json, isMOHAA ? "g_gametypestring" : "g_gametype");
    std::string maxclients = extractField(json, "sv_maxclients");
    size_t playerCount = 0;
    size_t playersPos = json.find("\"players\":");
    if (playersPos != std::string::npos) {
        std::string playersJson = json.substr(playersPos + 10);
        if (playersJson != "[]") {
            for (size_t i = 0; i < playersJson.length(); ++i) {
                if (playersJson[i] == '{') ++playerCount; // Count player objects
            }
        }
    }

    // Update hostname input
    WCHAR buffer[4096];
    MultiByteToWideChar(CP_UTF8, 0, hostname.c_str(), -1, buffer, sizeof(buffer) / sizeof(WCHAR));
    SetWindowTextW(hwndHostnameInput, buffer);

    // Clear existing map and gametype data
    for (WCHAR* ptr : mapData) free(ptr);
    for (WCHAR* ptr : gametypeData) free(ptr);
    mapData.clear();
    gametypeData.clear();

    // Populate map selector
    SendMessage(hwndMapSelector, CB_RESETCONTENT, 0, 0);
    std::map<std::string, std::string> mapList = ServerManager::parseList(server.maps);
    int selectedMapIndex = -1, index = 0;
    for (const auto& pair : mapList) {
        MultiByteToWideChar(CP_UTF8, 0, pair.second.c_str(), -1, buffer, sizeof(buffer) / sizeof(WCHAR));
        SendMessage(hwndMapSelector, CB_ADDSTRING, 0, (LPARAM)buffer);
        WCHAR* data = _wcsdup(buffer);
        SendMessage(hwndMapSelector, CB_SETITEMDATA, index, (LPARAM)data);
        mapData.push_back(data);
        if (pair.first == mapname) selectedMapIndex = index;
        ++index;
    }
    if (selectedMapIndex >= 0) {
        SendMessage(hwndMapSelector, CB_SETCURSEL, selectedMapIndex, 0);
    }

    // Populate gametype selector
    SendMessage(hwndGametypeSelector, CB_RESETCONTENT, 0, 0);
    std::map<std::string, std::string> gametypeList = ServerManager::parseList(server.gametypes);
    int selectedGametypeIndex = -1;
    index = 0;
    for (const auto& pair : gametypeList) {
        MultiByteToWideChar(CP_UTF8, 0, pair.second.c_str(), -1, buffer, sizeof(buffer) / sizeof(WCHAR));
        SendMessage(hwndGametypeSelector, CB_ADDSTRING, 0, (LPARAM)buffer);
        WCHAR* data = _wcsdup(buffer);
        SendMessage(hwndGametypeSelector, CB_SETITEMDATA, index, (LPARAM)data);
        gametypeData.push_back(data);
        if (pair.first == gametype || (isMOHAA && pair.second == gametype)) selectedGametypeIndex = index;
        ++index;
    }
    if (selectedGametypeIndex >= 0) {
        SendMessage(hwndGametypeSelector, CB_SETCURSEL, selectedGametypeIndex, 0);
    }

    // Update player count label
    std::string playersText = std::to_string(playerCount) + "/" + maxclients;
    MultiByteToWideChar(CP_UTF8, 0, playersText.c_str(), -1, buffer, sizeof(buffer) / sizeof(WCHAR));
    SetWindowTextW(hwndPlayersLabel, buffer);

    scheduleRefresh(hwnd, server); // Schedule next refresh
}

// Updates the player table with current server player data.
void UIRcon::updatePlayerTable(HWND hwnd, const Server& server) {
    Sleep(500); // Brief delay to handle server response timing
    HWND hwndPlayerTable = GetDlgItem(hwnd, 501);
    if (!hwndPlayerTable) {
        return; // Player table not found
    }

    // Recreate columns if game type changes
    if (lastGame != server.game) {
        ListView_DeleteAllItems(hwndPlayerTable);
        while (ListView_DeleteColumn(hwndPlayerTable, 0)) {}
        LVCOLUMN col = { 0 };
        col.mask = LVCF_TEXT | LVCF_WIDTH;
        WCHAR headers[][20] = { L"Num", L"Name", L"IP", L"Score", L"Ping" };
        int widths[] = { 40, 140, 140, 45, 45 };
        int colCount = 5;
        for (int i = 0; i < 5; ++i) {
            col.cx = widths[i];
            col.pszText = headers[i];
            ListView_InsertColumn(hwndPlayerTable, i, &col);
        }

        // Add game-specific action columns
        WCHAR renameText[] = L"Rename";
        WCHAR unbindText[] = L"Unbind";
        WCHAR kickText[] = L"Kick";
        WCHAR banText[] = L"Ban";
        bool isMOHAA = server.game == "Medal of Honor: Allied Assault" || server.game == "Medal of Honor: AA Spearhead";
        bool isMOHSHBT = server.game == "Medal of Honor: AA Breakthrough";
        bool isCOD = server.game.find("Call of Duty") != std::string::npos;

        if (isMOHAA) {
            int actionWidths[] = { 60, 60, 60 };
            col.cx = actionWidths[0]; col.pszText = renameText; ListView_InsertColumn(hwndPlayerTable, colCount++, &col);
            col.cx = actionWidths[1]; col.pszText = unbindText; ListView_InsertColumn(hwndPlayerTable, colCount++, &col);
            col.cx = actionWidths[2]; col.pszText = kickText; ListView_InsertColumn(hwndPlayerTable, colCount++, &col);
        }
        else if (isMOHSHBT) {
            col.cx = 80; col.pszText = kickText; ListView_InsertColumn(hwndPlayerTable, colCount++, &col);
        }
        else if (isCOD) {
            int actionWidths[] = { 80, 80 };
            col.cx = actionWidths[0]; col.pszText = kickText; ListView_InsertColumn(hwndPlayerTable, colCount++, &col);
            col.cx = actionWidths[1]; col.pszText = banText; ListView_InsertColumn(hwndPlayerTable, colCount++, &col);
        }
        lastGame = server.game;
    }
    else {
        ListView_DeleteAllItems(hwndPlayerTable); // Clear items, preserve columns
    }

    // Validate server details
    if (server.ipOrHostname.empty() || server.port == 0) {
        UIComponents::setOutputMessage(hwnd, "Invalid server details: IP/hostname or port is invalid.");
        return;
    }

    // Query player status
    const char* response = ProcessGameServerCommand(
        server.protocolId,
        false,
        server.ipOrHostname.c_str(),
        server.port,
        "rcon status",
        server.rconPassword.empty() ? "" : server.rconPassword.c_str()
    );

    if (!response || strncmp(response, "error=", 6) == 0) {
        std::string error = response ? response : "No response from server";
        UIComponents::setOutputMessage(hwnd, ("Server may be OFFLINE or changing map: " + error).c_str());
        if (response) FreeGameServerResponse(response);
        return;
    }

    std::string json = response;
    FreeGameServerResponse(response);

    std::vector<std::wstring> strings;
    int index = 0;

    // Parse players array
    size_t playersPos = json.find("\"players\":");
    if (playersPos == std::string::npos) {
        UIComponents::setOutputMessage(hwnd, "Invalid response format");
        return;
    }
    json = json.substr(playersPos + 10);

    // Find the end of the players array, accounting for quoted strings
    size_t arrayEnd = std::string::npos;
    bool inQuotes = false;
    for (size_t i = 0; i < json.length(); ++i) {
        if (json[i] == '"' && (i == 0 || json[i - 1] != '\\')) {
            inQuotes = !inQuotes; // Toggle quote state
        }
        else if (json[i] == ']' && !inQuotes) {
            arrayEnd = i;
            break;
        }
    }
    if (arrayEnd == std::string::npos) {
        UIComponents::setOutputMessage(hwnd, "Invalid response format: Could not find array end");
        return;
    }
    json = json.substr(0, arrayEnd);

    if (json.empty() || json == "[]") {
        return; // No players to display
    }

    // Determine game type for action buttons
    bool isMOHAA = server.game == "Medal of Honor: Allied Assault" || server.game == "Medal of Honor: AA Spearhead";
    bool isMOHSHBT = server.game == "Medal of Honor: AA Breakthrough";
    bool isCOD = server.game.find("Call of Duty") != std::string::npos;

    // Process each player
    size_t start = 0;
    if (json[0] == '[') start = 1; // Skip opening bracket
    while (start < json.length()) {
        size_t objStart = json.find('{', start);
        if (objStart == std::string::npos) break;

        // Find the end of the current player object
        size_t objEnd = std::string::npos;
        int braceCount = 1;
        inQuotes = false;
        for (size_t i = objStart + 1; i < json.length(); ++i) {
            if (json[i] == '"' && (i == 0 || json[i - 1] != '\\')) {
                inQuotes = !inQuotes;
            }
            else if (!inQuotes) {
                if (json[i] == '{') ++braceCount;
                else if (json[i] == '}') {
                    --braceCount;
                    if (braceCount == 0) {
                        objEnd = i;
                        break;
                    }
                }
            }
        }
        if (objEnd == std::string::npos) {
            UIComponents::setOutputMessage(hwnd, "Invalid response format: Malformed player object");
            return;
        }

        std::string playerJson = json.substr(objStart, objEnd - objStart + 1);
        parseAndAddPlayer(hwnd, playerJson, index++, strings, isMOHAA, isMOHSHBT, isCOD);
        start = objEnd + 1;

        // Skip comma or whitespace
        while (start < json.length() && (json[start] == ',' || isspace(json[start]))) ++start;
    }
}

// Parses and adds a player to the player table.
void UIRcon::parseAndAddPlayer(HWND hwnd, std::string playerJson, int index, std::vector<std::wstring>& strings, bool isMOHAA, bool isMOHSHBT, bool isCOD) {
    HWND hwndPlayerTable = GetDlgItem(hwnd, 501);
    if (!hwndPlayerTable) {
        return; // Player table not found
    }

    WCHAR buffer[4096];
    std::string num, name, ip, score, ping;

    // Helper to extract JSON fields
    auto extractField = [](const std::string& json, const std::string& field) {
        size_t pos = json.find("\"" + field + "\":");
        if (pos == std::string::npos) return std::string();
        pos += field.length() + 3;
        if (json[pos] == '"') {
            pos++;
            size_t end = json.find('"', pos);
            return json.substr(pos, end - pos);
        }
        else {
            size_t end = json.find_first_of(",}", pos);
            return json.substr(pos, end - pos);
        }
        };

    // Extract player information
    num = extractField(playerJson, "slot");
    name = extractField(playerJson, "name");
    ip = extractField(playerJson, "address");
    score = extractField(playerJson, "score");
    ping = extractField(playerJson, "ping");

    if (num.empty() || name.empty()) {
        return; // Skip invalid player data
    }

    // Add player to table
    LVITEM item = { 0 };
    item.mask = LVIF_TEXT;
    item.iItem = index;

    MultiByteToWideChar(CP_UTF8, 0, num.c_str(), -1, buffer, sizeof(buffer) / sizeof(WCHAR));
    strings.emplace_back(buffer);
    item.pszText = (LPWSTR)strings.back().c_str();
    ListView_InsertItem(hwndPlayerTable, &item);

    MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, buffer, sizeof(buffer) / sizeof(WCHAR));
    strings.emplace_back(buffer);
    ListView_SetItemText(hwndPlayerTable, index, 1, (LPWSTR)strings.back().c_str());

    MultiByteToWideChar(CP_UTF8, 0, ip.c_str(), -1, buffer, sizeof(buffer) / sizeof(WCHAR));
    strings.emplace_back(buffer);
    ListView_SetItemText(hwndPlayerTable, index, 2, (LPWSTR)strings.back().c_str());

    MultiByteToWideChar(CP_UTF8, 0, score.c_str(), -1, buffer, sizeof(buffer) / sizeof(WCHAR));
    strings.emplace_back(buffer);
    ListView_SetItemText(hwndPlayerTable, index, 3, (LPWSTR)strings.back().c_str());

    MultiByteToWideChar(CP_UTF8, 0, ping.c_str(), -1, buffer, sizeof(buffer) / sizeof(WCHAR));
    strings.emplace_back(buffer);
    ListView_SetItemText(hwndPlayerTable, index, 4, (LPWSTR)strings.back().c_str());

    // Add game-specific action buttons
    WCHAR renameText[] = L"Rename";
    WCHAR unbindText[] = L"Unbind";
    WCHAR kickText[] = L"Kick";
    WCHAR banText[] = L"Ban";
    int colIndex = 5;
    if (isMOHAA) {
        strings.emplace_back(renameText); ListView_SetItemText(hwndPlayerTable, index, colIndex++, (LPWSTR)strings.back().c_str());
        strings.emplace_back(unbindText); ListView_SetItemText(hwndPlayerTable, index, colIndex++, (LPWSTR)strings.back().c_str());
        strings.emplace_back(kickText); ListView_SetItemText(hwndPlayerTable, index, colIndex, (LPWSTR)strings.back().c_str());
    }
    else if (isMOHSHBT) {
        strings.emplace_back(kickText); ListView_SetItemText(hwndPlayerTable, index, colIndex, (LPWSTR)strings.back().c_str());
    }
    else if (isCOD) {
        strings.emplace_back(kickText); ListView_SetItemText(hwndPlayerTable, index, colIndex++, (LPWSTR)strings.back().c_str());
        strings.emplace_back(banText); ListView_SetItemText(hwndPlayerTable, index, colIndex, (LPWSTR)strings.back().c_str());
    }
}