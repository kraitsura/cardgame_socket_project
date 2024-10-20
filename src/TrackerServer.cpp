#include "TrackerServer.h"
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sstream>
#include <map>
#include <vector>
#include <algorithm>

#define ECHOMAX 1024    // Longest string to echo

TrackerServer::TrackerServer() : tracker() {}

std::string TrackerServer::formatResponse(const std::string& command, const std::string& trackerResponse) {
    if (trackerResponse.empty()) {
        return "FAILURE " + command + " Empty response from tracker";
    }
    
    if (trackerResponse.substr(0, 7) == "SUCCESS") {
        return "SUCCESS " + command + " " + (trackerResponse.length() > 8 ? trackerResponse.substr(8) : "");
    } else {
        return "FAILURE " + command + " " + trackerResponse;
    }
}

std::string TrackerServer::handleCommand(const Message& msg) {
    std::istringstream iss(msg.data);
    std::string response;

    switch (msg.cmd)
    {
        case CMD_REGISTER:
        {
            std::string name, ipAddress;
            int tPort, pPort;
            iss >> name >> ipAddress >> tPort >> pPort;
            response = tracker.registerPlayer(name, ipAddress, tPort, pPort);
            response = formatResponse("REGISTER", response);
            break;
        }
        case CMD_QUERY_PLAYERS:
            response = tracker.queryPlayers();
            response = formatResponse("QUERY_PLAYERS", response);
            break;
        case CMD_QUERY_GAMES:
            response = tracker.queryGames();
            response = formatResponse("QUERY_GAMES", response);
            break;
        case CMD_START_GAME: {
            std::string dealer;
            int n, holes;
            iss >> dealer >> n >> holes;
            response = tracker.startGame(dealer, n, holes);
            response = formatResponse("START_GAME", response);
            break;
        }
        case CMD_END_GAME: {
            int gameId;
            std::string dealer;
            iss >> gameId >> dealer;
            response = tracker.endGame(gameId, dealer);
            response = formatResponse("END_GAME", response);
            break;
        }
        case CMD_DEREGISTER: {
            std::string name;
            iss >> name;
            response = tracker.deregisterPlayer(name);
            response = formatResponse("DEREGISTER", response);
            break;
        }
        default:
            response = "FAILURE Unknown command";
        }

    return response;
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <UDP SERVER PORT>\n", argv[0]);
        exit(1);
    }

    int sock;                               // Socket
    struct sockaddr_in trackerServAddr;     // Local address of server
    struct sockaddr_in trackerClntAddr;     // Client address
    unsigned int cliAddrLen;                // Length of incoming message
    Message msg;                            // Buffer for incoming message
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
    if (bind(sock, (struct sockaddr *) &trackerServAddr, sizeof(trackerServAddr)) < 0)
        DieWithError("server: bind() failed");

    TrackerServer trackerServer;

    printf("Tracker server is running on port %d\n", trackerServPort);

    for (;;) {
        cliAddrLen = sizeof(trackerClntAddr);

        // Block until you receive a message from a client
        if ((recvMsgSize = recvfrom(sock, &msg, sizeof(Message), 0,
            (struct sockaddr *) &trackerClntAddr, &cliAddrLen)) < 0)
            DieWithError("server: recvfrom() failed");
        
        std::string clientIP = inet_ntoa(trackerClntAddr.sin_addr);
        std::string playerName = ipToPlayerName[clientIP];
        
        if (playerName.empty()) {
            printf("Received from client %s: Command %d, Data: %s\n", clientIP.c_str(), msg.cmd, msg.data);
        } else {
            printf("Received from client %s (%s): Command %d, Data: %s\n", clientIP.c_str(), playerName.c_str(), msg.cmd, msg.data);
        }
        
        // Handle the command using the new TrackerServer implementation
        std::string response = trackerServer.handleCommand(msg);

        // If this was a successful registration, update the ipToPlayerName map
        if (msg.cmd == CMD_REGISTER && response.substr(0, 7) == "SUCCESS") {
            std::istringstream iss(msg.data);
            std::string command, name, ip;
            int tPort, pPort;
            iss >> command >> name >> ip >> tPort >> pPort;
            ipToPlayerName[clientIP] = name;
        }

        // If this was a successful de-registration, remove from the ipToPlayerName map
        if (msg.cmd == CMD_DEREGISTER && response.substr(0, 7) == "SUCCESS") {
            ipToPlayerName.erase(clientIP);
        }

        // Send the response back to the client
        if (sendto(sock, response.c_str(), response.length(), 0,
             (struct sockaddr *) &trackerClntAddr, sizeof(trackerClntAddr)) != response.length())
            DieWithError("server: sendto() sent a different number of bytes than expected");
        
        printf("Sent response to client %s: %s\n", clientIP.c_str(), response.c_str());
    }

    // Close the socket (this part will never be reached in this implementation)
    close(sock);
    return 0;
}