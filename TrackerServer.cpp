#include "TrackerServer.h"
#include <stdio.h>      // for printf() and fprintf()
#include <sys/socket.h> // for socket() and bind()
#include <arpa/inet.h>  // for sockaddr_in and inet_ntoa()
#include <stdlib.h>     // for atoi() and exit()
#include <string.h>     // for memset()
#include <unistd.h>     // for close()
#include <sstream>

#define ECHOMAX 1000    // Longest string to echo

void DieWithError( const char *errorMessage ) // External error handling function
{
    perror( errorMessage );
    exit( 1 );
}

std::string TrackerServer::registerPlayer() {
    return "register";
}

std::string TrackerServer::queryPlayers() {
    return "query_players";
}

std::string TrackerServer::queryGames() {
    return "query_games";
}

std::string TrackerServer::deregisterPlayer() {
    return "de-register";
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
    char buffer[ ECHOMAX ];             // Buffer for echo string
    unsigned short trackerServPort;         // Server port
    int recvMsgSize;                        // Size of received message

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

	printf( "server: Port server is listening to is: %d\n", trackerServPort );

    for (;;) {
        cliAddrLen = sizeof(trackerClntAddr);

        // Block until you receive a message from a client
        if ((recvMsgSize = recvfrom(sock, buffer, ECHOMAX, 0,
            (struct sockaddr *) &trackerClntAddr, &cliAddrLen)) < 0)
            DieWithError("server: recvfrom() failed");
        
        // New Handling of Client
        buffer[recvMsgSize] = '\0';
        printf("Handling client %s\n", inet_ntoa(trackerClntAddr.sin_addr));
        
        // Handling of Commands
        std::string response;
        std::istringstream iss(buffer);
        std::string command;
        iss >> command;

        
        if (command == "register") {
       
        
        } else if (command == "query_players") {


        } else if (command == "query_games") {
        

        } else if (command == "de-register") {
        

        } else {
            response = "Unknown command";
        }
        
        // Send received datagram back to the client
        if( sendto( sock, buffer, strlen( buffer ), 0, (struct sockaddr *) &trackerClntAddr, sizeof( trackerClntAddr ) ) != strlen( buffer ) )
            DieWithError( "server: sendto() sent a different number of bytes than expected" );
    }
}
