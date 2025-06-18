#pragma once
#include <Windows.h>

#ifdef GAMESERVERQUERY_EXPORTS
#define GAMESERVERQUERY_API __declspec(dllexport)
#else
#define GAMESERVERQUERY_API __declspec(dllimport)
#endif

extern "C" GAMESERVERQUERY_API const char* ProcessGameServerCommand(
    int protocolId,
    bool raw,
    const char* ipOrHostname,
    int port,
    const char* command,
    const char* rconPassword
);

extern "C" GAMESERVERQUERY_API void FreeGameServerResponse(const char* response);