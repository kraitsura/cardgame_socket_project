#include "Tracker.h"
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <algorithm>
#include <chrono>

Tracker::Tracker() : nextGameId(1) {
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    rng.seed(seed);
}

std::string Tracker::registerPlayer(const std::string& name, const std::string& ipAddress, int tPort, int pPort) {
    if (players.find(name) != players.end()) {
        return "FAILURE Player already registered";
    }
    PlayerInfo newPlayer = {name, ipAddress, "free", tPort, pPort};
    players[name] = newPlayer;
    return "SUCCESS";
}

std::string Tracker::deregisterPlayer(const std::string& name) {
    if (players.find(name) == players.end()) {
        return "FAILURE Player not registered";
    }
    if (players[name].state == "in-play") {
        return "FAILURE Player is currently in a game";
    }
    players.erase(name);
    return "SUCCESS";
}

std::string Tracker::queryPlayers() {
    std::stringstream ss;
    ss << players.size() << " ";
    for (const auto& pair : players) {
        const PlayerInfo& player = pair.second;
        ss << player.name << " " << player.ipAddress << " " << player.tPort << " " << player.pPort << " " << player.state << " ";
    }
    return ss.str();
}

std::string Tracker::queryGames() {
    std::stringstream ss;
    ss << games.size() << " ";
    for (const auto& pair : games) {
        const GameInfo& game = pair.second;
        ss << game.gameId << " " << game.dealer << " " << game.holes << " ";
        for (const auto& player : game.players) {
            ss << player << " ";
        }
    }
    return ss.str();
}

std::string Tracker::startGame(const std::string& dealer, int n, int holes) {
    if (players.find(dealer) == players.end() || players[dealer].state != "free") {
        return "FAILURE Invalid dealer or dealer not available";
    }
    if (n < 1 || n > 3) {
        return "FAILURE Invalid number of additional players";
    }
    if (holes < 1 || holes > 9) {
        return "FAILURE Invalid number of holes";
    }
    
    std::vector<std::string> selectedPlayers = selectRandomPlayers(players, rng, n);
    if (selectedPlayers.size() < n) {
        return "FAILURE Not enough free players";
    }
    
    GameInfo newGame = {nextGameId, dealer, selectedPlayers, holes};
    newGame.players.push_back(dealer);
    games[nextGameId] = newGame;
    
    // Update player states
    for (const auto& player : newGame.players) {
        updatePlayerState(player, "in-play");
    }
    
    std::stringstream ss;
    ss << "SUCCESS " << nextGameId << " ";
    for (const auto& player : newGame.players) {
        const PlayerInfo& info = players[player];
        ss << info.name << " " << info.ipAddress << " " << info.pPort << " ";
    }
    
    nextGameId++;
    return ss.str();
}

std::string Tracker::endGame(int gameId, const std::string& dealer) {
    auto it = games.find(gameId);
    if (it == games.end()) {
        return "FAILURE Game not found";
    }
    
    if (it->second.dealer != dealer) {
        return "FAILURE Only the dealer can end the game";
    }
    
    // Update player states
    for (const auto& player : it->second.players) {
        updatePlayerState(player, "free");
    }
    
    games.erase(it);
    return "SUCCESS";
}

bool Tracker::isPlayerRegistered(const std::string& name) {
    return players.find(name) != players.end();
}

bool Tracker::isPlayerInGame(const std::string& name) {
    auto it = players.find(name);
    if (it == players.end()) {
        return false;
    }
    return it->second.state == "in-play";
}

void Tracker::updatePlayerState(const std::string& name, const std::string& state) {
    auto it = players.find(name);
    if (it != players.end()) {
        it->second.state = state;
    }
}

std::vector<std::string> Tracker::selectRandomPlayers(const std::unordered_map<std::string, PlayerInfo>& players, std::mt19937& rng, int n) {
    std::vector<std::string> freePlayers;
    for (const auto& pair : players) {
        if (pair.second.state == "free") {
            freePlayers.push_back(pair.first);
        }
    }
    std::shuffle(freePlayers.begin(), freePlayers.end(), rng);
    return std::vector<std::string>(freePlayers.begin(), freePlayers.begin() + std::min(n, static_cast<int>(freePlayers.size())));
}
