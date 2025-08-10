// --- xRcon\RconPage.cpp ---
// Implementation of the RCON page logic.
// Handles user interactions for sending RCON commands, updating server settings, and managing player actions.

#include "RconPage.h"
#include "UIRcon.h"
#include "UIComponents.h"
#include "GameServerQuery.h"
#include "ServerManager.h"
#include <commctrl.h>
#include <vector>
#include <sstream>
#include <string>
#include <map>
#include <thread>
#include <chrono>

#pragma comment(lib, "comctl32.lib") // Link Common Controls library
#pragma comment(lib, "Ws2_32.lib")   // Link Winsock library
#pragma comment(lib, "GameServerQuery.lib") // Link GameServerQuery library

// Sends an RCON command to the specified server and displays the response.
void RconPage::sendRconCommand(HWND hwnd, const Server& server, const char* command) {
    if (server.ipOrHostname.empty() || server.port == 0) {
        UIComponents::setOutputMessage(hwnd, "Invalid server details: IP/hostname or port is invalid.");
        return;
    }

    // Construct full RCON command
    std::string fullCommand = std::string("rcon ") + command;
    const char* response = ProcessGameServerCommand(
        server.protocolId,
        false,
        server.ipOrHostname.c_str(),
        server.port,
        fullCommand.c_str(),
        server.rconPassword.empty() ? "" : server.rconPassword.c_str()
    );

    // Handle response or error
    if (!response || strncmp(response, "error=", 6) == 0) {
        std::string error = response ? response : "No response from server";
        UIComponents::setOutputMessage(hwnd, ("Command failed: " + error).c_str());
    }
    else {
        UIComponents::setOutputMessage(hwnd, response);
    }
    if (response) FreeGameServerResponse(response); // Free allocated response memory
}

