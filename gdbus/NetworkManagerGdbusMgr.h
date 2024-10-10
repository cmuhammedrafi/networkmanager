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
#include <gio/gio.h>
#include <iostream>
#include <string>
#include <list>

/* include NetworkManager.h for the defines, but we don't link against libnm. */
#include <libnm/nm-dbus-interface.h>

namespace WPEFramework
{
    namespace Plugin
    {
        class DbusConnectionManager {
            public:
                DbusConnectionManager();
                ~DbusConnectionManager();

                bool InitializeBusConnection(const std::string& busName);
                void DeinitializeDbusConnection();
                GDBusConnection* getConnection() const;

            private:
                GDBusConnection* connection;
        };

        class DbusMgr {
            public:
                DbusMgr();
                ~DbusMgr();

                GDBusProxy* getNetworkManagerProxy();
                GDBusProxy* getNetworkManagerSettingsProxy();
                GDBusProxy* getNetworkManagerDeviceProxy(const char* devicePath);
                GDBusProxy* getNetworkManagerWirelessProxy(const char* wirelessDevPath);
                GDBusConnection* getConnection();

            private:
                GDBusConnection* connection;
                GDBusProxyFlags flags;
                GDBusProxy *nmProxy = NULL;
            };

    }
}