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
// #include <NetworkManager.h>
#include <nm-dbus-interface.h>

struct apProperties
{
    uint32_t flags;
    uint32_t wpaFlags;
    uint32_t rsnFlags;
    uint32_t frequency;
    uint32_t mode;
    std::string ssid;
    std::string bssid;
    uint8_t strength;
};

namespace WPEFramework
{
    namespace Plugin
    {
        class GnomeUtils {
            public:
                static void printKeyVariant(const char *setting_name, GVariant *setting);
                static bool getDeviceByIpIface(GDBusConnection *dbusConn, const gchar *iface_name, std::string& path);
                static bool getApDetails(GDBusConnection *dbusConn, const char* apPath, apProperties& apDetails);
                static bool getConnectionPaths(GDBusConnection *dbusConn, std::list<std::string>& pathsList);
                static bool getDeviceState(GDBusConnection *dbusConn, const gchar *iface_name, NMDeviceState& state);
                static bool getDeviceStateReason(GDBusConnection *dbusConn, const gchar *iface_name, NMDeviceStateReason& StateReason);
        };

    } // Plugin
} // WPEFramework

#define G_VARIANT_LOOKUP(dict, key, format, ...)            \
    if (!g_variant_lookup(dict, key, format, __VA_ARGS__)) {\
        NMLOG_ERROR("g_variant Key '%s' not found", key);               \
        return FALSE;                                       \
    }
