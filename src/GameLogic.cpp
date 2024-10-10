#include "GameLogic.h"
#include <algorithm>
#include <stdexcept>
#include <random>
#include <chrono>

SixGolfGameLogic::SixGolfGameLogic(int numPlayers, int numHoles) 
    : numPlayers(numPlayers), numHoles(numHoles), currentHole(0), 
      currentPlayerTurn(0), holeFinished(false), gameFinished(false) {
    initializeGame();
}

void SixGolfGameLogic::initializeGame() {
    playerHands.resize(numPlayers);
    playerScores.resize(numPlayers, std::vector<int>(numHoles, 0));
    startNewHole();
}

void SixGolfGameLogic::startNewHole() {
    currentHole++;
    holeFinished = false;
    deck = Deck();
    deck.shuffle();
    discardPile.clear();
    dealCards();
    initializePlayerHands();
}

void SixGolfGameLogic::dealCards() {
    for (int i = 0; i < 6; ++i) {
        for (auto& hand : playerHands) {
            hand.push_back(deck.drawCard());
        }
    }
    discardPile.push_back(deck.drawCard());
    discardPile.back().faceUp = true;
}

void SixGolfGameLogic::initializePlayerHands() {
    for (auto& hand : playerHands) {
        std::vector<int> indices = {0, 1, 2, 3, 4, 5};
        std::shuffle(indices.begin(), indices.end(), std::mt19937(std::random_device()()));
        hand[indices[0]].faceUp = true;
        hand[indices[1]].faceUp = true;
    }
}

bool SixGolfGameLogic::isHoleFinished() const {
    return holeFinished;
}

bool SixGolfGameLogic::isGameFinished() const {
    return gameFinished;
}

int SixGolfGameLogic::getCurrentPlayerTurn() const {
    return currentPlayerTurn;
}

void SixGolfGameLogic::nextTurn() {
    currentPlayerTurn = (currentPlayerTurn + 1) % numPlayers;
    checkHoleFinished();
}

void SixGolfGameLogic::checkHoleFinished() {
    for (const auto& hand : playerHands) {
        if (std::all_of(hand.begin(), hand.end(), [](const Card& card) { return card.faceUp; })) {
            holeFinished = true;
            break;
        }
    }
    
    if (holeFinished) {
        calculateHoleScores();
        if (currentHole == numHoles) {
            gameFinished = true;
        } else {
            startNewHole();
        }
    }
}

Card SixGolfGameLogic::drawCard(bool fromDeck) {
    if (fromDeck) {
        return deck.drawCard();
    } else {
        Card drawnCard = discardPile.back();
        discardPile.pop_back();
        return drawnCard;
    }
}

void SixGolfGameLogic::discardCard(const Card& card) {
    discardPile.push_back(card);
    discardPile.back().faceUp = true;
}

void SixGolfGameLogic::replaceCard(int playerIndex, int cardIndex, const Card& newCard) {
    Card oldCard = playerHands[playerIndex][cardIndex];
    playerHands[playerIndex][cardIndex] = newCard;
    playerHands[playerIndex][cardIndex].faceUp = true;
    discardCard(oldCard);
}

void SixGolfGameLogic::flipCard(int playerIndex, int cardIndex) {
    playerHands[playerIndex][cardIndex].faceUp = true;
}

int SixGolfGameLogic::calculateScore(int playerIndex) const {
    int score = 0;
    for (const auto& card : playerHands[playerIndex]) {
        if (card.faceUp) {
            if (card.rank >= 2 && card.rank <= 10) {
                score += card.rank;
            } else if (card.rank == 11 || card.rank == 12 || card.rank == 13) {
                score += 10;
            } else if (card.rank == 14) {
                score += 1;
            }
        }
    }
    return score;
}

void SixGolfGameLogic::calculateHoleScores() {
    for (int i = 0; i < numPlayers; ++i) {
        playerScores[i][currentHole - 1] = calculateScore(i);
    }
}

std::vector<int> SixGolfGameLogic::getFinalScores() const {
    std::vector<int> finalScores(numPlayers, 0);
    for (int i = 0; i < numPlayers; ++i) {
        finalScores[i] = std::accumulate(playerScores[i].begin(), playerScores[i].end(), 0);
    }
    return finalScores;
}

int SixGolfGameLogic::getWinner() const {
    auto finalScores = getFinalScores();
    return std::distance(finalScores.begin(), std::min_element(finalScores.begin(), finalScores.end()));
}
