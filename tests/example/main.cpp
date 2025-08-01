#include "nm_example.h"
#include <iostream>

int main(int argc, char *argv[]) {
    NetworkManagerExample nmExample;
    
    std::vector<NetworkManagerExample::InterfaceDetails> interfaces = nmExample.GetAvailableInterfaces();
    
    std::cout << "NetworkManager Interfaces:" << std::endl;
    for (const auto &iface : interfaces) {
        std::cout << "Interface: " << iface.name << std::endl;
        std::cout << "  Type: " << iface.type << std::endl;
        std::cout << "  MAC: " << iface.mac << std::endl;
        std::cout << "  Enabled: " << (iface.enabled ? "Yes" : "No") << std::endl;
        std::cout << "  Connected: " << (iface.connected ? "Yes" : "No") << std::endl;
        std::cout << std::endl;
    }
    return 0;
}