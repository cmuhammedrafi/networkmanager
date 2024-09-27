#pragma once

#include "dbusConnectionManager.h"
#include <string>

class WpaSupplicantClient {
public:
    WpaSupplicantClient();
    ~WpaSupplicantClient();

    std::string getWifiStatus();

private:
    DbusConnectionManager dbusConnection;
};