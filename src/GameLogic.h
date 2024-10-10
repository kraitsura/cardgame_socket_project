#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include "Utils.h"
#include <vector>
#include <string>

class SixGolfGameLogic {
public:
    SixGolfGameLogic(int numPlayers, int numHoles);
    void startNewHole();
    void dealCards();
    bool isHoleFinished() const;
    bool isGameFinished() const;
    int getCurrentPlayerTurn() const;
    void nextTurn();
    Card drawCard(bool fromDeck);
    void discardCard(const Card& card);
    void replaceCard(int playerIndex, int cardIndex, const Card& newCard);
    void flipCard(int playerIndex, int cardIndex);
    int calculateScore(int playerIndex) const;
    void calculateHoleScores();
    std::vector<int> getFinalScores() const;
    int getWinner() const;

private:
    int numPlayers;
    int numHoles;
    int currentHole;
    int currentPlayerTurn;
    bool holeFinished;
    bool gameFinished;
    Deck deck;
    std::vector<Card> discardPile;
    std::vector<std::vector<Card>> playerHands;
    std::vector<std::vector<int>> playerScores;

    void initializeGame();
    void initializePlayerHands();
    void checkHoleFinished();
};

#endif // GAME_LOGIC_H
