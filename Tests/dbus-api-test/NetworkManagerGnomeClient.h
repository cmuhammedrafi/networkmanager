#pragma once
#include <string>
#include <list>

#include "dbusConnectionManager.h"
#include "NetworkManagerGnomeUtils.h"


class NetworkManagerClient {
public:
    NetworkManagerClient();
    ~NetworkManagerClient();

    bool getKnownSSIDs(std::list<std::string>& ssids);
    bool getavilableSSID(std::list<std::string>& ssids);
    bool getConnectedSSID();
    bool startWifiScanning(const std::string& ssid);

private:
    DbusConnectionManager dbusConnection;
};
