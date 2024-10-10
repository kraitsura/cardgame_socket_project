#include "Utils.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <sstream>
#include <iomanip>


std::string Card::toString() const
{
    if (!faceUp)
        return "***";
    std::stringstream ss;
    if (rank <= 10)
        ss << rank;
    else if (rank == 11)
        ss << 'J';
    else if (rank == 12)
        ss << 'Q';
    else if (rank == 13)
        ss << 'K';
    else
        ss << 'A';
    ss << suit;
    return ss.str();
}

Deck::Deck()
{
    const char suits[] = {'H', 'D', 'S', 'C'};
    for (char suit : suits)
    {
        for (int rank = 2; rank <= 14; ++rank)
        {
            cards.emplace_back(rank, suit);
        }
    }
}

void Deck::shuffle()
{
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(cards.begin(), cards.end(), std::default_random_engine(seed));
}

Card Deck::drawCard()
{
    if (cards.empty())
        throw std::runtime_error("Deck is empty");
    Card card = cards.back();
    cards.pop_back();
    return card;
}

bool Deck::isEmpty() const
{
    return cards.empty();
}

std::string displayHand(const std::vector<Card> &hand, int cardsPerRow)
{
    std::stringstream ss;
    for (size_t i = 0; i < hand.size(); ++i)
    {
        ss << std::setw(3) << std::left << hand[i].toString();
        if ((i + 1) % cardsPerRow == 0 || i == hand.size() - 1)
            ss << '\n';
        else
            ss << ' ';
    }
    return ss.str();
}

void updateHand(std::vector<Card> &hand, size_t index, Card newCard)
{
    if (index >= hand.size())
    {
        throw std::out_of_range("Invalid index for updating hand");
    }
    hand[index] = newCard;
}

// Function to flip a card in the hand
void flipCard(std::vector<Card> &hand, size_t index)
{
    if (index >= hand.size())
    {
        throw std::out_of_range("Invalid index for flipping card");
    }
    hand[index].faceUp = !hand[index].faceUp;
}

const char *cmdToString(CommandType cmd)
{
    switch (cmd)
    {
    case CMD_REGISTER:
        return "REGISTER";
    case CMD_DEREGISTER:
        return "DEREGISTER";
    case CMD_QUERY_PLAYERS:
        return "QUERY_PLAYERS";
    case CMD_START_GAME:
        return "START_GAME";
    case CMD_QUERY_GAMES:
        return "QUERY_GAMES";
    case CMD_END_GAME:
        return "END_GAME";
    default:
        return "UNKNOWN";
    }
}

void DieWithError(const char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}

void ShowHelp()
{
    std::cout << "Available commands:" << std::endl;
    std::cout << "  register <name> <ip> <tracker_port> <peer_port> - Register player" << std::endl;
    std::cout << "  deregister - De-register player" << std::endl;
    std::cout << "  start <dealer> <num_players> <num_holes> - Start a new game" << std::endl;
    std::cout << "  query_players - Query registered players" << std::endl;
    std::cout << "  query_games - Query ongoing games" << std::endl;
    std::cout << "  help - Show this help message" << std::endl;
    std::cout << "  quit - Exit the program" << std::endl;
}