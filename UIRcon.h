#ifndef UIRCON_H
#define UIRCON_H

#include <windows.h>
#include <string>
#include <vector>
#include "ServerManager.h"

class UIRcon {
public:
    static void createRconPage(HWND hwnd, HINSTANCE hInstance);
    static void showRconPage(HWND hwnd);
    static void hideRconPage(HWND hwnd);
    static void updateServerSelector(HWND hwnd);
    static void updatePlayerTable(HWND hwnd, const Server& server);
    static void updateServerSettings(HWND hwnd, const Server& server);
    static void parseAndAddPlayer(HWND hwnd, std::string playerJson, int index, std::vector<std::wstring>& strings, bool isMOHAA, bool isMOHSHBT, bool isCOD);
private:
    static void scheduleRefresh(HWND hwnd, const Server& server);
};

#endif