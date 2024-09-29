#include "TrackerServer.h"
#include <stdio.h>      // for printf() and fprintf()
#include <sys/socket.h> // for socket() and bind()
#include <arpa/inet.h>  // for sockaddr_in and inet_ntoa()
#include <stdlib.h>     // for atoi() and exit()
#include <string.h>     // for memset()
#include <unistd.h>     // for close()
#include <sstream>
#include <map>

#define ECHOMAX 1000    // Longest string to echo

void DieWithError( const char *errorMessage ) // External error handling function
{
    perror( errorMessage );
    exit( 1 );
}

TrackerServer::TrackerServer() : nextGameId(1) {}

std::string TrackerServer::registerPlayer(const std::string& name, const std::string& ipAddress, int tPort, int pPort) {

    if (isPlayerRegistered(name))
        return "FAILURE";
    
    PlayerInfo newPlayer = {name, ipAddress, "free", tPort, pPort};
    addPlayer(newPlayer);
    printf("Player registered: %s\n", name.c_str());
    return "SUCCESS";
}

std::string TrackerServer::deregisterPlayer(const std::string& name) {

    if (!isPlayerRegistered(name))
        return "FAILURE";

    if (players[name].state == "in-play")
        return "FAILURE";

    removePlayer(name);
    printf("Player de-registered: %s\n", name.c_str());
    return "SUCCESS";
}

std::string TrackerServer::queryPlayers() {
    std::ostringstream oss;
    oss << players.size() << "\n";
    for (const auto& pair : players) {
        const PlayerInfo& player = pair.second;
        oss << player.name << " " << player.ipAddress << " " << player.tPort << " " << player.pPort << " " << player.state << "\n";
    }
    return oss.str();
}

std::string TrackerServer::queryGames() {
    std::ostringstream oss;
    oss << games.size() << "\n";
    for (const auto& pair : games) {
        const GameInfo& game = pair.second;
        oss << game.gameId << " " << game.dealer;
        for (const auto& player : game.players) {
            oss << " " << player;
        }
        oss << "\n";
    }
    return oss.str();
}

// Helper functions
bool TrackerServer::isPlayerRegistered(const std::string& name) {
    return players.find(name) != players.end();
}

void TrackerServer::addPlayer(const PlayerInfo& player) {
    players[player.name] = player;
}

void TrackerServer::removePlayer(const std::string& name){
    players.erase(name);
}

int main(int argc, char *argv[]) {

    if (argc != 2) {                        // Test for correct number of parameters
        fprintf(stderr, "Usage: %s <UDP SERVER PORT>\n", argv[0]);
        exit(1);
    }

    int sock;                               // Socket
    struct sockaddr_in trackerServAddr;     // Local address of server
    struct sockaddr_in trackerClntAddr;     // Client address
    unsigned int cliAddrLen;                // Length of incoming message
    char buffer[ ECHOMAX ];                 // Buffer for echo string
    unsigned short trackerServPort;         // Server port
    int recvMsgSize;                        // Size of received message
    std::map<std::string, std::string> ipToPlayerName; // Map of IP addresses to player names

    trackerServPort = atoi(argv[1]);        // First arg: local port

    // Create socket for sending/receiving datagrams
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("server: socket() failed");

    // Construct local address structure
    memset(&trackerServAddr, 0, sizeof(trackerServAddr));
    trackerServAddr.sin_family = AF_INET;
    trackerServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    trackerServAddr.sin_port = htons(trackerServPort);

    // Bind to the local address
    if( bind( sock, (struct sockaddr *) &trackerServAddr, sizeof(trackerServAddr)) < 0 )
        DieWithError( "server: bind() failed" );

    TrackerServer tracker;

    printf("Tracker server is running on port %d\n", trackerServPort);

    for (;;) {
        cliAddrLen = sizeof(trackerClntAddr);

        // Block until you receive a message from a client
        if ((recvMsgSize = recvfrom(sock, buffer, ECHOMAX, 0,
            (struct sockaddr *) &trackerClntAddr, &cliAddrLen)) < 0)
            DieWithError("server: recvfrom() failed");
        
        // New Handling of Client
        buffer[recvMsgSize] = '\0';
        std::string clientIP = inet_ntoa(trackerClntAddr.sin_addr);
        std::string playerName = ipToPlayerName[clientIP];
        
        if (playerName.empty()) {
            printf("Received from client %s: %s\n", clientIP.c_str(), buffer);
        } else {
            printf("Received from client %s (%s): %s\n", clientIP.c_str(), playerName.c_str(), buffer);
        }
        
        // Handling of Commands
        std::string response;
        std::istringstream iss(buffer);
        std::string command;
        iss >> command;
        
        if (command == "register") {
            std::string name, ipAddress;
            int tPort, pPort;
            iss >> name >> ipAddress >> tPort >> pPort;
            response = tracker.registerPlayer(name, ipAddress, tPort, pPort);
            if (response == "SUCCESS") {
                ipToPlayerName[clientIP] = name;
            }
        } else if (command == "query_players") {
            response = tracker.queryPlayers();
        } else if (command == "query_games") {
            response = tracker.queryGames();
        } else if (command == "de-register") {
            std::string name;
            iss >> name;
            response = tracker.deregisterPlayer(name);
            if (response == "SUCCESS") {
                ipToPlayerName.erase(clientIP);
            }
        } else {
            response = "Unknown command";
        }

        if (sendto(sock, response.c_str(), response.length(), 0,
             (struct sockaddr *) &trackerClntAddr, sizeof(trackerClntAddr)) != response.length())
            DieWithError("server: sendto() sent a different number of bytes than expected");
    }
}
