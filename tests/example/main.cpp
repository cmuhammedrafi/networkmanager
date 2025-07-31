#include "nm_example.h"
#include <iostream>

int main(int argc, char *argv[]) {
    NetworkManagerExample nmExample;
    
    std::vector<std::string> interfaces = nmExample.getInterfaceNames();
    
    std::cout << "NetworkManager Interfaces:" << std::endl;
    for (const auto &iface : interfaces) {
        std::cout << " - " << iface << std::endl;
    }
    
    return 0;
}