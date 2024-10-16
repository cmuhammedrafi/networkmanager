#include "NetworkManagerGnomeClient.h"
#include "NetworkManagerGnomeUtils.h"
#include "../../NetworkManagerLogger.h"
#include "INetworkManager.h"
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
       // nmClient->getKnownSSIDs(ssids);
        if (ssids.empty()) {
            NMLOG_INFO("No SSID found");
        } else {
           // NMLOG_INFO("SSID list : %s", GnomeUtils::getCommaSeparatedSSIDs(ssids).c_str());
        }

        Exchange::INetworkManager::WiFiSSIDInfo ssidinfo_test;
        nmClient->getConnectedSSID(ssidinfo_test);
    //     //std::this_thread::sleep_for(std::chrono::milliseconds(1));


    //    // nmClient->getAvailableSSIDs(ssids);
    //     std::string ssid = "Mr13";
    //     std::string psk ="123454321";
    //     nmClient->startWifiScanning();

        Exchange::INetworkManager::WiFiConnectTo ssidinfo = {.m_ssid = "HomeNet", 
                                                             .m_passphrase = "rafi@123",
                                                             .m_securityMode = WPEFramework::Exchange::INetworkManager::WIFI_SECURITY_WPA_PSK_AES,
                                                             .m_persistSSIDInfo = true
                                                             };
       // nmClient->wifiConnect(ssidinfo);
       // sleep(10);
        Exchange::INetworkManager::WiFiState state;
        nmClient->getWifiState(state);
        std::string ssid, signalStrength; 
        Exchange::INetworkManager::WiFiSignalQuality quality;
        nmClient->getWiFiSignalStrength(ssid, signalStrength, quality);
    }
    NMLOG_INFO("Program completed successfully");
    return 0;
}