/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 RDK Management
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
 */

#pragma once
#include <NetworkManager.h>
#include <string.h>
#include <iostream>
#include <atomic>

#include "NetworkManagerGdbusMgr.h"
#include "INetworkManager.h"

namespace WPEFramework
{
    namespace Plugin
    {

    typedef struct {
        GDBusProxy *wirelessDevProxy;
    } GdbusProxys;

    typedef struct {
        std::string activeConnPath;
        std::string wifiDevicePath;
        std::string ethDevicePath;
        std::string ipV4ConfPath;
        std::string ipV6ConfPath;
        GMainLoop *loop;
    } NMEvents;

    class NetworkManagerEvents
    {

    public:
        static void onInterfaceStateChangeCb(Exchange::INetworkManager::InterfaceState newState, std::string iface); // ReportInterfaceStateChangedEvent
        static void onAddressChangeCb(std::string iface, std::string ipAddress, bool acqired, bool isIPv6); // ReportIPAddressChangedEvent
        static void onActiveInterfaceChangeCb(std::string newInterface); // ReportActiveInterfaceChangedEvent
        static void onAvailableSSIDsCb(const char* wifiDevicePath); // ReportAvailableSSIDsEvent
        static void onWIFIStateChanged(Exchange::INetworkManager::WiFiState state, std::string& wifiStateStr); // ReportWiFiStateChangedEvent

    public:
        static NetworkManagerEvents* getInstance();
        bool startNetworkMangerEventMonitor();
        void stopNetworkMangerEventMonitor();
        void setwifiScanOptions(bool doNotify);

    private:
        static void* networkMangerEventMonitor(void *arg);
        NetworkManagerEvents();
        ~NetworkManagerEvents();
        std::atomic<bool>isEventThrdActive = {false};
        std::atomic<bool>doScanNotify = {true};
        NMEvents nmEvents;
        GThread *eventThrdID;
    public:
        GdbusProxys gdbusProxys;
        DbusMgr eventDbus;
    };

    }   // Plugin
}   // WPEFramework
