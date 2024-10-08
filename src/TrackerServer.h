#ifndef TRACKER_SERVER_H
#define TRACKER_SERVER_H

#include "Tracker.h"
#include "Utils.h"
#include <string>

class TrackerServer {
private:
    Tracker tracker;

    std::string startGame(const std::string& dealer, int n, int holes);

public:
    TrackerServer();

    std::string handleCommand(const Message& msg);
};

#endif // TRACKER_SERVER_H