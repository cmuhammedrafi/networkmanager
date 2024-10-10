#include "NetworkManagerGnomeClient.h"
#include "../../NetworkManagerLogger.h"
#include "INetworkManagerTmp.h"
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
        //std::this_thread::sleep_for(std::chrono::milliseconds(1));

        //nmClient->getConnectedSSID();
       // nmClient->getAvailableSSIDs(ssids);
        std::string ssid = "Mr13";
        std::string psk ="123454321";
        nmClient->startWifiScanning();

        Exchange::INetworkManager::WiFiConnectTo ssidinfo = {.m_ssid = "Mi11", 
                                                             .m_passphrase = "123454321",
                                                             .m_securityMode = WPEFramework::Exchange::INetworkManager::WIFI_SECURITY_WPA_PSK_AES,
                                                             .m_persistSSIDInfo = true
                                                             };
        nmClient->removeKnownSSIDs(ssidinfo.m_ssid);
        nmClient->wifiConnect(ssidinfo);
        sleep(50);
        nmClient->wifiDisconnect();
    }
    NMLOG_INFO("Program completed successfully");
    return 0;
}