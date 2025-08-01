#pragma once

#include <string>
#include <vector>

class NetworkManagerExample {
public:
    // Interface details structure
    struct InterfaceDetails {
        std::string name;
        std::string mac;
        std::string type;  // "WIFI" or "ETHERNET"
        bool enabled;
        bool connected;
    };

    NetworkManagerExample();
    ~NetworkManagerExample();

    // Get network interface names
    std::vector<std::string> getInterfaceNames();
    
    // Get detailed interface information
    std::vector<InterfaceDetails> GetAvailableInterfaces();
};