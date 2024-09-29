#ifndef PLAYER_CLIENT_H
#define PLAYER_CLIENT_H

#include <string>

class PlayerClient {
private:
    int sock;
    std::string serverIP;
    unsigned short serverPort;
    
public:
    PlayerClient(const std::string& serverIP, unsigned short serverPort);
    ~PlayerClient();

    std::string sendRequest(const std::string& request);

    // Wrapper functions for specific requests
    std::string registerPlayer(const std::string& name, const std::string& ipAddress, int tPort, int pPort);
    std::string queryPlayers();
    std::string queryGames();
    std::string deregisterPlayer(const std::string& name);
};

#endif // PLAYER_CLIENT_H
