#include "NetworkManagerGnomeClient.h"
#include "dbusConnectionManager.h"
#include <iostream>
#include <thread>
#include <chrono>


int main() {
    // Create instances of the NetworkManager and wpa_supplicant clients
    NetworkManagerClient nmClient;
   // WpaSupplicantClient wpaClient;
    int i=1;
    while(i-- > 0)
    {
        std::list<std::string> ssids;
        nmClient.getKnownSSIDs(ssids);
        if (ssids.empty()) {
            NMLOG_INFO("No SSID found");
        } else {
            NMLOG_INFO("SSID list available");
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    
        nmClient.getConnectedSSID();
        nmClient.getavilableSSID(ssids);
    }
    NMLOG_INFO("Program completed successfully");
    return 0;
}
