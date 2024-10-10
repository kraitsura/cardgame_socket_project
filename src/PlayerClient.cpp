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
#include <mutex>
#include <condition_variable>
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
    bool isRegistered = false;
    std::vector<int> peerSockets;
    std::mutex mtx;
    std::condition_variable cv;
    std::string lastResponse;

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
                
                std::lock_guard<std::mutex> lock(mtx);
                lastResponse = buffer;
                cv.notify_one();
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

    bool waitForResponse(const std::string &expectedPrefix, int timeoutSeconds = 5)
    {
        std::unique_lock<std::mutex> lock(mtx);
        bool received = cv.wait_for(lock, std::chrono::seconds(timeoutSeconds),
            [this, &expectedPrefix]() {
                return !lastResponse.empty() && lastResponse.substr(0, expectedPrefix.length()) == expectedPrefix;
            });

        if (received)
        {
            const char *buffer = lastResponse.c_str();
            if (strncmp(buffer, "SUCCESS REGISTER", 16) == 0)
            {
                std::cout << "Successfully registered!" << std::endl;
                isRegistered = true;
            }
            else if (strncmp(buffer, "SUCCESS DEREGISTER", 18) == 0)
            {
                std::cout << "Successfully deregistered!" << std::endl;
                isRegistered = false;
            }
            else if (strncmp(buffer, "SUCCESS START_GAME", 18) == 0)
            {
                std::cout << "Game started successfully!" << std::endl;
                std::string gameInfo = buffer + 19;
                std::cout << "Game info: " << gameInfo << std::endl;
                setupPeerConnections(buffer + 19); // Skip "SUCCESS START_GAME "
            }
            else if (strncmp(buffer, "SUCCESS QUERY_PLAYERS", 21) == 0)
            {
                std::cout << "Player query successful. Players:" << std::endl;
                std::cout << buffer + 22 << std::endl; // Skip "SUCCESS QUERY_PLAYERS "
            }
            else if (strncmp(buffer, "SUCCESS QUERY_GAMES", 19) == 0)
            {
                std::cout << "Game query successful. Games:" << std::endl;
                std::cout << buffer + 20 << std::endl; // Skip "SUCCESS QUERY_GAMES "
            }
            else if (strncmp(buffer, "FAILURE", 7) == 0)
            {
                std::cout << "Operation failed: " << buffer + 8 << std::endl;
            }
            lastResponse.clear();
        }
        else
        {
            std::cout << "No response received or unexpected response." << std::endl;
        }

        return received;
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

    void sendMessage(CommandType cmd, const std::string &data)
    {
        Message msg;
        msg.cmd = cmd;
        strncpy(msg.data, data.c_str(), sizeof(msg.data) - 1);
        msg.data[sizeof(msg.data) - 1] = '\0';

        if (sendto(trackerSock, &msg, sizeof(msg), 0, (struct sockaddr *)&trackerServAddr, sizeof(trackerServAddr)) != sizeof(msg))
            DieWithError("sendto() sent a different number of bytes than expected");

        std::cout << cmdToString(cmd) << " request sent. Waiting for response..." << std::endl;
        waitForResponse("SUCCESS " + std::string(cmdToString(cmd)));
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

        std::string data = name + " " + ip + " " + std::to_string(trackerPort) + " " + std::to_string(peerPort);
        sendMessage(CMD_REGISTER, data);
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

        waitForResponse("SUCCESS DEREGISTER");
    }

    void startGame(const std::string& dealer, int n, int holes) {
        if (!isRegistered) {
            std::cout << "You must be registered to start a game." << std::endl;
            return;
        }
        std::string data = dealer + " " + std::to_string(n) + " " + std::to_string(holes);
        sendMessage(CMD_START_GAME, data);
    }

    void endGame(int gameId, const std::string& dealer) {
        if (!isRegistered) {
            std::cout << "You must be registered to end a game." << std::endl;
            return;
        }
        std::string data = std::to_string(gameId) + " " + dealer;
        sendMessage(CMD_END_GAME, data);
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
                std::string dealer;
                int numPlayers, numHoles;
                if (iss >> dealer >> numPlayers >> numHoles) {
                    startGame(dealer, numPlayers, numHoles);
                } else {
                    std::cout << "Usage: start <dealer> <num_players> <num_holes>" << std::endl;
                }
            } else if (cmd == "end") {
                int gameId;
                std::string dealer;
                if (iss >> gameId >> dealer) {
                    endGame(gameId, dealer);
                } else {
                    std::cout << "Usage: end <game_id> <dealer>" << std::endl;
                }
            } else if (cmd == "query_players") {
                sendMessage(CMD_QUERY_PLAYERS, "");
            } else if (cmd == "query_games") {
                sendMessage(CMD_QUERY_GAMES, "");
            } else if (cmd == "help") {
                ShowHelp();
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