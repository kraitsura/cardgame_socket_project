#ifndef TRACKER_SERVER_H
#define TRACKER_SERVER_H

#include <string>
#include <vector>
#include <unordered_map>

const int MAX_PLAYERS = 5;

struct PlayerInfo {
    std::string name;
    std::string ipAddress;
    std::string state;  // "free" or "in-play"
    int tPort;
    int pPort;
};

struct GameInfo {
    int gameId;
    std::string dealer;
    std::vector<std::string> players;
};

class TrackerServer {
    private:
        std::unordered_map<std::string, PlayerInfo> players;
        std::unordered_map<int, GameInfo> games;
        int nextGameId;

    public:
        TrackerServer();

        std::string registerPlayer(const std::string& name, const std::string& ipAddress, int tPort, int pPort);
        std::string queryPlayers();
        std::string queryGames();
        std::string deregisterPlayer(const std::string& name);

        bool isPlayerRegistered(const std::string& name);
        void addPlayer(const PlayerInfo& player);
        void removePlayer(const std::string& name);
};

#endif // TRACKER_SERVER_H
