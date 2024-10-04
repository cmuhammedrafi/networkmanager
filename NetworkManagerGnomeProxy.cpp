#include "NetworkManagerImplementation.h"
#include "NetworkManagerGnomeClient.h"

using namespace WPEFramework;
using namespace WPEFramework::Plugin;
using namespace std;

namespace WPEFramework
{
    namespace Plugin
    {
        NetworkManagerImplementation* _instance = nullptr;
        NetworkManagerClient* gnomeClient = nullptr;
        void NetworkManagerInternalEventHandler(const char *owner, int eventId, void *data, size_t len)
        {
            return;
        }

        void NetworkManagerImplementation::platform_init()
        {
            ::_instance = this;
            gnomeClient = NetworkManagerClient::getInstance();
        }

        uint32_t NetworkManagerImplementation::GetAvailableInterfaces (Exchange::INetworkManager::IInterfaceDetailsIterator*& interfacesItr/* @out */)
        {
            return Core::ERROR_NONE;
        }

        uint32_t NetworkManagerImplementation::GetPrimaryInterface (string& interface /* @out */)
        {
           return Core::ERROR_NONE;
        }

        uint32_t NetworkManagerImplementation::SetPrimaryInterface (const string& interface/* @in */)
        {
            return Core::ERROR_NONE;
        }

        uint32_t NetworkManagerImplementation::SetInterfaceState(const string& interface/* @in */, const bool& enabled /* @in */)
        {
            return Core::ERROR_NONE;
        }

        uint32_t NetworkManagerImplementation::GetInterfaceState(const string& interface/* @in */, bool& isEnabled /* @out */)
        {
            return Core::ERROR_NONE;
        } 

        uint32_t NetworkManagerImplementation::GetIPSettings(const string& interface /* @in */, const string& ipversion /* @in */, IPAddressInfo& result /* @out */)
        {
            return Core::ERROR_NONE;
        }

        uint32_t NetworkManagerImplementation::SetIPSettings(const string& interface /* @in */, const string &ipversion /* @in */, const IPAddressInfo& address /* @in */)
        {
            return Core::ERROR_NONE;
        }

        uint32_t NetworkManagerImplementation::StartWiFiScan(const WiFiFrequency frequency /* @in */)
        {
            return Core::ERROR_NONE;
        }

        uint32_t NetworkManagerImplementation::StopWiFiScan(void)
        {
            return Core::ERROR_NONE;
        }

        uint32_t NetworkManagerImplementation::GetKnownSSIDs(IStringIterator*& ssids /* @out */)
        {
            uint32_t rc = Core::ERROR_RPC_CALL_FAILED;
            std::list<string> ssidList;
            if(gnomeClient->getKnownSSIDs(ssidList))
            {
                if (!ssidList.empty())
                {
                    ssids = Core::Service<RPC::StringIterator>::Create<RPC::IStringIterator>(ssidList);
                    rc = Core::ERROR_NONE;
                }
                else
                {
                    NMLOG_ERROR("know ssids not found !");
                    rc = Core::ERROR_GENERAL;
                }
            }
            return rc;
        }

        uint32_t NetworkManagerImplementation::AddToKnownSSIDs(const WiFiConnectTo& ssid /* @in */)
        {
            return Core::ERROR_NONE;
        }

        uint32_t NetworkManagerImplementation::RemoveKnownSSID(const string& ssid /* @in */)
        {
            return Core::ERROR_NONE;
        }

        uint32_t NetworkManagerImplementation::WiFiConnect(const WiFiConnectTo& ssid /* @in */)
        {
            return Core::ERROR_NONE;
        }

        uint32_t NetworkManagerImplementation::WiFiDisconnect(void)
        {
            return Core::ERROR_NONE;
        }

        uint32_t NetworkManagerImplementation::GetConnectedSSID(WiFiSSIDInfo&  ssidInfo /* @out */)
        {
            return Core::ERROR_NONE;
        }

        uint32_t NetworkManagerImplementation::GetWiFiSignalStrength(string& ssid /* @out */, string& signalStrength /* @out */, WiFiSignalQuality& quality /* @out */)
        {
           return Core::ERROR_NONE;
        }

        uint32_t NetworkManagerImplementation::GetWifiState(WiFiState &state)
        {
            return Core::ERROR_NONE;
        }

        uint32_t NetworkManagerImplementation::StartWPS(const WiFiWPS& method /* @in */, const string& wps_pin /* @in */)
        {
            return Core::ERROR_NONE;
        }

        uint32_t NetworkManagerImplementation::StopWPS(void)
        {
            return Core::ERROR_NONE;
        }

    }
}
