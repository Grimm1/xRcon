#pragma once
#include <Windows.h>
#include "ServerManager.h"

class RconPage {
public:
    static void handleRconPage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
private:
    static void sendRconCommand(HWND hwnd, const Server& server, const char* command);
};