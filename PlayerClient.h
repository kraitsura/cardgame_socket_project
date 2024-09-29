#ifndef PLAYER_CLIENT_H
#define PLAYER_CLIENT_H

#include <string>

class PlayerClient {
    public:
        PlayerClient();

        std::string sendRequest();
        std::string registerPlayer();
        std::string queryPlayers();
        std::string queryGames();
        std::string deregisterPlayer();

};

#endif // PLAYER_CLIENT_H