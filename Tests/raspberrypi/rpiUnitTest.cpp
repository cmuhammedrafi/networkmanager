#include "NetworkManagerGnomeClient.h"
#include "../../NetworkManagerLogger.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace WPEFramework;
using namespace WPEFramework::Plugin;
using namespace std;

int main() {

    NetworkManagerClient* nmClient = NetworkManagerClient::getInstance();
    int i=1;
    while(i-- > 0)
    {
        std::list<std::string> ssids;
        nmClient->getKnownSSIDs(ssids);
        if (ssids.empty()) {
            NMLOG_INFO("No SSID found");
        } else {
            NMLOG_INFO("SSID list available");
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    
        //nmClient->getConnectedSSID();
       // nmClient->getAvailableSSIDs(ssids);
        std::string ssid = "Mr13";
        std::string psk ="123454321";
        nmClient->startWifiScanning();
        nmClient->wifiConnect(ssid, psk);
    }
    NMLOG_INFO("Program completed successfully");
    return 0;
}