# cardgame_socket_project
Six Card Golf implemented in C++ using sockets

## Project Overview

This project implements a peer-to-peer (P2P) application for tracking players and games in the card game "Six Card Golf". It consists of two main components:

1. TrackerServer: A central server that manages player registrations and game information.
2. PlayerClient: A client application that players use to interact with the TrackerServer.

The system uses UDP for communication between clients and the server.

## Features

- Player registration and de-registration
- Querying registered players
- Querying ongoing games
- Basic game management (future implementation)

## Requirements

- C++11 compatible compiler (e.g., GCC 4.8+, Clang 3.3+)
- POSIX-compliant operating system (Linux, macOS, etc.)

## File Structure

```
.
├── TrackerServer.h
├── TrackerServer.cpp
├── PlayerClient.cpp
└── README.md
```

## Compilation Instructions

To compile the TrackerServer:

```bash
g++ -o server TrackerServer.cpp -std=c++11
```

To compile the PlayerClient:

```bash
g++ -o client PlayerClient.cpp -std=c++11
```

## Usage Instructions

### Starting the TrackerServer

Run the TrackerServer with the following command:

```bash
./trackerServer <port_number>
```

Replace `<port_number>` with the desired port number (e.g., 1500).

### Using the PlayerClient

Run the PlayerClient with the following command:

```bash
./playerClient <server_ip> <server_port>
```

Replace `<server_ip>` with the IP address of the TrackerServer and `<server_port>` with the port number the server is listening on.

### Available Commands

The PlayerClient supports the following commands:

1. Register a player:
   ```
   register <player_name> <ip_address> <t_port> <p_port>
   ```

2. Query registered players:
   ```
   query_players
   ```

3. Query ongoing games:
   ```
   query_games
   ```

4. De-register a player:
   ```
   de-register <player_name>
   ```

5. Exit the client:
   ```
   quit
   ```

## Implementation Details

### TrackerServer

- Uses an unordered_map to store player information and game information.
- Implements a multi-threaded design to handle multiple client connections simultaneously.
- Provides real-time logging of client interactions and server operations.

### PlayerClient

- Implements a simple command-line interface for user interactions.
- Uses UDP sockets for communication with the TrackerServer.
- Provides feedback on the success or failure of operations.







