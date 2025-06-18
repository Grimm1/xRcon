#pragma once
#include <string>
#include <vector>
#include <Windows.h>
#include <map>

struct Server {
    std::string name;
    std::string ipOrHostname;
    int port = -1;
    std::string game;
    int protocolId = 0;
    std::string rconPassword;
    std::string gametypes; // Format: "id:humanreadable,id2:humanreadable2"
    std::string maps;      // Format: "id:humanreadable,id2:humanreadable2"
};

class ServerManager {
public:
    static std::vector<Server> loadServers();
    static void saveServer(const Server& server);
    static void deleteServer(const std::string& name);
    static std::vector<std::pair<std::string, int>> getGameOptions();
    static std::string getDefaultMaps(const std::string& game);
    static std::string getDefaultGametypes(const std::string& game);
    static bool validateIpOrHostname(const std::string& input);
    static bool validatePort(int port);
    static bool validateListFormat(const std::string& input);
    static bool validateServer(const Server& server);
    static void logDebug(const std::string& message);
    static std::map<std::string, std::string> parseList(const std::string& list); // Moved here
};