#pragma once
#include <Windows.h>
#include "ServerManager.h"


class ServerPage {
public:
    static void handleServerPage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static void editServer(HWND hwnd, int index);
};