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
#include <list>
#include <uuid/uuid.h>
#include <NetworkManager.h>
#include <libnm/NetworkManager.h>
#include "NetworkManagerLogger.h"
#include "NetworkManagerGnomeClient.h"
#include "NetworkManagerGnomeUtils.h"

namespace WPEFramework
{
    namespace Plugin
    {

        NetworkManagerClient::NetworkManagerClient() {
            NMLOG_INFO("NetworkManagerClient created");
            if (!dbusConnection.InitializeBusConnection("org.freedesktop.NetworkManager")) {
                NMLOG_ERROR("Failed to initialize D-Bus connection for NetworkManager");
            }
        }

        NetworkManagerClient::~NetworkManagerClient() {
            NMLOG_INFO("NetworkManagerClient destroyed");
        }

        static void fetssidandbssid(GVariant *setting80211, std::string &ssid, std::string &bssid)
        {
            GVariantIter iter;
            const char  *property_name;
            GVariant *value;
            char *printed_value;

            g_variant_iter_init(&iter, setting80211);
            while (g_variant_iter_next(&iter, "{&sv}", &property_name, &value)) {

                printed_value = g_variant_print(value, FALSE);
                if (strcmp(property_name, "seen-bssids") == 0) {
                    bssid.clear();
                    GVariantIter bssid_iter;
                    g_variant_iter_init(&bssid_iter, value);
                    gchar *bssid_elem;

                    if (g_variant_iter_next(&bssid_iter, "s", &bssid_elem)) {
                        bssid = bssid_elem;
                       // NMLOG_VERBOSE("BSSID: %s", bssid.c_str());
                    }

                    //g_variant_iter_free(&bssid_iter);
                }
                else if (strcmp(property_name, "ssid") == 0) {
                    // Decode SSID from GVariant of type "ay"
                    gsize ssid_length = 0;
                    const guchar *ssid_data = static_cast<const guchar*>(g_variant_get_fixed_array(value, &ssid_length, sizeof(guchar)));
                    if (ssid_data && ssid_length > 0 && ssid_length <= 32) {
                        ssid.assign(reinterpret_cast<const char*>(ssid_data), ssid_length);
                        //NMLOG_DEBUG("SSID: %s", ssid.c_str());
                    } else {
                        NMLOG_ERROR("Invalid SSID length: %zu (maximum is 32)", ssid_length);
                        ssid.clear();
                    }
                }
            }

            g_free(printed_value);
            g_variant_unref(value);
        }

        static bool fetchSSIDFromConnection(GDBusConnection *dbusConn, const std::string& path, std::string& ssid)
        {
            GError *error = NULL;
            GDBusProxy *ConnProxy = NULL;
            GVariant *settingsProxy= NULL, *connection= NULL, *sCon= NULL;
            bool isFound;
            const char *connId= NULL, *connTyp= NULL, *iface= NULL;

            ConnProxy = g_dbus_proxy_new_sync(dbusConn,
                                G_DBUS_PROXY_FLAGS_NONE,
                                NULL,
                                "org.freedesktop.NetworkManager",
                                path.c_str(),
                                "org.freedesktop.NetworkManager.Settings.Connection",
                                NULL,
                                &error);

            if (error != NULL) {
                NMLOG_ERROR("Failed to create proxy: %s", error->message);
                g_error_free(error);
                return false;
            }

            settingsProxy = g_dbus_proxy_call_sync(ConnProxy,
                                    "GetSettings",
                                    NULL,
                                    G_DBUS_CALL_FLAGS_NONE,
                                    -1,
                                    NULL,
                                    &error);
            if (!settingsProxy) {
                g_dbus_error_strip_remote_error(error);
                NMLOG_WARNING("Failed to get connection settings: %s", error->message);
                g_error_free(error);
            }

            g_variant_get(settingsProxy, "(@a{sa{sv}})", &connection);
            sCon = g_variant_lookup_value(connection, "connection", NULL);
            if(sCon == NULL) {
                NMLOG_ERROR("connection Gvarient Error");
                g_variant_unref(settingsProxy);
                return false;
            }
            g_assert(sCon != NULL);
            G_VARIANT_LOOKUP(sCon, "type", "&s", &connTyp);
            //NMLOG_VERBOSE("type= %s, iface= %s", connTyp, iface);
            if(strcmp(connTyp,"802-11-wireless") == 0)
            {
                G_VARIANT_LOOKUP(sCon, "interface-name", "&s", &iface);
                if(strcmp(iface,"wlan0") != 0)
                {
                    NMLOG_ERROR("interafce-name connection not wlan0");
                    if (sCon)
                        g_variant_unref(sCon);
                }

                GVariant *setting = g_variant_lookup_value(connection, "802-11-wireless", NULL);
                if (setting)
                {
                    std::string bssid;
                    fetssidandbssid(setting, ssid, bssid);
                    g_variant_unref(setting);
                }
            }

            if (sCon)
                g_variant_unref(sCon);
            if (connection)
                g_variant_unref(connection);
            if (settingsProxy)
                g_variant_unref(settingsProxy);
            g_object_unref(ConnProxy);

            return true;
        }

