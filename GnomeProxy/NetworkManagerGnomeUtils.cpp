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

#include <glib.h>
#include <gio/gio.h>
#include <string>
#include <nm-dbus-interface.h>

#include "NetworkManagerLogger.h"
#include "NetworkManagerGnomeUtils.h"
#include "NetworkManagerDbusMgr.h"

namespace WPEFramework
{
    namespace Plugin
    {
        static const char* ifnameEth = "eth0";
        static const char* ifnameWlan = "wlp0s20f3";

        std::string GnomeUtils::getCommaSeparatedSSIDs(const std::list<std::string>& ssids) {
            std::string result;
            for (auto it = ssids.begin(); it != ssids.end(); ++it) {
                result += *it;
                if (std::next(it) != ssids.end()) {
                    result += ", ";
                }
            }
            return result;
        }

        void GnomeUtils::printKeyVariant(const char *setting_name, GVariant *setting)
        {
            GVariantIter iter;
            const char  *property_name;
            GVariant    *value;
            char        *printed_value;

            NMLOG_DEBUG("  %s:", setting_name);
            g_variant_iter_init(&iter, setting);
            while (g_variant_iter_next(&iter, "{&sv}", &property_name, &value)) {
                printed_value = g_variant_print(value, FALSE);
                if (strcmp(printed_value, "[]") != 0)
                    NMLOG_DEBUG("    %s: %s", property_name, printed_value);
                g_free(printed_value);
                g_variant_unref(value);
            }
        }

        // TODO change this function 
        const char* GnomeUtils::getWifiIfname() { return ifnameWlan; }
        const char* GnomeUtils::getEthIfname() { return ifnameEth; }

        bool GnomeUtils::getCachedPropertyU(GDBusProxy* proxy, const char* propertiy, uint32_t *value)
        {
            GVariant* result = NULL;
            result = g_dbus_proxy_get_cached_property(proxy, propertiy);
            if (!result) {
                NMLOG_ERROR("Failed to get '%s' properties", propertiy);
                g_object_unref(proxy);
                return false;
            }

            if (g_variant_is_of_type (result, G_VARIANT_TYPE_UINT32)) {
                *value = g_variant_get_uint32(result);
                NMLOG_DEBUG("%s: %d", propertiy, *value);
            }
            else
                NMLOG_WARNING("Unexpected type returned property: %s", g_variant_get_type_string(result));
            g_variant_unref(result);
            return true;
        }

        bool GnomeUtils::getDevicePropertiesByPath(GDBusConnection *dbusConn, const char* devicePath, deviceProperties& properties)
        {
            GError *error = NULL;
            GVariant *devicesVar = NULL;
            GDBusProxy* nmProxy = NULL;
            u_int32_t value;

            nmProxy = g_dbus_proxy_new_sync(dbusConn,
                                G_DBUS_PROXY_FLAGS_NONE,
                                                NULL,  // GDBusInterfaceInfo
                                                "org.freedesktop.NetworkManager",  // Bus name
                                                devicePath,  // Object path
                                                "org.freedesktop.NetworkManager.Device",  // Interface name
                                                NULL,  // GCancellable
                                                &error);


            if (error) {
                NMLOG_ERROR("Error calling DBus.Properties method: %s", error->message);
                g_error_free(error);
                return false;
            }


            if(GnomeUtils::getCachedPropertyU(nmProxy, "DeviceType", &value))
            {
               properties.deviceType = static_cast<NMDeviceType>(value);
            }
            else
                 NMLOG_ERROR("'DeviceType' property failed");

            devicesVar = g_dbus_proxy_get_cached_property(nmProxy, "Interface");
            if (devicesVar) {
                const gchar *iface = g_variant_get_string(devicesVar, NULL);
                if(iface != NULL)
                    properties.interface = iface;
                NMLOG_DEBUG("Interface: %s", iface);
                g_variant_unref(devicesVar);
            }
            else
                NMLOG_ERROR("'Interface' property failed");

            guint32 devState, stateReason;
            devicesVar = g_dbus_proxy_get_cached_property(nmProxy, "StateReason");
            if (devicesVar) {
                g_variant_get(devicesVar, "(uu)", &devState, &stateReason);
                properties.state = static_cast<NMDeviceState>(devState);
                properties.stateReason = static_cast<NMDeviceStateReason>(stateReason);
                g_variant_unref(devicesVar);
            }
            else
                NMLOG_ERROR("'StateReason' property failed");
            
            g_object_unref(nmProxy);
            return true;
        }

