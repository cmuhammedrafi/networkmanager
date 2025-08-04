#pragma once

#include <string>
#include <vector>

class NetworkManagerExample {
public:
    NetworkManagerExample();
    ~NetworkManagerExample();

    // Get network interface names from NetworkManager
    std::vector<std::string> getInterfaceNames();
};