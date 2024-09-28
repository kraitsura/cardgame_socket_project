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

    public:

};

#endif // TRACKER_SERVER_H