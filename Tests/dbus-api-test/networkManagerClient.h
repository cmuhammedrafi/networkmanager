#pragma once

#include "dbusConnectionManager.h"
#include <string>
#include <list>

class NetworkManagerClient {
public:
    NetworkManagerClient();
    ~NetworkManagerClient();

    bool getKnownSSIDs(std::list<std::string>& ssids);

private:
    DbusConnectionManager dbusConnection;
};
