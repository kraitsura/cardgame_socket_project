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
    std::shared_ptr<std::vector<Card>> hand; // New field for the player's hand

    PlayerInfo(const std::string& n, const std::string& ip, const std::string& s, int t, int p)
        : name(n), ipAddress(ip), state(s), tPort(t), pPort(p), hand(nullptr) {}
};

struct GameInfo {
    int gameId;
    std::string dealer;
    std::vector<std::string> players;
    int holes;
};

struct Card;
class Deck;

std::string displayHand(const std::vector<Card>& hand, int cardsPerRow = 6);
void updateHand(std::vector<Card>& hand, size_t index, Card newCard);
void flipCard(std::vector<Card>& hand, size_t index);

void DieWithError(const char *errorMessage);
void ShowHelp();

#endif // UTILS_H