        static bool deleteConnection(GDBusConnection *dbusConn, const std::string& path, std::string& ssid)
        {
            GError *error = NULL;
            GDBusProxy *ConnProxy = NULL;
            GVariant *deleteVar= NULL;

            ConnProxy = g_dbus_proxy_new_sync(dbusConn,
                                G_DBUS_PROXY_FLAGS_NONE,
                                NULL,
                                "org.freedesktop.NetworkManager",
                                path.c_str(),
                                "org.freedesktop.NetworkManager.Settings.Connection",
                                NULL,
                                &error);

            if (error != NULL) {
                NMLOG_ERROR("Failed to create proxy: %s", error->message);
                g_error_free(error);
                return false;
            }

            deleteVar = g_dbus_proxy_call_sync(ConnProxy,
                                    "Delete",
                                    NULL,
                                    G_DBUS_CALL_FLAGS_NONE,
                                    -1,
                                    NULL,
                                    &error);
            if (!deleteVar) {
                g_dbus_error_strip_remote_error(error);
                NMLOG_WARNING("Failed to get connection settings: %s", error->message);
                g_error_free(error);
                g_object_unref(ConnProxy);
            }
            else
            {
               NMLOG_INFO("connection '%s' Removed path: %s", ssid.c_str(), path.c_str()); 
            }

            if (deleteVar)
                g_variant_unref(deleteVar);
            g_object_unref(ConnProxy);

            return true;
        }

        bool NetworkManagerClient::getKnownSSIDs(std::list<std::string>& ssids)
        {
            std::list<std::string> paths;
            if(!GnomeUtils::getConnectionPaths(dbusConnection.getConnection(), paths))
            {
                NMLOG_ERROR("Connection path fetch failed");
                return false;
            }

            for (const std::string& path : paths) {
               // NMLOG_VERBOSE("connection path %s", path.c_str());
                std::string ssid;
                fetchSSIDFromConnection(dbusConnection.getConnection(), path, ssid);
                if(!ssid.empty())
                    ssids.push_back(ssid);
            }

            if(ssids.empty())
                return false;
            return true;
        }

