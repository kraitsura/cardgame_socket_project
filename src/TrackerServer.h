#ifndef TRACKER_SERVER_H
#define TRACKER_SERVER_H

#include "Tracker.h"
#include "Utils.h"
#include <string>

class TrackerServer {
private:
    Tracker tracker;

public:
    TrackerServer();
    std::string formatResponse(const std::string& command, const std::string& trackerResponse);
    std::string handleCommand(const Message& msg);
};

#endif // TRACKER_SERVER_H