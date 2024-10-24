/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2022 RDK Management
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
**/

#include "NetworkManagerImplementation.h"
#include "NetworkManagerGdbusClient.h"

using namespace WPEFramework;
using namespace WPEFramework::Plugin;
using namespace std;

namespace WPEFramework
{
    namespace Plugin
    {
        NetworkManagerImplementation* _instance = nullptr;
        NetworkManagerClient* gnomeDbusClient = nullptr;
        void NetworkManagerInternalEventHandler(const char *owner, int eventId, void *data, size_t len)
        {
            return;
        }

        void NetworkManagerImplementation::platform_init()
        {
            ::_instance = this;
            gnomeDbusClient = NetworkManagerClient::getInstance();
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
            uint32_t rc = Core::ERROR_GENERAL;
            if(gnomeDbusClient->startWifiScan())
                rc = Core::ERROR_NONE;
            else
                NMLOG_ERROR("StartWiFiScan failed");
            return rc;
        }

        uint32_t NetworkManagerImplementation::StopWiFiScan(void)
        {
            // TODO FIX
            uint32_t rc = Core::ERROR_GENERAL;
            if(gnomeDbusClient->stopWiFiScan())
                rc = Core::ERROR_NONE;
            else
                NMLOG_ERROR("stopWiFiScan failed");
            return rc;
        }

        uint32_t NetworkManagerImplementation::GetKnownSSIDs(IStringIterator*& ssids /* @out */)
        {
            uint32_t rc = Core::ERROR_GENERAL;
            std::list<string> ssidList;
            if(gnomeDbusClient->getKnownSSIDs(ssidList) && !ssidList.empty())
            {
                ssids = Core::Service<RPC::StringIterator>::Create<RPC::IStringIterator>(ssidList);
                rc = Core::ERROR_NONE;
            }
            else
                NMLOG_ERROR("GetKnownSSIDs failed");

            return rc;
        }

        uint32_t NetworkManagerImplementation::AddToKnownSSIDs(const WiFiConnectTo& ssid /* @in */)
        {
            uint32_t rc = Core::ERROR_GENERAL;
            if(gnomeDbusClient->addToKnownSSIDs(ssid))
                rc = Core::ERROR_NONE;
            else
                NMLOG_ERROR("addToKnownSSIDs failed");
            return rc;
        }

        uint32_t NetworkManagerImplementation::RemoveKnownSSID(const string& ssid /* @in */)
        {
            uint32_t rc = Core::ERROR_GENERAL;
            if(gnomeDbusClient->removeKnownSSIDs(ssid))
                rc = Core::ERROR_NONE;
            else
                NMLOG_ERROR("RemoveKnownSSID failed");
            return rc;
        }

        uint32_t NetworkManagerImplementation::WiFiConnect(const WiFiConnectTo& ssid /* @in */)
        {
            uint32_t rc = Core::ERROR_GENERAL;
            if(gnomeDbusClient->wifiConnect(ssid))
                rc = Core::ERROR_NONE;
            else
                NMLOG_ERROR("WiFiConnect failed");
            return rc;
        }

        uint32_t NetworkManagerImplementation::WiFiDisconnect(void)
        {
            uint32_t rc = Core::ERROR_GENERAL;
            if(gnomeDbusClient->wifiDisconnect())
                rc = Core::ERROR_NONE;
            else
                NMLOG_ERROR("WiFiDisconnect failed");
            return rc;
        }

        uint32_t NetworkManagerImplementation::GetConnectedSSID(WiFiSSIDInfo&  ssidInfo /* @out */)
        {
            uint32_t rc = Core::ERROR_GENERAL;
            if(gnomeDbusClient->getConnectedSSID(ssidInfo))
                rc = Core::ERROR_NONE;
            else
                NMLOG_ERROR("getConnectedSSID failed");
            return rc;
        }

        uint32_t NetworkManagerImplementation::GetWiFiSignalStrength(string& ssid /* @out */, string& signalStrength /* @out */, WiFiSignalQuality& quality /* @out */)
        {
            uint32_t rc = Core::ERROR_GENERAL;
            if(gnomeDbusClient->getWiFiSignalStrength(ssid, signalStrength, quality))
                rc = Core::ERROR_NONE;
            else
                NMLOG_ERROR("GetWiFiSignalStrength failed");
            return rc;
        }

        uint32_t NetworkManagerImplementation::GetWifiState(WiFiState &state)
        {
            uint32_t rc = Core::ERROR_GENERAL;
            if(gnomeDbusClient->getWifiState(state))
                rc = Core::ERROR_NONE;
            else
                NMLOG_ERROR("GetWifiState failed");
            return rc;
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