        bool GnomeUtils::getDevicePropertiesByIfname(GDBusConnection *dbusConn, const char* ifaceName, deviceProperties& properties)
        {
             GError *error = NULL;
            GVariant *devicesVar = NULL;
            bool ret = false;

            GDBusProxy* nmProxy  = g_dbus_proxy_new_sync(dbusConn,
                            G_DBUS_PROXY_FLAGS_NONE,
                            NULL,
                            "org.freedesktop.NetworkManager",
                            "/org/freedesktop/NetworkManager",
                            "org.freedesktop.NetworkManager",
                            NULL,
                            &error);

            devicesVar = g_dbus_proxy_call_sync(nmProxy, "GetDevices", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
            if (error) {
                NMLOG_ERROR("Error calling GetDevices method: %s", error->message);
                g_error_free(error);
                g_object_unref(nmProxy);
                return false;
            }

            GVariantIter* iter;
            const gchar* devicePath;
            g_variant_get(devicesVar, "(ao)", &iter);
            while (g_variant_iter_loop(iter, "o", &devicePath))
            {
                if(devicePath != NULL )
                {
                    if(getDevicePropertiesByPath(dbusConn, devicePath, properties) && properties.interface == ifaceName)
                    {
                        properties.path = devicePath;
                        ret = true;
                        break;
                    }
                }
            }
            g_variant_iter_free(iter);

            if(!ret)
                NMLOG_ERROR("'%s' interface not found", ifaceName);

            if(devicesVar)
                g_variant_unref(devicesVar);
            if(nmProxy)
                g_object_unref(nmProxy);
            return ret;
        }

        bool GnomeUtils::getDeviceByIpIface(GDBusConnection *dbusConn, const gchar *iface_name, std::string& path)
        {
            // TODO Fix Error calling method: 
            // GDBus.Error:org.freedesktop.NetworkManager.UnknownDevice: No device found for the requested iface 
            // in wsl linux
            GError *error = NULL;
            GVariant *result;
            gchar *device_path = NULL;
            bool ret = false;

            result = g_dbus_connection_call_sync(
                dbusConn,
                "org.freedesktop.NetworkManager",               // D-Bus name
                "/org/freedesktop/NetworkManager",              // Object path
                "org.freedesktop.NetworkManager",               // Interface
                "GetDeviceByIpIface",                           // Method name
                g_variant_new("(s)", iface_name),               // Input parameter (the interface name)
                G_VARIANT_TYPE("(o)"),                          // Expected return type (object path)
                G_DBUS_CALL_FLAGS_NONE,
                -1,                                             // Default timeout
                NULL,
                &error
            );

            if (error != NULL) {
                NMLOG_ERROR("calling GetDeviceByIpIface: %s", error->message);
                g_error_free(error);
                return ret;
            }

            if (g_variant_is_of_type (result, (const GVariantType *) "(o)"))
            {
                g_variant_get(result, "(o)", &device_path);
                if(g_strdup(device_path) != NULL)
                {
                    path = std::string(g_strdup(device_path));
                    ret = true;
                }
            }
            //NMLOG_DEBUG("%s device path %s", iface_name, path.c_str());
            g_variant_unref(result);
            return ret;
        }

        bool GnomeUtils::getApDetails(GDBusConnection *connection, const char* apPath, apProperties& apDetails)
        {
            char *bssid = NULL;
            uint8_t strength = 0;
            GError* error = NULL;
            GVariant* result = NULL;

            GDBusProxy* proxy  = g_dbus_proxy_new_sync(connection,
                                G_DBUS_PROXY_FLAGS_NONE,
                                NULL,
                                "org.freedesktop.NetworkManager",
                                apPath,
                                "org.freedesktop.NetworkManager.AccessPoint",
                                NULL,
                                &error);

            if (error) {
                NMLOG_ERROR("creating proxy: %s", error->message);
                g_error_free(error);
                return false;
            }

            gsize ssid_length = 0;
            result = g_dbus_proxy_get_cached_property(proxy,"Ssid");
            if (!result) {
                NMLOG_ERROR("Failed to get AP properties.");
                g_object_unref(proxy);
                return false;
            }
            const guchar *ssid_data = static_cast<const guchar*>(g_variant_get_fixed_array(result, &ssid_length, sizeof(guchar)));
            if (ssid_data && ssid_length > 0 && ssid_length <= 32) {
                apDetails.ssid.assign(reinterpret_cast<const char*>(ssid_data), ssid_length);
                NMLOG_DEBUG("SSID: %s", apDetails.ssid.c_str());
            } else {
                NMLOG_ERROR("Invalid SSID length: %zu (maximum is 32)", ssid_length);
                apDetails.ssid="---";
            }
            g_variant_unref(result);

            result = g_dbus_proxy_get_cached_property(proxy,"HwAddress");
            if (!result) {
                NMLOG_ERROR("Failed to get AP properties.");
                g_object_unref(proxy);
                return false;
            }
            g_variant_get(result, "s", &bssid);
            apDetails.bssid.assign(bssid);
            NMLOG_DEBUG("bssid %s", apDetails.bssid.c_str());
            g_variant_unref(result);

            result = g_dbus_proxy_get_cached_property(proxy,"Strength");
            if (!result) {
                NMLOG_ERROR("Failed to get AP properties.");
                g_object_unref(proxy);
                return false;
            }
            g_variant_get(result, "y", &strength);
            NMLOG_DEBUG("strength %d", strength);
            g_variant_unref(result);

            GnomeUtils::getCachedPropertyU(proxy, "Flags", &apDetails.flags);
            GnomeUtils::getCachedPropertyU(proxy, "WpaFlags", &apDetails.wpaFlags);
            GnomeUtils::getCachedPropertyU(proxy, "RsnFlags", &apDetails.rsnFlags);
            GnomeUtils::getCachedPropertyU(proxy, "Mode", &apDetails.mode);
            GnomeUtils::getCachedPropertyU(proxy, "Frequency", &apDetails.frequency);

            g_object_unref(proxy);

            return true;
        }

        bool GnomeUtils::getConnectionPaths(GDBusConnection *dbusConn, std::list<std::string>& pathsList)
        {
            GDBusProxy *sProxy= NULL;
            GError *error= NULL;
            GVariant *listProxy = NULL;
            char **paths = NULL;

            sProxy = g_dbus_proxy_new_sync(dbusConn,
                                    G_DBUS_PROXY_FLAGS_NONE,
                                    NULL,
                                    "org.freedesktop.NetworkManager",
                                    "/org/freedesktop/NetworkManager/Settings",
                                    "org.freedesktop.NetworkManager.Settings",
                                    NULL, // GCancellable
                                    &error);

            if(sProxy == NULL)
            {
                if (error != NULL) {
                    NMLOG_ERROR("Failed to create proxy: %s", error->message);
                    g_error_free(error);
                }
                return false;
            }

            listProxy = g_dbus_proxy_call_sync(sProxy,
                                        "ListConnections",
                                        NULL,
                                        G_DBUS_CALL_FLAGS_NONE,
                                        -1,
                                        NULL,
                                        &error);
            if(listProxy == NULL)
            {
                if (!error) {
                    NMLOG_ERROR("ListConnections failed: %s", error->message);
                    g_error_free(error);
                    g_object_unref(sProxy);
                    return false;
                }
                else
                    NMLOG_ERROR("ListConnections proxy failed no error message");
            }

            g_variant_get(listProxy, "(^ao)", &paths);
            g_variant_unref(listProxy);

            if(paths == NULL)
            {
                NMLOG_WARNING("no connection path available");
                g_object_unref(sProxy);
                return false;
            }

            for (int i = 0; paths[i]; i++) {
                //NMLOG_DEBUG("Connection path: %s", paths[i]);
                pathsList.push_back(paths[i]);
            }

            g_object_unref(sProxy);
            if(pathsList.empty())
                return false;

            return true;
        }

        bool GnomeUtils::getDeviceState(GDBusConnection *dbusConn, const gchar *iface_name, NMDeviceState& state)
        {
            deviceProperties devProp;
            if(!GnomeUtils::getDevicePropertiesByIfname(dbusConn, iface_name, devProp))
            {
                NMLOG_ERROR("Interface unknow to NetworkManger Deamon");
                return false;
            }

            state = devProp.state;
            return true;
        }

        bool GnomeUtils::getDeviceStateReason(GDBusConnection *dbusConn, const gchar *iface_name, NMDeviceStateReason& StateReason)
        {
            deviceProperties devProp;
            if(!GnomeUtils::getDevicePropertiesByIfname(dbusConn, iface_name, devProp))
            {
                NMLOG_ERROR("Interface unknow to NetworkManger Deamon");
                return false;
            }

            StateReason = devProp.stateReason;
            NMLOG_WARNING("Device state: %u (reason: %u)", devProp.state, devProp.stateReason);
            return true;
        }

    } // Plugin
} // WPEFramework

