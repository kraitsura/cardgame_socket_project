#ifndef TRACKER_H
#define TRACKER_H

#include "Utils.h"
#include <unordered_map>
#include <random>

class Tracker {
private:
    std::unordered_map<std::string, PlayerInfo> players;
    std::unordered_map<int, GameInfo> games;
    int nextGameId;
    std::mt19937 rng;

public:
    Tracker();

    std::string registerPlayer(const std::string& name, const std::string& ipAddress, int tPort, int pPort);
    std::string queryPlayers();
    std::string queryGames();
    std::string deregisterPlayer(const std::string& name);
    std::string startGame(const std::string& dealer, int n, int holes);
    std::string endGame(int gameId, const std::string& dealer);
    std::vector<std::string> selectRandomPlayers(const std::unordered_map<std::string, PlayerInfo>& players, std::mt19937& rng, int n);

    bool isPlayerRegistered(const std::string& name);
    bool isPlayerInGame(const std::string& name);
    void updatePlayerState(const std::string& name, const std::string& state);
};

#endif // TRACKER_H