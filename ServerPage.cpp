// --- xRcon\ServerPage.cpp ---
#include "ServerPage.h"
#include "UIServers.h"
#include "ServerManager.h"
#include "UIComponents.h"
#include "UIRcon.h"
#include <commctrl.h>
#include <regex>

// Declaration for hideAllPageControls from main.cpp
extern void hideAllPageControls(HWND hwnd, bool showingServerPage);

void ServerPage::handleServerPage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static std::string editingServerName;
    if (msg == WM_COMMAND) {
        int id = LOWORD(wParam);
        if (id == 307) { // Save Server button
            WCHAR buffer[4096];
            char ansiBuffer[4096];
            Server server;

            GetDlgItemTextW(hwnd, 300, buffer, sizeof(buffer) / sizeof(WCHAR));
            WideCharToMultiByte(CP_UTF8, 0, buffer, -1, ansiBuffer, sizeof(ansiBuffer), nullptr, nullptr);
            server.name = ansiBuffer;
            server.name.erase(0, server.name.find_first_not_of(" \t\r\n"));
            server.name.erase(server.name.find_last_not_of(" \t\r\n") + 1);

            GetDlgItemTextW(hwnd, 301, buffer, sizeof(buffer) / sizeof(WCHAR));
            WideCharToMultiByte(CP_UTF8, 0, buffer, -1, ansiBuffer, sizeof(ansiBuffer), nullptr, nullptr);
            server.ipOrHostname = ansiBuffer;
            server.ipOrHostname.erase(0, server.ipOrHostname.find_first_not_of(" \t\r\n"));
            server.ipOrHostname.erase(server.ipOrHostname.find_last_not_of(" \t\r\n") + 1);

            GetDlgItemTextW(hwnd, 302, buffer, sizeof(buffer) / sizeof(WCHAR));
            try {
                server.port = std::stoi(std::wstring(buffer));
            }
            catch (...) {
                server.port = -1;
            }

            int gameIndex = static_cast<int>(SendDlgItemMessage(hwnd, 303, CB_GETCURSEL, 0, 0));
            auto games = ServerManager::getGameOptions();
            if (gameIndex >= 0 && gameIndex < static_cast<int>(games.size())) {
                server.game = games[gameIndex].first;
                server.protocolId = games[gameIndex].second;
            }
            else {
                UIComponents::setOutputMessage(hwnd, "Error: Invalid game selection");
                return;
            }

            GetDlgItemTextW(hwnd, 304, buffer, sizeof(buffer) / sizeof(WCHAR));
            WideCharToMultiByte(CP_UTF8, 0, buffer, -1, ansiBuffer, sizeof(ansiBuffer), nullptr, nullptr);
            server.rconPassword = ansiBuffer;
            server.rconPassword.erase(0, server.rconPassword.find_first_not_of(" \t\r\n"));
            server.rconPassword.erase(server.rconPassword.find_last_not_of(" \t\r\n") + 1);

            GetDlgItemTextW(hwnd, 305, buffer, sizeof(buffer) / sizeof(WCHAR));
            size_t wideLength = wcslen(buffer);
            WideCharToMultiByte(CP_UTF8, 0, buffer, -1, ansiBuffer, sizeof(ansiBuffer), nullptr, nullptr);
            if (wideLength >= 4096) {
                UIComponents::setOutputMessage(hwnd, "Error: Gametypes input exceeds 4096 characters");
                return;
            }
            WideCharToMultiByte(CP_UTF8, 0, buffer, -1, ansiBuffer, sizeof(ansiBuffer), nullptr, nullptr);
            server.gametypes = ansiBuffer;
            server.gametypes.erase(0, server.gametypes.find_first_not_of(" \t\r\n"));
            server.gametypes.erase(server.gametypes.find_last_not_of(" \t\r\n") + 1);

            GetDlgItemTextW(hwnd, 306, buffer, sizeof(buffer) / sizeof(WCHAR));
            wideLength = wcslen(buffer);
            WideCharToMultiByte(CP_UTF8, 0, buffer, -1, ansiBuffer, sizeof(ansiBuffer), nullptr, nullptr);
            if (wideLength >= 4096) {
                UIComponents::setOutputMessage(hwnd, "Error: Maps input exceeds 4096 characters");
                return;
            }
            WideCharToMultiByte(CP_UTF8, 0, buffer, -1, ansiBuffer, sizeof(ansiBuffer), nullptr, nullptr);
            server.maps = ansiBuffer;
            server.maps.erase(0, server.maps.find_first_not_of(" \t\r\n"));
            server.maps.erase(server.maps.find_last_not_of(" \t\r\n") + 1);

            if (server.name.empty()) {
                UIComponents::setOutputMessage(hwnd, "Error: Server name cannot be empty or whitespace");
                return;
            }
            if (!ServerManager::validateIpOrHostname(server.ipOrHostname)) {
                std::regex ipAttempt(R"(^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})$)");
                std::string errorMsg = std::regex_match(server.ipOrHostname, ipAttempt)
                    ? "Error: Invalid IP address (use format: 192.168.0.1)"
                    : "Error: Invalid hostname (use format: example.com)";
                UIComponents::setOutputMessage(hwnd, errorMsg.c_str());
                return;
            }
            if (!ServerManager::validatePort(server.port)) {
                UIComponents::setOutputMessage(hwnd, "Error: Invalid port (1-65535)");
                return;
            }
            if (!ServerManager::validateListFormat(server.gametypes)) {
                UIComponents::setOutputMessage(hwnd, "Error: Invalid gametypes format (id:humanreadable,...)");
                return;
            }
            if (!ServerManager::validateListFormat(server.maps)) {
                UIComponents::setOutputMessage(hwnd, "Error: Invalid maps format (id:humanreadable,...)");
                return;
            }

            if (server.gametypes.empty()) {
                server.gametypes = ServerManager::getDefaultGametypes(server.game);
            }
            if (server.maps.empty()) {
                server.maps = ServerManager::getDefaultMaps(server.game);
            }

            if (!editingServerName.empty() && editingServerName != server.name) {
                ServerManager::deleteServer(editingServerName);
            }

            ServerManager::saveServer(server);
            UIServers::updateServerTable(hwnd);
            UIComponents::setOutputMessage(hwnd, "Server saved successfully");

            UIRcon::updateServerSelector(hwnd);

            // Clear the form
            SetDlgItemTextW(hwnd, 300, L"");
            SetDlgItemTextW(hwnd, 301, L"");
            SetDlgItemTextW(hwnd, 302, L"");
            SendDlgItemMessage(hwnd, 303, CB_SETCURSEL, 0, 0);
            SetDlgItemTextW(hwnd, 304, L"");
            SetDlgItemTextW(hwnd, 305, L"");
            SetDlgItemTextW(hwnd, 306, L"");
            editingServerName.clear();
            InvalidateRect(GetDlgItem(hwnd, 305), nullptr, TRUE);
            UpdateWindow(GetDlgItem(hwnd, 305));
            InvalidateRect(GetDlgItem(hwnd, 306), nullptr, TRUE);
            UpdateWindow(GetDlgItem(hwnd, 306));

            // Reset UI to show only Servers page controls
            hideAllPageControls(hwnd, true);
            UIServers::showServerPage(hwnd);
            ServerManager::logDebug("UI reset after saving server: " + server.name);
        }
    }
    else if (msg == WM_NOTIFY) {
        LPNMHDR nmhdr = (LPNMHDR)lParam;
        if (nmhdr->idFrom == 200 && (nmhdr->code == NM_CLICK || nmhdr->code == NM_DBLCLK)) {
            POINT pt;
            GetCursorPos(&pt);
            ScreenToClient(GetDlgItem(hwnd, 200), &pt);
            LVHITTESTINFO hit = { 0 };
            hit.pt = pt;
            ListView_SubItemHitTest(GetDlgItem(hwnd, 200), &hit);
            if (hit.iSubItem == 4 && hit.iItem >= 0) { // Edit column
                auto servers = ServerManager::loadServers();
                if (hit.iItem < static_cast<int>(servers.size())) {
                    editingServerName = servers[hit.iItem].name;
                    editServer(hwnd, hit.iItem);
                    UIComponents::setOutputMessage(hwnd, ("Editing server: " + editingServerName).c_str());
                }
            }
            else if (hit.iSubItem == 5 && hit.iItem >= 0) { // Delete column
                auto servers = ServerManager::loadServers();
                if (hit.iItem < static_cast<int>(servers.size())) {
                    ServerManager::deleteServer(servers[hit.iItem].name);
                    UIServers::updateServerTable(hwnd);
                    UIComponents::setOutputMessage(hwnd, "Server deleted successfully");

                    UIRcon::updateServerSelector(hwnd);

                    if (editingServerName == servers[hit.iItem].name) {
                        SetDlgItemTextW(hwnd, 300, L"");
                        SetDlgItemTextW(hwnd, 301, L"");
                        SetDlgItemTextW(hwnd, 302, L"");
                        SendDlgItemMessage(hwnd, 303, CB_SETCURSEL, 0, 0);
                        SetDlgItemTextW(hwnd, 304, L"");
                        SetDlgItemTextW(hwnd, 305, L"");
                        SetDlgItemTextW(hwnd, 306, L"");
                        editingServerName.clear();
                    }

                    // Reset UI to ensure only Servers page controls are visible
                    hideAllPageControls(hwnd, true);
                    UIServers::showServerPage(hwnd);
                    ServerManager::logDebug("UI reset after deleting server: " + servers[hit.iItem].name);
                }
            }
        }
    }
}

