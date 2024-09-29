#include <stdio.h>      // for printf() and fprintf()
#include <sys/socket.h> // for socket(), connect(), sendto(), and recvfrom()
#include <arpa/inet.h>  // for sockaddr_in and inet_addr()
#include <stdlib.h>     // for atoi() and exit()
#include <string.h>     // for memset()
#include <unistd.h>     // for close()
#include <stdbool.h>    // for bool type

#define ECHOMAX 1024 // Maximum buffer size for messages

void DieWithError(const char *errorMessage) // Error handling function
{
    perror(errorMessage);
    exit(1);
}

int main(int argc, char *argv[])
{
    int sock;                        // Socket descriptor
    struct sockaddr_in servAddr;     // Server address
    struct sockaddr_in fromAddr;     // Source address of response
    unsigned short servPort;         // Server port
    unsigned int fromSize;           // In-out of address size for recvfrom()
    char *servIP;                    // IP address of server
    char sendBuffer[ECHOMAX];     // Buffer for sending data
    char recvBuffer[ECHOMAX];     // Buffer for receiving data
    int respStringLen;               // Length of received response
    bool isPlayerRegistered = false; // Flag to track if a player is registered

    if (argc < 3)    // Test for correct number of arguments
    {
        fprintf(stderr, "Usage: %s <Server IP> <Server Port>\n", argv[0]);
        exit(1);
    }

    servIP = argv[1];           // First arg: server IP address (dotted decimal)
    servPort = atoi(argv[2]);   // Second arg: Use given port

    // Create a datagram/UDP socket
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("PlayerClient: socket() failed");

    // Construct the server address structure
    memset(&servAddr, 0, sizeof(servAddr));    // Zero out structure
    servAddr.sin_family = AF_INET;             // Use internet addr family
    servAddr.sin_addr.s_addr = inet_addr(servIP); // Server's IP address
    servAddr.sin_port = htons(servPort);       // Server's port

    while (1)
    {
        printf("\nEnter command (or 'quit' to exit):\n");
        fgets(sendBuffer, ECHOMAX, stdin);
        sendBuffer[strcspn(sendBuffer, "\n")] = 0; // Remove newline

        if (strcmp(sendBuffer, "quit") == 0)
            break;

        // Check if the command is a registration attempt
        if (strncmp(sendBuffer, "register", 8) == 0)
        {
            if (isPlayerRegistered)
            {
                printf("PlayerClient: Error: A player is already registered in this session.\n");
                continue; // Skip to next iteration of the loop
            }
        }

        // Send the command to the server
        if (sendto(sock, sendBuffer, strlen(sendBuffer), 0, (struct sockaddr *)&servAddr, sizeof(servAddr)) != strlen(sendBuffer))
            DieWithError("PlayerClient: sendto() sent a different number of bytes than expected");

        // Receive a response
        fromSize = sizeof(fromAddr);
        if ((respStringLen = recvfrom(sock, recvBuffer, ECHOMAX, 0, (struct sockaddr *)&fromAddr, &fromSize)) < 0)
            DieWithError("PlayerClient: recvfrom() failed");

        recvBuffer[respStringLen] = '\0';  // Null-terminate the received data

        if (servAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr)
            DieWithError("PlayerClient: Error: received a packet from unknown source.\n");

        printf("PlayerClient: Received response from server: %s\n", recvBuffer);

        // If registration was successful, update the flag
        if (strncmp(sendBuffer, "register", 8) == 0 && strcmp(recvBuffer, "SUCCESS") == 0)
        {
            isPlayerRegistered = true;
            printf("PlayerClient: Player successfully registered.\n");
        }
        // If de-registration was successful, update the flag
        else if (strncmp(sendBuffer, "de-register", 11) == 0 && strcmp(recvBuffer, "SUCCESS") == 0)
        {
            isPlayerRegistered = false;
            printf("PlayerClient: Player successfully de-registered.\n");
        }
    }

    close(sock);
    return 0;
}