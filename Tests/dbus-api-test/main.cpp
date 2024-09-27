#include "networkManagerClient.h"
#include "wpaSupplicantClient.h"
#include "dbusConnectionManager.h"
#include <iostream>
#include <thread>
#include <chrono>


int main() {
    // Create instances of the NetworkManager and wpa_supplicant clients
    NetworkManagerClient nmClient;
   // WpaSupplicantClient wpaClient;
    int i=100;
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
        NMLOG_INFO("count %d", i);
    }
    // std::string wifiStatus = wpaClient.getWifiStatus();
    // if (!wifiStatus.empty()) {
    //     NMLOG_INFO("Wi-Fi status: %s", wifiStatus.c_str());
    // } else {
    //     NMLOG_WARNING("Failed to retrieve Wi-Fi status");
    // }

    // Sleep for a short period to simulate real-world conditions

    NMLOG_INFO("Program completed successfully");
    return 0;
}