        bool NetworkManagerClient::getConnectedSSID(const Exchange::INetworkManager::WiFiSSIDInfo &ssidinfo)
        {
            std::string wifiDevicePath;
            GError* error = NULL;
            GDBusProxy* wProxy = NULL;
            GVariant* result = NULL;
            gchar *activeApPath = NULL;
            bool ret = false;
            NMDeviceState state;

            if(!GnomeUtils::getDeviceByIpIface(dbusConnection.getConnection(),"wlan0", wifiDevicePath))
            {
                NMLOG_ERROR("no wifi device found");
                return false;
            }

            if(GnomeUtils::getDeviceState(dbusConnection.getConnection(), "wlan0", state) && state < NM_DEVICE_STATE_DISCONNECTED)
            {
                NMDeviceStateReason StateReason;
                GnomeUtils::getDeviceStateReason(dbusConnection.getConnection(), "wlan0", StateReason);
                NMLOG_ERROR("wifi device state is invallied");
                return false;
            }

            wProxy = g_dbus_proxy_new_sync(dbusConnection.getConnection(),
                                    G_DBUS_PROXY_FLAGS_NONE,
                                    NULL,
                                    "org.freedesktop.NetworkManager",
                                    wifiDevicePath.c_str(),
                                    "org.freedesktop.NetworkManager.Device.Wireless",
                                    NULL, // GCancellable
                                    &error);

            if (error) {
                NMLOG_ERROR("Error creating proxy: %s", error->message);
                g_error_free(error);
                return false;
            }

            result = g_dbus_proxy_get_cached_property(wProxy, "ActiveAccessPoint");
            if (!result) {
                NMLOG_ERROR("Failed to get ActiveAccessPoint property.");
                g_object_unref(wProxy);
                return false;
            }

            g_variant_get(result, "o", &activeApPath);
            if(g_strdup(activeApPath) != NULL && g_strcmp0(activeApPath, "/") != 0)
            {
                NMLOG_TRACE("ActiveAccessPoint property path %s", activeApPath);
                apProperties apDetails;
                if(GnomeUtils::getApDetails(dbusConnection.getConnection(), g_strdup(activeApPath), apDetails))
                {
                    NMLOG_INFO("getApDetails success");
                    ret = true;
                }
            }
            else
                NMLOG_ERROR("active access point not found");

            g_variant_unref(result);
            g_object_unref(wProxy);
            return ret;
        }

