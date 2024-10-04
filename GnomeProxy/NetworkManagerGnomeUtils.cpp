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
        static const char* ifnameWlan = "wlan0";

        void GnomeUtils::printKeyVariant(const char *setting_name, GVariant *setting)
        {
            GVariantIter iter;
            const char  *property_name;
            GVariant    *value;
            char        *printed_value;

            NMLOG_VERBOSE("  %s:", setting_name);
            g_variant_iter_init(&iter, setting);
            while (g_variant_iter_next(&iter, "{&sv}", &property_name, &value)) {
                printed_value = g_variant_print(value, FALSE);
                if (strcmp(printed_value, "[]") != 0)
                    NMLOG_VERBOSE("    %s: %s", property_name, printed_value);
                g_free(printed_value);
                g_variant_unref(value);
            }
        }

        bool GnomeUtils::getDeviceByIpIface(GDBusConnection *connection, const gchar *iface_name, std::string& path)
        {
            // TODO Fix Error calling method: 
            // GDBus.Error:org.freedesktop.NetworkManager.UnknownDevice: No device found for the requested iface 
            // in wsl linux
            GError *error = NULL;
            GVariant *result;
            gchar *device_path = NULL;
            bool ret = false;

            result = g_dbus_connection_call_sync(
                connection,
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

            g_variant_get(result, "(o)", &device_path);
            if(g_strdup(device_path) != NULL)
            {
                path = std::string(g_strdup(device_path));
                ret = true;
            }

            //NMLOG_TRACE("%s device path %s", iface_name, path.c_str());
            g_variant_unref(result);
            return ret;
        }

        static bool get_cached_property_u(GDBusProxy* proxy, const char* propertiy, uint32_t value)
        {
            GVariant* result = NULL;
            result = g_dbus_proxy_get_cached_property(proxy, propertiy);
            if (!result) {
                NMLOG_ERROR("Failed to get AP properties");
                g_object_unref(proxy);
                return false;
            }
            g_variant_get(result, "u", &value);
            NMLOG_TRACE("%s: %d", propertiy, value);
            g_variant_unref(result);
            return true;
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
                NMLOG_TRACE("SSID: %s", apDetails.ssid.c_str());
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
            NMLOG_TRACE("bssid %s", apDetails.bssid.c_str());
            g_variant_unref(result);

            result = g_dbus_proxy_get_cached_property(proxy,"Strength");
            if (!result) {
                NMLOG_ERROR("Failed to get AP properties.");
                g_object_unref(proxy);
                return false;
            }
            g_variant_get(result, "y", &strength);
            NMLOG_TRACE("strength %d", strength);
            g_variant_unref(result);

            get_cached_property_u(proxy, "Flags", apDetails.flags);
            get_cached_property_u(proxy, "WpaFlags", apDetails.wpaFlags);
            get_cached_property_u(proxy, "RsnFlags", apDetails.rsnFlags);
            get_cached_property_u(proxy, "Mode", apDetails.mode);
            get_cached_property_u(proxy, "Frequency", apDetails.frequency);

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
                NMLOG_VERBOSE("Connection path: %s", paths[i]);
                pathsList.push_back(paths[i]);
            }

            g_object_unref(sProxy);
            if(pathsList.empty())
                return false;

            return true;
        }

        bool GnomeUtils::getDeviceState(GDBusConnection *dbusConn, const gchar *iface_name, NMDeviceState& state)
        {
            GError *error = NULL;
            GVariant *result;
            std::string ifaceDevicePath;

            if(!GnomeUtils::getDeviceByIpIface(dbusConn, iface_name, ifaceDevicePath) && !ifaceDevicePath.empty())
            {
                NMLOG_ERROR("Interface unknow to NetworkManger Deamon");
                return false;
            }

            result = g_dbus_connection_call_sync(
                                    dbusConn,
                                    "org.freedesktop.NetworkManager",               // D-Bus name
                                    ifaceDevicePath.c_str(),                        // Object path
                                    "org.freedesktop.DBus.Properties",              // Interface for property access
                                    "Get",                                          // Method name for getting a property
                                    g_variant_new("(ss)",                           // Input parameters (interface, property name)
                                                "org.freedesktop.NetworkManager.Device",            // Interface name
                                                "State"),               // Property name
                                    G_VARIANT_TYPE("(v)"),                          // Expected return type (variant)
                                    G_DBUS_CALL_FLAGS_NONE,
                                    -1,                                             // Default timeout
                                    NULL,
                                    &error
                                );
            if (error != NULL) {
                NMLOG_ERROR("failed to DBus.Properties: %s", error->message);
                g_error_free(error);
            } else {
                GVariant *variantValue;
                g_variant_get(result, "(v)", &variantValue);

                state = static_cast<NMDeviceState>(g_variant_get_uint32(variantValue));
                NMLOG_TRACE("Device state: %u", state);

                g_variant_unref(variantValue);
                g_variant_unref(result);
                return true;
            }

            return false;
        }

        bool GnomeUtils::getDeviceStateReason(GDBusConnection *dbusConn, const gchar *iface_name, NMDeviceStateReason& StateReason)
        {
            GError *error = NULL;
            GVariant *resultReason;
            std::string ifaceDevicePath;

            if(!GnomeUtils::getDeviceByIpIface(dbusConn, iface_name, ifaceDevicePath) && !ifaceDevicePath.empty())
            {
                NMLOG_ERROR("Interface unknow to NetworkManger Deamon");
                return false;
            }

            resultReason = g_dbus_connection_call_sync( dbusConn,
                                    "org.freedesktop.NetworkManager",
                                    ifaceDevicePath.c_str(),
                                    "org.freedesktop.DBus.Properties",
                                    "Get",
                                    g_variant_new("(ss)", 
                                                "org.freedesktop.NetworkManager.Device",
                                                "StateReason"),
                                    G_VARIANT_TYPE("(v)"),
                                    G_DBUS_CALL_FLAGS_NONE,
                                    -1,
                                    NULL,
                                    &error
                                );

            if (error != NULL) {
                NMLOG_ERROR("getting StateReason property: %s", error->message);
                g_error_free(error);
            } else if (resultReason != NULL) {
                GVariant *variantReason;
                g_variant_get(resultReason, "(v)", &variantReason);

                guint32 state_reason_code, state_reason_detail;
                g_variant_get(variantReason, "(uu)", &state_reason_code, &state_reason_detail);
                NMLOG_WARNING("Device state reason: %u (detail: %u)", state_reason_code, state_reason_detail);
                g_variant_unref(variantReason);
                g_variant_unref(resultReason);
                return true;
            }
            return false;
        }

    } // Plugin
} // WPEFramework

