// --- C:\Projects\xrcon\xRcon\ServerManager.cpp ---
// Implementation of server management functionality.
// Handles loading, saving, validating, and deleting game server configurations.

#include "ServerManager.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <regex>
#include <ctime>

// Logs a debug message to debug.log with a timestamp.
void ServerManager::logDebug(const std::string& message) {
    std::ofstream logFile("debug.log", std::ios::app);
    if (logFile.is_open()) {
        std::time_t now = std::time(nullptr);
        char timeStr[26];
        ctime_s(timeStr, sizeof(timeStr), &now);
        timeStr[24] = '\0'; // Remove newline
        logFile << "[" << timeStr << "] " << message << "\n";
        logFile.close();
    }
}

// Validates an IP address or hostname.
bool ServerManager::validateIpOrHostname(const std::string& input) {
    if (input.empty()) {
        return false; // Empty input is invalid
    }

    // Validate IP address format
    std::regex ipRegex(R"(^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})$)");
    std::smatch match;
    if (std::regex_match(input, match, ipRegex)) {
        for (size_t i = 1; i <= 4; ++i) {
            std::string segment = match[i].str();
            if (segment.size() > 1 && segment[0] == '0') {
                return false; // Leading zeros are invalid
            }
            try {
                size_t pos;
                int num = std::stoi(segment, &pos);
                if (pos != segment.size() || num < 0 || num > 255) {
                    return false; // Invalid segment
                }
            }
            catch (...) {
                return false; // Conversion error
            }
        }
        return true;
    }

    // Validate hostname format
    if (input.size() > 255) {
        return false; // Hostname too long
    }
    std::regex hostnameRegex(R"(^[a-zA-Z0-9][a-zA-Z0-9-]*(\.[a-zA-Z0-9][a-zA-Z0-9-]*)+$)");
    if (!std::regex_match(input, hostnameRegex)) {
        return false; // Invalid hostname format
    }
    if (input.find("..") != std::string::npos || input[0] == '-' || input[input.size() - 1] == '-') {
        return false; // Invalid characters or structure
    }
    return true;
}

// Validates a network port number.
bool ServerManager::validatePort(int port) {
    return port >= 1 && port <= 65535; // Valid port range
}

// Validates the format of a comma-separated key:value list.
bool ServerManager::validateListFormat(const std::string& input) {
    if (input.empty()) {
        return true; // Empty list is valid
    }
    std::stringstream ss(input);
    std::string pair;
    while (std::getline(ss, pair, ',')) {
        size_t pos = pair.find(':');
        if (pos == std::string::npos || pos == 0 || pos == pair.size() - 1) {
            return false; // Invalid pair format
        }
    }
    return true;
}

// Validates a server configuration.
bool ServerManager::validateServer(const Server& server) {
    // Trim whitespace from strings
    std::string name = server.name;
    name.erase(0, name.find_first_not_of(" \t\r\n"));
    name.erase(name.find_last_not_of(" \t\r\n") + 1);

    std::string ipOrHostname = server.ipOrHostname;
    ipOrHostname.erase(0, ipOrHostname.find_first_not_of(" \t\r\n"));
    ipOrHostname.erase(ipOrHostname.find_last_not_of(" \t\r\n") + 1);

    std::string gametypes = server.gametypes;
    gametypes.erase(0, gametypes.find_first_not_of(" \t\r\n"));
    gametypes.erase(gametypes.find_last_not_of(" \t\r\n") + 1);

    std::string maps = server.maps;
    maps.erase(0, maps.find_first_not_of(" \t\r\n"));
    maps.erase(maps.find_last_not_of(" \t\r\n") + 1);

    // Check all required fields
    return !name.empty() &&
        validateIpOrHostname(ipOrHostname) &&
        validatePort(server.port) &&
        validateListFormat(gametypes) &&
        validateListFormat(maps);
}

// Loads server configurations from servers.ini.
std::vector<Server> ServerManager::loadServers() {
    std::vector<Server> servers;
    std::ifstream file("servers.ini");
    std::string line, section;
    Server server;

    if (!file.is_open()) {
        return servers; // Return empty list if file cannot be opened
    }

    while (std::getline(file, line)) {
        if (line.empty()) continue;
        if (line[0] == '[' && line.back() == ']') {
            // Save previous server if valid
            if (!section.empty() && validateServer(server)) {
                servers.push_back(server);
            }
            section = line.substr(1, line.size() - 2);
            server = Server();
            server.name = section;
            continue;
        }
        size_t pos = line.find('=');
        if (pos == std::string::npos) continue;
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        // Assign server properties
        if (key == "ip") server.ipOrHostname = value;
        else if (key == "port") {
            try { server.port = std::stoi(value); }
            catch (...) { server.port = -1; }
        }
        else if (key == "game") server.game = value;
        else if (key == "protocol") {
            try { server.protocolId = std::stoi(value); }
            catch (...) { server.protocolId = 0; }
        }
        else if (key == "rconPassword") server.rconPassword = value;
        else if (key == "gametypes") server.gametypes = value;
        else if (key == "maps") server.maps = value;
    }
    // Save last server if valid
    if (!section.empty() && validateServer(server)) {
        servers.push_back(server);
    }
    file.close();
    return servers;
}

