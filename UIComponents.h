#ifndef UICOMPONENTS_H
#define UICOMPONENTS_H

#include <windows.h>
#include <string>
#include "ServerManager.h"

class UIComponents {
public:
    static void createSidebar(HWND hwnd, HINSTANCE hInstance);
    static void createOutputBox(HWND hwnd, HINSTANCE hInstance);
    static void setOutputMessage(HWND hwnd, const char* message);
};

#endif