#ifndef SIX_CARD_GOLF_UTILS_H
#define SIX_CARD_GOLF_UTILS_H

#include <string>
#include <vector>
#include <unordered_map>
#include <random>

#define ECHOMAX 1024
#define MAX_PLAYERS 4

enum CommandType {
    CMD_REGISTER = 1,
    CMD_QUERY_PLAYERS,
    CMD_START_GAME,
    CMD_QUERY_GAMES,
    CMD_END_GAME,
    CMD_DEREGISTER
};

struct Message {
    CommandType cmd;
    char data[ECHOMAX - sizeof(CommandType)];
};

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
    int holes;
};

void DieWithError(const char *errorMessage);

#endif // UTILS_H