void ServerPage::editServer(HWND hwnd, int index) {
    auto servers = ServerManager::loadServers();
    if (index >= 0 && index < static_cast<int>(servers.size())) {
        const auto& server = servers[index];
        WCHAR buffer[4096];

        MultiByteToWideChar(CP_UTF8, 0, server.name.c_str(), -1, buffer, sizeof(buffer) / sizeof(WCHAR));
        SetDlgItemTextW(hwnd, 300, buffer);

        MultiByteToWideChar(CP_UTF8, 0, server.ipOrHostname.c_str(), -1, buffer, sizeof(buffer) / sizeof(WCHAR));
        SetDlgItemTextW(hwnd, 301, buffer);

        swprintf_s(buffer, L"%d", server.port);
        SetDlgItemTextW(hwnd, 302, buffer);

        auto games = ServerManager::getGameOptions();
        for (size_t i = 0; i < games.size(); ++i) {
            if (games[i].first == server.game) {
                SendDlgItemMessage(hwnd, 303, CB_SETCURSEL, i, 0);
                break;
            }
        }

        MultiByteToWideChar(CP_UTF8, 0, server.rconPassword.c_str(), -1, buffer, sizeof(buffer) / sizeof(WCHAR));
        SetDlgItemTextW(hwnd, 304, buffer);

        MultiByteToWideChar(CP_UTF8, 0, server.gametypes.c_str(), -1, buffer, sizeof(buffer) / sizeof(WCHAR));
        SetDlgItemTextW(hwnd, 305, buffer);

        MultiByteToWideChar(CP_UTF8, 0, server.maps.c_str(), -1, buffer, sizeof(buffer) / sizeof(WCHAR));
        SetDlgItemTextW(hwnd, 306, buffer);

        InvalidateRect(GetDlgItem(hwnd, 305), nullptr, TRUE);
        UpdateWindow(GetDlgItem(hwnd, 305));
        InvalidateRect(GetDlgItem(hwnd, 306), nullptr, TRUE);
        UpdateWindow(GetDlgItem(hwnd, 306));
    }
}