// Handles messages for the RCON page, including commands, notifications, and timers.
void RconPage::handleRconPage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_COMMAND) {
        int id = LOWORD(wParam);
        if (id == 500 && HIWORD(wParam) == CBN_SELCHANGE) { // Server selection changed
            HWND hwndServerCombo = GetDlgItem(hwnd, 500);
            if (!hwndServerCombo) {
                UIComponents::setOutputMessage(hwnd, "Error: Server selector not available");
                return;
            }
            int index = static_cast<int>(SendMessage(hwndServerCombo, CB_GETCURSEL, 0, 0));
            if (index != CB_ERR) {
                auto servers = ServerManager::loadServers();
                if (index < static_cast<int>(servers.size())) {
                    UIRcon::updatePlayerTable(hwnd, servers[index]);
                    UIRcon::updateServerSettings(hwnd, servers[index]);
                }
            }
        }
        else if (id == 503) { // Send RCON command
            WCHAR buffer[1024];
            char ansiBuffer[1024];
            GetDlgItemTextW(hwnd, 502, buffer, sizeof(buffer) / sizeof(WCHAR));
            WideCharToMultiByte(CP_UTF8, 0, buffer, -1, ansiBuffer, sizeof(ansiBuffer), nullptr, nullptr);
            std::string command = ansiBuffer;
            if (command.empty()) {
                UIComponents::setOutputMessage(hwnd, "Error: Command cannot be empty");
                return;
            }

            HWND hwndServerCombo = GetDlgItem(hwnd, 500);
            if (!hwndServerCombo) {
                UIComponents::setOutputMessage(hwnd, "Error: Server selector not available");
                return;
            }
            int index = static_cast<int>(SendMessage(hwndServerCombo, CB_GETCURSEL, 0, 0));
            if (index == CB_ERR) {
                UIComponents::setOutputMessage(hwnd, "Error: No server selected");
                return;
            }

            auto servers = ServerManager::loadServers();
            if (index < static_cast<int>(servers.size())) {
                // Confirm sensitive commands
                if (command.find("kick") != std::string::npos || command.find("ban") != std::string::npos ||
                    command.find("rename") != std::string::npos || command.find("unbind") != std::string::npos) {
                    std::wstring confirmMsg = L"Are you sure you want to execute the command:\n" +
                        std::wstring(buffer) + L"?";
                    int result = MessageBoxW(hwnd, confirmMsg.c_str(), L"Confirm Action", MB_YESNO | MB_ICONWARNING);
                    if (result != IDYES) {
                        return;
                    }
                }
                sendRconCommand(hwnd, servers[index], ansiBuffer);
                // Update UI for player-affecting commands
                if (command.find("kick") != std::string::npos || command.find("ban") != std::string::npos ||
                    command.find("rename") != std::string::npos || command.find("unbind") != std::string::npos) {
                    UIRcon::updatePlayerTable(hwnd, servers[index]);
                    UIRcon::updateServerSettings(hwnd, servers[index]);
                }
            }
        }
        else if (id == 512) { // Apply hostname
            WCHAR buffer[1024];
            char ansiBuffer[1024];
            GetDlgItemTextW(hwnd, 511, buffer, sizeof(buffer) / sizeof(WCHAR));
            WideCharToMultiByte(CP_UTF8, 0, buffer, -1, ansiBuffer, sizeof(ansiBuffer), nullptr, nullptr);
            std::string hostname = ansiBuffer;
            if (hostname.empty()) {
                UIComponents::setOutputMessage(hwnd, "Error: Hostname cannot be empty");
                return;
            }

            HWND hwndServerCombo = GetDlgItem(hwnd, 500);
            if (!hwndServerCombo) {
                UIComponents::setOutputMessage(hwnd, "Error: Server not available");
                return;
            }
            int index = static_cast<int>(SendMessage(hwndServerCombo, CB_GETCURSEL, 0, 0));
            if (index == CB_ERR) {
                UIComponents::setOutputMessage(hwnd, "Error: No server selected");
                return;
            }

            auto servers = ServerManager::loadServers();
            if (index < static_cast<int>(servers.size())) {
                std::wstring confirmMsg = L"Are you sure you want to change the hostname to:\n" +
                    std::wstring(buffer) + L"?";
                int result = MessageBoxW(hwnd, confirmMsg.c_str(), L"Confirm Hostname Change", MB_YESNO | MB_ICONWARNING);
                if (result != IDYES) {
                    return;
                }
                std::string command = "sv_hostname \"" + hostname + "\"";
                sendRconCommand(hwnd, servers[index], command.c_str());
                UIRcon::updateServerSettings(hwnd, servers[index]);
            }
        }
        else if (id == 515) { // Apply map
            HWND hwndServerCombo = GetDlgItem(hwnd, 500);
            HWND hwndMapSelector = GetDlgItem(hwnd, 514);
            if (!hwndServerCombo || !hwndMapSelector) {
                UIComponents::setOutputMessage(hwnd, "Error: Server selector or map selector not available");
                return;
            }
            int serverIndex = static_cast<int>(SendMessage(hwndServerCombo, CB_GETCURSEL, 0, 0));
            int mapIndex = static_cast<int>(SendMessage(hwndMapSelector, CB_GETCURSEL, 0, 0));
            if (serverIndex == CB_ERR || mapIndex == CB_ERR) {
                UIComponents::setOutputMessage(hwnd, "Error: No server or map selected");
                return;
            }

            auto servers = ServerManager::loadServers();
            if (serverIndex < static_cast<int>(servers.size())) {
                std::map<std::string, std::string> mapList = ServerManager::parseList(servers[serverIndex].maps);
                auto it = mapList.begin();
                std::advance(it, mapIndex);
                std::string mapValue = it->first;
                std::wstring confirmMsg = L"Are you sure you want to change the map to:\n" +
                    std::wstring(it->second.begin(), it->second.end()) + L"?";
                int result = MessageBoxW(hwnd, confirmMsg.c_str(), L"Confirm Map Change", MB_YESNO | MB_ICONWARNING);
                if (result != IDYES) {
                    return;
                }
                std::string command = "map " + mapValue;
                sendRconCommand(hwnd, servers[serverIndex], command.c_str());
                UIRcon::updatePlayerTable(hwnd, servers[serverIndex]);
                UIRcon::updateServerSettings(hwnd, servers[serverIndex]);
            }
        }
        else if (id == 518) { // Apply gametype
            HWND hwndServerCombo = GetDlgItem(hwnd, 500);
            HWND hwndGametypeSelector = GetDlgItem(hwnd, 517);
            if (!hwndServerCombo || !hwndGametypeSelector) {
                UIComponents::setOutputMessage(hwnd, "Error: Server or gametype selector not available");
                return;
            }
            int serverIndex = static_cast<int>(SendMessage(hwndServerCombo, CB_GETCURSEL, 0, 0));
            int gametypeIndex = static_cast<int>(SendMessage(hwndGametypeSelector, CB_GETCURSEL, 0, 0));
            if (serverIndex == CB_ERR || gametypeIndex == CB_ERR) {
                UIComponents::setOutputMessage(hwnd, "Error: No server or gametype selected");
                return;
            }

            auto servers = ServerManager::loadServers();
            if (serverIndex < static_cast<int>(servers.size())) {
                bool isMOHAA = servers[serverIndex].game == "Medal of Honor: Allied Assault";
                std::map<std::string, std::string> gametypeList = ServerManager::parseList(servers[serverIndex].gametypes);
                auto it = gametypeList.begin();
                std::advance(it, gametypeIndex);
                std::string gametypeValue = it->first;
                std::wstring confirmMsg = L"Are you sure you want to change the gametype to:\n" +
                    std::wstring(it->second.begin(), it->second.end()) + L"?";
                int result = MessageBoxW(hwnd, confirmMsg.c_str(), L"Confirm Gametype Change", MB_YESNO | MB_ICONWARNING);
                if (result != IDYES) {
                    return;
                }
                std::string command = "g_gametype " + gametypeValue;
                sendRconCommand(hwnd, servers[serverIndex], command.c_str());
                if (isMOHAA) {
                    std::map<std::string, std::string> mapList = ServerManager::parseList(servers[serverIndex].maps);
                    int mapIndex = static_cast<int>(SendMessage(GetDlgItem(hwnd, 514), CB_GETCURSEL, 0, 0));
                    auto mapIt = mapList.begin();
                    std::advance(mapIt, mapIndex);
                    std::string mapCommand = "map " + mapIt->first;
                    sendRconCommand(hwnd, servers[serverIndex], mapCommand.c_str());
                }
                else {
                    // Add a one second delay before running the map_restart command
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    sendRconCommand(hwnd, servers[serverIndex], "map_restart");
                }
                UIRcon::updatePlayerTable(hwnd, servers[serverIndex]);
                UIRcon::updateServerSettings(hwnd, servers[serverIndex]);
            }
        }
        else if (id == 521 || id == 522 || id == 523) { // Restart actions
            HWND hwndServerCombo = GetDlgItem(hwnd, 500);
            if (!hwndServerCombo) {
                UIComponents::setOutputMessage(hwnd, "Error: Server selector not available");
                return;
            }
            int index = static_cast<int>(SendMessage(hwndServerCombo, CB_GETCURSEL, 0, 0));
            if (index == CB_ERR) {
                UIComponents::setOutputMessage(hwnd, "Error: No server selected");
                return;
            }

            auto servers = ServerManager::loadServers();
            if (index < static_cast<int>(servers.size())) {
                std::string command;
                if (id == 521) {
                    bool isMOHAA = servers[index].game == "Medal of Honor: Allied Assault";
                    command = isMOHAA ? "restart" : "map_restart";
                }
                else if (id == 522) command = "fast_restart";
                else if (id == 523) command = "map_rotate";
                std::wstring confirmMsg = L"Are you sure you want to execute:\n" +
                    std::wstring(command.begin(), command.end()) + L"?";
                int result = MessageBoxW(hwnd, confirmMsg.c_str(), L"Confirm Action", MB_YESNO | MB_ICONWARNING);
                if (result != IDYES) {
                    return;
                }
                sendRconCommand(hwnd, servers[index], command.c_str());
                UIRcon::updatePlayerTable(hwnd, servers[index]);
                UIRcon::updateServerSettings(hwnd, servers[index]);
            }
        }
    }
    else if (msg == WM_NOTIFY) {
        LPNMHDR nmhdr = (LPNMHDR)lParam;
        if (nmhdr->idFrom == 501 && (nmhdr->code == NM_CLICK || nmhdr->code == NM_DBLCLK)) { // Player table click
            POINT pt;
            GetCursorPos(&pt);
            HWND playerTable = GetDlgItem(hwnd, 501);
            if (!playerTable) {
                return;
            }
            ScreenToClient(playerTable, &pt);
            LVHITTESTINFO hit = { 0 };
            hit.pt = pt;
            ListView_SubItemHitTest(playerTable, &hit);
            if (hit.iItem >= 0 && hit.iSubItem >= 5) {
                HWND hwndServerCombo = GetDlgItem(hwnd, 500);
                if (!hwndServerCombo) {
                    UIComponents::setOutputMessage(hwnd, "Error: Server selector not available");
                    return;
                }
                int index = static_cast<int>(SendMessage(hwndServerCombo, CB_GETCURSEL, 0, 0));
                if (index == CB_ERR) {
                    return;
                }
                auto servers = ServerManager::loadServers();
                if (index >= static_cast<int>(servers.size())) {
                    return;
                }
                const Server& server = servers[index];

                // Determine game-specific commands
                bool isMOHAA = server.game == "Medal of Honor: Allied Assault" || server.game == "Medal of Honor: AA Spearhead";
                bool isMOHSHBT = server.game == "Medal of Honor: AA Breakthrough";
                bool isCOD = server.game.find("Call of Duty") != std::string::npos;

                std::string command;
                WCHAR buffer[1024];
                char ansiNum[512];
                ListView_GetItemText(playerTable, hit.iItem, 0, buffer, sizeof(buffer) / sizeof(WCHAR));
                WideCharToMultiByte(CP_UTF8, 0, buffer, -1, ansiNum, sizeof(ansiNum), nullptr, nullptr);

                if (isMOHAA) {
                    if (hit.iSubItem == 5) command = "set namechange " + std::string(ansiNum);
                    else if (hit.iSubItem == 6) command = "set unbindplayer " + std::string(ansiNum);
                    else if (hit.iSubItem == 7) command = "clientkick " + std::string(ansiNum);
                }
                else if (isMOHSHBT && hit.iSubItem == 5) {
                    command = "clientkick " + std::string(ansiNum);
                }
                else if (isCOD) {
                    if (hit.iSubItem == 5) command = "clientkick " + std::string(ansiNum);
                    else if (hit.iSubItem == 6) command = "banclient " + std::string(ansiNum);
                }

                if (!command.empty()) {
                    std::wstring confirmMsg = L"Are you sure you want to execute:\n" +
                        std::wstring(command.begin(), command.end()) + L" on this player?";
                    int result = MessageBoxW(hwnd, confirmMsg.c_str(), L"Confirm action", MB_YESNO | MB_ICONWARNING);
                    if (result != IDYES) {
                        return;
                    }

                    sendRconCommand(hwnd, server, command.c_str());
                    UIRcon::updatePlayerTable(hwnd, server);
                    UIRcon::updateServerSettings(hwnd, server);
                }
            }
        }
    }
    else if (msg == WM_TIMER) {
        if (wParam == 1001) { // Refresh timer
            HWND hwndServerCombo = GetDlgItem(hwnd, 500);
            if (!hwndServerCombo) {
                UIComponents::setOutputMessage(hwnd, "Error: Server selector not available");
                return;
            }
            int index = static_cast<int>(SendMessage(hwndServerCombo, CB_GETCURSEL, 0, 0));
            if (index == CB_ERR) {
                UIComponents::setOutputMessage(hwnd, "No server selected for refresh");
                return;
            }
            auto servers = ServerManager::loadServers();
            if (index < static_cast<int>(servers.size())) {
                UIRcon::updatePlayerTable(hwnd, servers[index]);
                UIRcon::updateServerSettings(hwnd, servers[index]);
            }
            else {
                UIComponents::setOutputMessage(hwnd, "Error: Invalid server index");
            }
        }
    }
}
