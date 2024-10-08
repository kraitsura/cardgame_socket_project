#include <iostream>
#include <cstring>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <thread>
#include <vector>
#include <sstream>
#include <map>
#include "Utils.h"

#define ECHOMAX 1024

class PlayerClient {
private:
    int trackerSock;
    struct sockaddr_in trackerServAddr;
    std::string playerName;
    std::string playerIP;
    int tPort;
    int pPort;
    bool isRegistered;
    std::vector<int> peerSockets;

    void setupNonBlocking(int sock) {
        int flags = fcntl(sock, F_GETFL, 0);
        if (flags == -1) DieWithError("fcntl F_GETFL");
        flags |= O_NONBLOCK;
        if (fcntl(sock, F_SETFL, flags) == -1) DieWithError("fcntl F_SETFL O_NONBLOCK");
    }

    void handleTrackerCommunication() {
        char buffer[ECHOMAX];
        struct sockaddr_in fromAddr;
        unsigned int fromSize = sizeof(fromAddr);
        int recvMsgSize;

        while (true) {
            if ((recvMsgSize = recvfrom(trackerSock, buffer, ECHOMAX, 0, 
                (struct sockaddr *) &fromAddr, &fromSize)) > 0) {
                buffer[recvMsgSize] = '\0';
                std::cout << "Received from tracker: " << buffer << std::endl;
                // Handle tracker responses here
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    void handlePeerCommunication(int sock, const std::string& peerName) {
        char buffer[ECHOMAX];
        struct sockaddr_in fromAddr;
        unsigned int fromSize = sizeof(fromAddr);
        int recvMsgSize;

        while (true) {
            if ((recvMsgSize = recvfrom(sock, buffer, ECHOMAX, 0, 
                (struct sockaddr *) &fromAddr, &fromSize)) > 0) {
                buffer[recvMsgSize] = '\0';
                std::cout << "Received from " << peerName << ": " << buffer << std::endl;
                // Handle peer messages here
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

public:
    PlayerClient(const char *servIP, unsigned short servPort) : isRegistered(false) {
        if ((trackerSock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
            DieWithError("socket() failed");

        memset(&trackerServAddr, 0, sizeof(trackerServAddr));
        trackerServAddr.sin_family = AF_INET;
        trackerServAddr.sin_addr.s_addr = inet_addr(servIP);
        trackerServAddr.sin_port = htons(servPort);

        setupNonBlocking(trackerSock);
    }

    void registerPlayer(const std::string& name, const std::string& ip, int trackerPort, int peerPort) {
        if (isRegistered) {
            std::cout << "Already registered. Please de-register first." << std::endl;
            return;
        }

        playerName = name;
        playerIP = ip;
        tPort = trackerPort;
        pPort = peerPort;

        Message msg;
        msg.cmd = CMD_REGISTER;
        snprintf(msg.data, sizeof(msg.data), "%s %s %d %d", name.c_str(), ip.c_str(), trackerPort, peerPort);

        if (sendto(trackerSock, &msg, sizeof(msg), 0, (struct sockaddr *)&trackerServAddr, sizeof(trackerServAddr)) != sizeof(msg))
            DieWithError("sendto() sent a different number of bytes than expected");

        std::cout << "Registration request sent. Waiting for response..." << std::endl;
    }

    void deregisterPlayer() {
        if (!isRegistered) {
            std::cout << "Not registered. Please register first." << std::endl;
            return;
        }

        Message msg;
        msg.cmd = CMD_DEREGISTER;
        snprintf(msg.data, sizeof(msg.data), "%s", playerName.c_str());

        if (sendto(trackerSock, &msg, sizeof(msg), 0, (struct sockaddr *)&trackerServAddr, sizeof(trackerServAddr)) != sizeof(msg))
            DieWithError("sendto() sent a different number of bytes than expected");

        std::cout << "De-registration request sent. Waiting for response..." << std::endl;
    }

    void startGame(int n, int holes) {
        if (!isRegistered) {
            std::cout << "You must be registered to start a game." << std::endl;
            return;
        }

        Message msg;
        msg.cmd = CMD_START_GAME;
        snprintf(msg.data, sizeof(msg.data), "%s %d %d", playerName.c_str(), n, holes);

        if (sendto(trackerSock, &msg, sizeof(msg), 0, (struct sockaddr *)&trackerServAddr, sizeof(trackerServAddr)) != sizeof(msg))
            DieWithError("sendto() sent a different number of bytes than expected");

        std::cout << "Game start request sent. Waiting for response..." << std::endl;
    }

    void setupPeerConnections(const char* gameInfo) {
        std::istringstream iss(gameInfo);
        std::string success;
        int gameId, holes, numPlayers;
        iss >> success >> gameId >> holes >> numPlayers;

        for (int i = 0; i < numPlayers; ++i) {
            std::string name, ip, rightName, rightIp;
            int port, rightPort;
            iss >> name >> ip >> port >> rightName >> rightIp >> rightPort;

            if (name != playerName) {
                int sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
                if (sock < 0) DieWithError("socket() failed");

                setupNonBlocking(sock);

                struct sockaddr_in peerAddr;
                memset(&peerAddr, 0, sizeof(peerAddr));
                peerAddr.sin_family = AF_INET;
                peerAddr.sin_addr.s_addr = inet_addr(ip.c_str());
                peerAddr.sin_port = htons(port);

                peerSockets.push_back(sock);

                // Start a new thread to handle communication with this peer
                std::thread peerThread(&PlayerClient::handlePeerCommunication, this, sock, name);
                peerThread.detach();
            }
        }

        std::cout << "Peer connections set up. Ready to play!" << std::endl;
    }

    void queryPlayers() {
        Message msg;
        msg.cmd = CMD_QUERY_PLAYERS;
        
        if (sendto(trackerSock, &msg, sizeof(msg), 0, (struct sockaddr *)&trackerServAddr, sizeof(trackerServAddr)) != sizeof(msg))
            DieWithError("sendto() sent a different number of bytes than expected");

        std::cout << "Query players request sent. Waiting for response..." << std::endl;
    }

    void queryGames() {
        Message msg;
        msg.cmd = CMD_QUERY_GAMES;
        
        if (sendto(trackerSock, &msg, sizeof(msg), 0, (struct sockaddr *)&trackerServAddr, sizeof(trackerServAddr)) != sizeof(msg))
            DieWithError("sendto() sent a different number of bytes than expected");

        std::cout << "Query games request sent. Waiting for response..." << std::endl;
    }

    void showHelp() {
        std::cout << "Available commands:" << std::endl;
        std::cout << "  register <name> <ip> <tracker_port> <peer_port> - Register player" << std::endl;
        std::cout << "  deregister - De-register player" << std::endl;
        std::cout << "  start <num_players> <num_holes> - Start a new game" << std::endl;
        std::cout << "  query_players - Query registered players" << std::endl;
        std::cout << "  query_games - Query ongoing games" << std::endl;
        std::cout << "  help - Show this help message" << std::endl;
        std::cout << "  quit - Exit the program" << std::endl;
    }

    void run() {
        // Start a thread to handle tracker communication
        std::thread trackerThread(&PlayerClient::handleTrackerCommunication, this);
        trackerThread.detach();

        std::string command;
        while (true) {
            std::cout << "> ";
            std::getline(std::cin, command);

            std::istringstream iss(command);
            std::string cmd;
            iss >> cmd;

            if (cmd == "quit") {
                break;
            } else if (cmd == "register") {
                std::string name, ip;
                int trackerPort, peerPort;
                if (iss >> name >> ip >> trackerPort >> peerPort) {
                    registerPlayer(name, ip, trackerPort, peerPort);
                } else {
                    std::cout << "Usage: register <name> <ip> <tracker_port> <peer_port>" << std::endl;
                }
            } else if (cmd == "deregister") {
                deregisterPlayer();
            } else if (cmd == "start") {
                int numPlayers, numHoles;
                if (iss >> numPlayers >> numHoles) {
                    startGame(numPlayers, numHoles);
                } else {
                    std::cout << "Usage: start <num_players> <num_holes>" << std::endl;
                }
            } else if (cmd == "query_players") {
                queryPlayers();
            } else if (cmd == "query_games") {
                queryGames();
            } else if (cmd == "help") {
                showHelp();
            } else {
                std::cout << "Unknown command. Type 'help' for a list of commands." << std::endl;
            }
        }
    }

    ~PlayerClient() {
        close(trackerSock);
        for (int sock : peerSockets) {
            close(sock);
        }
    }
};

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <Server IP> <Server Port>\n", argv[0]);
        exit(1);
    }

    PlayerClient client(argv[1], atoi(argv[2]));
    
    std::cout << "Welcome to the Six Card Golf client!" << std::endl;
    std::cout << "Type 'help' for a list of available commands." << std::endl;

    client.run();

    return 0;
}