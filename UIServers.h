#ifndef UISERVERS_H
#define UISERVERS_H

#include <windows.h>
#include "ServerManager.h"

class UIServers {
public:
    static void createServerTable(HWND hwnd, HINSTANCE hInstance);
    static void createServerForm(HWND hwnd, HINSTANCE hInstance);
    static void updateServerTable(HWND hwnd);
    static void createServerPage(HWND hwnd, HINSTANCE hInstance);
    static void showServerPage(HWND hwnd);
    static void hideServerPage(HWND hwnd);
};

#endif