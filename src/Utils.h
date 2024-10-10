#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <unordered_map>
#include <random>

#define ECHOMAX 1024
#define MAX_PLAYERS 4

enum CommandType
{
    CMD_REGISTER = 1,
    CMD_QUERY_PLAYERS,
    CMD_START_GAME,
    CMD_QUERY_GAMES,
    CMD_END_GAME,
    CMD_DEREGISTER
};

struct Message
{
    CommandType cmd;
    char data[ECHOMAX - sizeof(CommandType)];
};

struct Card
{
    int rank;  // 2-10, 11(J), 12(Q), 13(K), 14(A)
    char suit; // 'H', 'D', 'S', 'C'
    bool faceUp;

    Card(int r, char s) : rank(r), suit(s), faceUp(false) {}

    std::string toString() const;
};

class Deck
{
public:
    Deck();
    void shuffle();
    Card drawCard();
    bool isEmpty() const;

private:
    std::vector<Card> cards;
};

struct PlayerInfo
{
    std::string name;
    std::string ipAddress;
    std::string state; // "free" or "in-play"
    int tPort;
    int pPort;
    std::vector<Card> hand;

    PlayerInfo() : name(""), ipAddress(""), state("free"), tPort(0), pPort(0) {}

    PlayerInfo(const std::string &n, const std::string &ip, const std::string &s, int t, int p, const std::vector<Card>& h = std::vector<Card>())
        : name(n), ipAddress(ip), state(s), tPort(t), pPort(p), hand(h) {}
};

struct GameInfo
{
    int gameId;
    std::string dealer;
    std::vector<std::string> players;
    int holes;
};

const char* cmdToString(CommandType cmd);

std::string displayHand(const std::vector<Card> &hand, int cardsPerRow = 6);
void updateHand(std::vector<Card> &hand, size_t index, Card newCard);
void flipCard(std::vector<Card> &hand, size_t index);

void DieWithError(const char *errorMessage);
void ShowHelp();

#endif // UTILS_H