// Saves a server configuration to servers.ini.
void ServerManager::saveServer(const Server& server) {
    if (!validateServer(server)) {
        return; // Skip invalid server
    }
    std::vector<Server> servers = loadServers();
    // Remove server with same name
    servers.erase(std::remove_if(servers.begin(), servers.end(),
        [&server](const Server& s) { return s.name == server.name; }), servers.end());
    servers.push_back(server);

    // Write updated server list to file
    std::ofstream file("servers.ini");
    if (!file.is_open()) {
        return;
    }
    for (const auto& s : servers) {
        file << "[" << s.name << "]\n";
        file << "ip=" << s.ipOrHostname << "\n";
        file << "port=" << s.port << "\n";
        file << "game=" << s.game << "\n";
        file << "protocol=" << s.protocolId << "\n";
        file << "rconPassword=" << s.rconPassword << "\n";
        file << "gametypes=" << s.gametypes << "\n";
        file << "maps=" << s.maps << "\n\n";
    }
    file.close();
}

// Deletes a server configuration by name.
void ServerManager::deleteServer(const std::string& name) {
    std::vector<Server> servers = loadServers();
    // Remove server with matching name
    servers.erase(std::remove_if(servers.begin(), servers.end(),
        [&name](const Server& s) { return s.name == name; }), servers.end());

    // Write updated server list to file
    std::ofstream file("servers.ini");
    if (!file.is_open()) {
        return;
    }
    for (const auto& s : servers) {
        file << "[" << s.name << "]\n";
        file << "ip=" << s.ipOrHostname << "\n";
        file << "port=" << s.port << "\n";
        file << "game=" << s.game << "\n";
        file << "protocol=" << s.protocolId << "\n";
        file << "rconPassword=" << s.rconPassword << "\n";
        file << "gametypes=" << s.gametypes << "\n";
        file << "maps=" << s.maps << "\n\n";
    }
    file.close();
}

// Returns a list of supported games and their protocol IDs.
std::vector<std::pair<std::string, int>> ServerManager::getGameOptions() {
    return {
        {"Medal of Honor: Allied Assault", 1},
        {"Medal of Honor: AA Spearhead", 1},
        {"Medal of Honor: AA Breakthrough", 1},
        {"Call of Duty", 2},
        {"Call of Duty: United Offensive", 2},
        {"Call of Duty 2", 2},
        {"Call of Duty 4: Modern Warfare", 2},
        {"Call of Duty: World at War", 2}
    };
}

// Retrieves default maps for a game from default_maps.ini.
std::string ServerManager::getDefaultMaps(const std::string& game) {
    std::ifstream file("default_maps.ini");
    std::string line, section, maps;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        if (line[0] == '[' && line.back() == ']') {
            section = line.substr(1, line.size() - 2);
            continue;
        }
        if (section == game && line.find("maps=") == 0) {
            maps = line.substr(5);
            break;
        }
    }
    file.close();
    return maps;
}

// Retrieves default gametypes for a game from default_gametypes.ini.
std::string ServerManager::getDefaultGametypes(const std::string& game) {
    std::ifstream file("default_gametypes.ini");
    std::string line, section, gametypes;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        if (line[0] == '[' && line.back() == ']') {
            section = line.substr(1, line.size() - 2);
            continue;
        }
        if (section == game && line.find("gametypes=") == 0) {
            gametypes = line.substr(10);
            break;
        }
    }
    file.close();
    return gametypes;
}

// Parses a comma-separated key:value list into a map.
std::map<std::string, std::string> ServerManager::parseList(const std::string& list) {
    std::map<std::string, std::string> result;
    std::stringstream ss(list);
    std::string pair;
    while (std::getline(ss, pair, ',')) {
        size_t pos = pair.find(':');
        if (pos != std::string::npos && pos != 0 && pos != pair.size() - 1) {
            result[pair.substr(0, pos)] = pair.substr(pos + 1);
        }
    }
    return result;
}