        bool NetworkManagerClient::getAvailableSSIDs(std::list<std::string>& ssids)
        {
            GError* error = nullptr;
            GDBusProxy* wProxy = nullptr;
            std::string wifiDevicePath;
            NMDeviceState state;

            if(GnomeUtils::getDeviceState(dbusConnection.getConnection(), "wlan0", state) && state < NM_DEVICE_STATE_DISCONNECTED)
            {
                NMDeviceStateReason StateReason;
                GnomeUtils::getDeviceStateReason(dbusConnection.getConnection(), "wlan0", StateReason);
                NMLOG_ERROR("wifi device state is invallied");
                return false;
            }

            if(!GnomeUtils::getDeviceByIpIface(dbusConnection.getConnection(),"wlan0", wifiDevicePath))
            {
                NMLOG_ERROR("no wifi device found");
                return false;
            }

            wProxy = g_dbus_proxy_new_sync(dbusConnection.getConnection(),
                                    G_DBUS_PROXY_FLAGS_NONE,
                                    NULL,
                                    "org.freedesktop.NetworkManager",
                                    wifiDevicePath.c_str(),
                                    "org.freedesktop.NetworkManager.Device.Wireless",
                                    NULL,
                                    &error);

            if (error) {
                NMLOG_ERROR("Error creating proxy: %s", error->message);
                g_error_free(error);
                return false;
            }

            GVariant* result = g_dbus_proxy_call_sync(wProxy, "GetAccessPoints", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
            // TODO change to GetAllAccessPoints to include hidden ssid 
            if (error) {
                NMLOG_ERROR("Error creating proxy: %s", error->message);
                g_error_free(error);
                g_object_unref(wProxy);
                return false;
            }

            GVariantIter* iter;
            const gchar* apPath;
            g_variant_get(result, "(ao)", &iter);

            while (g_variant_iter_loop(iter, "o", &apPath)) {
                apProperties apDetails;
                NMLOG_VERBOSE("Access Point Path: %s", apPath);
                if(!GnomeUtils::getApDetails(dbusConnection.getConnection(), apPath, apDetails))
                {
                    NMLOG_WARNING("getApDetails failed");
                }
            }

            g_variant_iter_free(iter);
            g_variant_unref(result);
            g_object_unref(wProxy);

            return true;
        }

        bool NetworkManagerClient::startWifiScanning(const std::string ssid)
        {
            GError* error = NULL;
            GDBusProxy* wProxy = NULL;
            std::string wifiDevicePath;
            if(!GnomeUtils::getDeviceByIpIface(dbusConnection.getConnection(),"wlan0", wifiDevicePath))
            {
                NMLOG_ERROR("no wifi device found");
                return false;
            }

            wProxy = g_dbus_proxy_new_sync(dbusConnection.getConnection(),
                                G_DBUS_PROXY_FLAGS_NONE,
                                NULL,
                                "org.freedesktop.NetworkManager",
                                wifiDevicePath.c_str(),
                                "org.freedesktop.NetworkManager.Device.Wireless",
                                NULL,
                                &error);

            if (error) {
                NMLOG_ERROR("Error creating proxy: %s", error->message);
                g_error_free(error);
                return false;
            }

            GVariant* timestampVariant = NULL;
            timestampVariant = g_dbus_proxy_get_cached_property(wProxy, "LastScan");
            if (!timestampVariant) {
                NMLOG_ERROR("Failed to get LastScan properties");
                g_object_unref(wProxy);
                return false;
            }

            if (g_variant_is_of_type (timestampVariant, G_VARIANT_TYPE_INT64)) {
                gint64 timestamp;
                timestamp = g_variant_get_int64 (timestampVariant);
                NMLOG_TRACE("Last scan timestamp: %lld", timestamp);
            } else {
                g_warning("Unexpected variant type: %s", g_variant_get_type_string (timestampVariant));
            }

            GVariant *options = NULL;
            if (!ssid.empty()) 
            {
                // TODO fix specific ssid scanning
                NMLOG_INFO("staring wifi scanning .. %s", ssid.c_str());
                GVariantBuilder settingsBuilder;
                g_variant_builder_init (&settingsBuilder, G_VARIANT_TYPE ("a{sv}"));
                GVariant *ssid_array = g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE, (const guint8 *)ssid.c_str(), ssid.length(), 1);
                g_variant_builder_add (&settingsBuilder, "{sv}", "ssids", ssid_array);

                // GVariantBuilder builder, array_builder;
                // g_variant_builder_init(&builder, G_VARIANT_TYPE_VARDICT);
                // g_variant_builder_init(&array_builder, G_VARIANT_TYPE("aay"));
                // g_variant_builder_add(&array_builder, "@ay",
                //                     g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE, (const guint8 *) ssid.c_str(), ssid.length(), 1)
                //                     );
                // g_variant_builder_add(&builder, "{sv}", "ssids", g_variant_builder_end(&array_builder));
                // g_variant_builder_add(&builder, "{sv}", "hidden", g_variant_new_boolean(TRUE));

                options = g_variant_builder_end(&settingsBuilder);
                g_dbus_proxy_call_sync(wProxy, "RequestScan", options, G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
            }
            else
                g_dbus_proxy_call_sync(wProxy, "RequestScan",   g_variant_new("(a{sv})", options), G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
            if (error) {
                NMLOG_ERROR("Error RequestScan: %s", error->message);
                g_error_free(error);
                g_object_unref(wProxy);
                return false;
            }

            if(options)
                g_variant_unref(options);
            g_object_unref(wProxy);

            return true;
        }

        static char * nm_utils_uuid_generate (void)
        {
            uuid_t uuid;
            char *buf;

            buf = static_cast<char*>(g_malloc0(37));
            uuid_generate_random (uuid);
            uuid_unparse_lower (uuid, &buf[0]);
            return buf;
        }

        bool addNewConnctionAndactivate(GDBusConnection *dbusConn, GVariantBuilder connBuilder, const char* devicePath, bool persist)
        {
            GDBusProxy* proxy = nullptr;
            GError* error = nullptr;
            GVariant* result = nullptr;

            // Define the D-Bus service, object path, and interface
            const char* service = "org.freedesktop.NetworkManager";
            const char* objectPath = "/org/freedesktop/NetworkManager";
            const char* interface = "org.freedesktop.NetworkManager";
        
            proxy = g_dbus_proxy_new_sync(dbusConn, G_DBUS_PROXY_FLAGS_NONE, NULL, service, objectPath, interface, NULL, &error);

            if (error) {
                std::cerr << "Failed to create proxy: " << error->message << std::endl;
                g_clear_error(&error);
                return false;
            }

            const char* specific_object = "/";

            GVariantBuilder optionsBuilder;
            g_variant_builder_init (&optionsBuilder, G_VARIANT_TYPE ("a{sv}"));
            if(!persist)
            {
                NMLOG_ERROR("wifi connection will not persist to the disk");
                g_variant_builder_add(&optionsBuilder, "{sv}", "persist", g_variant_new_string("volatile"));
            }
            //else
                //g_variant_builder_add(&optionsBuilder, "{sv}", "persist", g_variant_new_string("disk"));

            result = g_dbus_proxy_call_sync (proxy, "AddAndActivateConnection2",
                g_variant_new("(a{sa{sv}}ooa{sv})", connBuilder, devicePath, specific_object, optionsBuilder),
                G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error);

            if (error) {
                NMLOG_ERROR("Failed to call AddAndActivateConnection2: %s", error->message);
                g_clear_error(&error);
                g_object_unref(proxy);
                return false;
            }

            GVariant* pathVariant;
            GVariant* activeConnVariant;
            GVariant* resultDict;

            g_variant_get(result, "(@o@o@a{sv})", &pathVariant, &activeConnVariant, &resultDict);

            // Extract connection and active connection paths
            const char* newConnPath = g_variant_get_string(pathVariant, nullptr);
            const char* activeConnPath = g_variant_get_string(activeConnVariant, nullptr);

            NMLOG_TRACE("newconn %s; activeconn %s",newConnPath, activeConnPath);
            g_variant_unref(pathVariant);
            g_variant_unref(activeConnVariant);
            g_variant_unref(resultDict);
            g_variant_unref(result);
            g_object_unref(proxy);
        
            return true;
        }

        static bool gVariantConnectionBuilder(const Exchange::INetworkManager::WiFiConnectTo& ssidinfo, GVariantBuilder& connBuilder)
        {
            char *uuid = NULL;
            GVariantBuilder settingsBuilder;

            if(ssidinfo.m_ssid.empty() || ssidinfo.m_ssid.length() > 32)
            {
                NMLOG_WARNING("ssid name is missing or invalied");
                return false;
            }

            switch(ssidinfo.m_securityMode)
            {
                case Exchange::INetworkManager::WIFISecurityMode::WIFI_SECURITY_WPA_PSK_AES:
                case Exchange::INetworkManager::WIFISecurityMode::WIFI_SECURITY_WPA_WPA2_PSK:
                case Exchange::INetworkManager::WIFISecurityMode::WIFI_SECURITY_WPA_PSK_TKIP:
                case Exchange::INetworkManager::WIFISecurityMode::WIFI_SECURITY_WPA2_PSK_AES:
                case Exchange::INetworkManager::WIFISecurityMode::WIFI_SECURITY_WPA2_PSK_TKIP:
                case Exchange::INetworkManager::WIFISecurityMode::WIFI_SECURITY_WPA3_SAE:
                {
                    if(ssidinfo.m_passphrase.empty())
                    {
                        NMLOG_WARNING("wifi securtity type need password");
                        return false;
                    }
                    break;
                }
                case Exchange::INetworkManager::WIFI_SECURITY_NONE:
                    NMLOG_WARNING("open wifi network configuration");
                    break;
                default:
                {
                    NMLOG_WARNING("connection wifi securtity type not supported");
                    return false;
                }
            }

            /* Build up the complete connection */
            g_variant_builder_init (&connBuilder, G_VARIANT_TYPE ("a{sa{sv}}"));

            /* Build up the 'connection' Setting */
            g_variant_builder_init (&settingsBuilder, G_VARIANT_TYPE ("a{sv}"));

            uuid = nm_utils_uuid_generate();
            g_variant_builder_add (&settingsBuilder, "{sv}",
                                NM_SETTING_CONNECTION_UUID,
                                g_variant_new_string (uuid));
            g_free (uuid);

            g_variant_builder_add (&settingsBuilder, "{sv}",
                                NM_SETTING_CONNECTION_ID,
                                g_variant_new_string (ssidinfo.m_ssid.c_str()));

            g_variant_builder_add (&settingsBuilder, "{sv}",
                                NM_SETTING_CONNECTION_INTERFACE_NAME,
                                g_variant_new_string ("wlan0"));

            g_variant_builder_add (&settingsBuilder, "{sv}",
                                NM_SETTING_CONNECTION_TYPE,
                                g_variant_new_string (NM_SETTING_WIRELESS_SETTING_NAME));

            g_variant_builder_add (&connBuilder, "{sa{sv}}",
                                NM_SETTING_CONNECTION_SETTING_NAME,
                                &settingsBuilder);

            /* Add the '802-11-wireless' Setting */
            g_variant_builder_init (&settingsBuilder, G_VARIANT_TYPE ("a{sv}"));
            GVariant *ssid_array = g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE, (const guint8 *)ssidinfo.m_ssid.c_str(), ssidinfo.m_ssid.length(), 1);
            g_variant_builder_add (&settingsBuilder, "{sv}",
                                NM_SETTING_WIRELESS_SSID,
                                ssid_array);

            g_variant_builder_add (&settingsBuilder, "{sv}",
                                NM_SETTING_WIRELESS_MODE,
                                g_variant_new_string(NM_SETTING_WIRELESS_MODE_INFRA));

            g_variant_builder_add (&settingsBuilder, "{sv}",
                                NM_SETTING_WIRELESS_HIDDEN,
                                g_variant_new_boolean(true)); // set hidden: yes

            g_variant_builder_add (&connBuilder, "{sa{sv}}",
                                NM_SETTING_WIRELESS_SETTING_NAME,
                                &settingsBuilder);

            if(!ssidinfo.m_passphrase.empty())
            {
                /* Build up the '802-11-wireless-security' settings */
                g_variant_builder_init (&settingsBuilder, G_VARIANT_TYPE ("a{sv}"));

                g_variant_builder_add (&settingsBuilder, "{sv}",
                                    NM_SETTING_WIRELESS_SECURITY_KEY_MGMT,
                                    g_variant_new_string("wpa-psk"));

                g_variant_builder_add (&settingsBuilder, "{sv}",
                        NM_SETTING_WIRELESS_SECURITY_PSK,
                        g_variant_new_string(ssidinfo.m_passphrase.c_str()));

                g_variant_builder_add (&connBuilder, "{sa{sv}}",
                        NM_SETTING_WIRELESS_SECURITY_SETTING_NAME,
                        &settingsBuilder);
            }

            /* Build up the 'ipv4' Setting */
            g_variant_builder_init (&settingsBuilder, G_VARIANT_TYPE ("a{sv}"));
            g_variant_builder_add (&settingsBuilder, "{sv}",
                                NM_SETTING_IP_CONFIG_METHOD,
                                g_variant_new_string (NM_SETTING_IP4_CONFIG_METHOD_AUTO));

            g_variant_builder_add (&connBuilder, "{sa{sv}}",
                                NM_SETTING_IP4_CONFIG_SETTING_NAME,
                                &settingsBuilder);

            /* Build up the 'ipv6' Setting */
            g_variant_builder_init (&settingsBuilder, G_VARIANT_TYPE ("a{sv}"));
            g_variant_builder_add (&settingsBuilder, "{sv}",
                                NM_SETTING_IP_CONFIG_METHOD,
                                g_variant_new_string (NM_SETTING_IP6_CONFIG_METHOD_AUTO));

            g_variant_builder_add (&connBuilder, "{sa{sv}}",
                                NM_SETTING_IP6_CONFIG_SETTING_NAME,
                                &settingsBuilder);
            return true;
        }

        bool NetworkManagerClient::addToKnownSSIDs(const Exchange::INetworkManager::WiFiConnectTo& ssidinfo)
        {
            GVariantBuilder connBuilder;
            bool ret = false;
            GVariant *result = NULL;
            GError* error = nullptr;
            const char *newConPath = NULL;

            ret = gVariantConnectionBuilder(ssidinfo, connBuilder);
            if(!ret) {
              NMLOG_WARNING("connection builder failed");
              return false;
            }

            GDBusProxy *proxy = NULL;
            proxy = g_dbus_proxy_new_sync(dbusConnection.getConnection(),
                                G_DBUS_PROXY_FLAGS_NONE,
                                NULL,
                                "org.freedesktop.NetworkManager",
                                "/org/freedesktop/NetworkManager/Settings",
                                "org.freedesktop.NetworkManager.Settings",
                                NULL,
                                &error);
                                
            result = g_dbus_proxy_call_sync (proxy,
                                        "AddConnection",
                                        g_variant_new ("(a{sa{sv}})", &connBuilder),
                                        G_DBUS_CALL_FLAGS_NONE, -1,
                                        NULL, &error);

            if (!proxy) {
                g_dbus_error_strip_remote_error (error);
                NMLOG_ERROR ("Could not create NetworkManager D-Bus proxy: %s", error->message);
                g_error_free (error);
                return false;
            }

            if (result) {
                g_variant_get (result, "(&o)", &newConPath);
                NMLOG_INFO("Added: %s", newConPath);
                g_variant_unref (result);
            } else {
                g_dbus_error_strip_remote_error (error);
                NMLOG_ERROR("Error adding connection: %s", error->message);
                g_clear_error (&error);
            }

            return true;
        }

        bool NetworkManagerClient::removeKnownSSIDs(const std::string& ssid)
        {
            bool ret = false;

            if(ssid.empty())
            {
                NMLOG_ERROR("ssid name is empty");
                return false;
            }

            std::list<std::string> paths;
            if(!GnomeUtils::getConnectionPaths(dbusConnection.getConnection(), paths))
            {
                NMLOG_ERROR("Connection path fetch failed");
                return false;
            }

            for (const std::string& path : paths) {
               // NMLOG_VERBOSE("connection path %s", path.c_str());
                std::string connSsid;
                fetchSSIDFromConnection(dbusConnection.getConnection(), path, connSsid);
                if(connSsid == ssid) {
                    ret = deleteConnection(dbusConnection.getConnection(), path, connSsid);
                }
            }

            if(!ret)
                NMLOG_ERROR("ssid '%s' Connection remove failed", ssid.c_str());

            return ret;
        }

        bool NetworkManagerClient::wifiConnect(const Exchange::INetworkManager::WiFiConnectTo& ssidinfo)
        {
            GVariantBuilder connBuilder;
            bool ret = false;

            //TODO check if wificonnected to same ssid
            //TODO check if same connection is there and 
            //   verify ssid and password is same call read connection path and activate connection 
            //   if not same remove the connection

            ret = gVariantConnectionBuilder(ssidinfo, connBuilder);
            if(!ret) {
                NMLOG_WARNING("connection builder failed");
                return false;
            }

            std::string wifiDevicePath;
            if(!GnomeUtils::getDeviceByIpIface(dbusConnection.getConnection(),"wlan0", wifiDevicePath)) {
                NMLOG_ERROR("no wifi device found");
                return false;
            }

            if(addNewConnctionAndactivate(dbusConnection.getConnection(), connBuilder, wifiDevicePath.c_str(), ssidinfo.m_persistSSIDInfo))
                NMLOG_INFO("wifi connect request success");
            else
                NMLOG_ERROR("wifi connect request Failed");

            return true;
        }

        bool NetworkManagerClient::wifiDisconnect()
        {
            GError* error = NULL;
            GDBusProxy* wProxy = NULL;
            std::string wifiDevicePath;
            NMDeviceState state;
            if(GnomeUtils::getDeviceState(dbusConnection.getConnection(), "wlan0", state) && state <= NM_DEVICE_STATE_DISCONNECTED)
            {
                NMLOG_ERROR("wifi device state is disconneted");
                return true;
            }

            if(!GnomeUtils::getDeviceByIpIface(dbusConnection.getConnection(),"wlan0", wifiDevicePath))
            {
                NMLOG_ERROR("no wifi device found");
                return false;
            }

            wProxy = g_dbus_proxy_new_sync(dbusConnection.getConnection(),
                                G_DBUS_PROXY_FLAGS_NONE,
                                NULL,
                                "org.freedesktop.NetworkManager",
                                wifiDevicePath.c_str(),
                                "org.freedesktop.NetworkManager.Device",
                                NULL,
                                &error);

            if (error) {
                NMLOG_ERROR("Error creating proxy: %s", error->message);
                g_error_free(error);
                return false;
            }

            GVariant* result = g_dbus_proxy_call_sync(wProxy, "Disconnect", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
            if (error) {
                NMLOG_ERROR("Error calling Disconnect method: %s", error->message);
                g_error_free(error);
                g_object_unref(wProxy);
                return false;
            }
            else
               NMLOG_INFO("wifi disconnected success"); 

            return true;
        }

    } // WPEFramework
} // Plugin
