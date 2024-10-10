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

#include "NetworkManagerGdbusMgr.h"
#include "NetworkManagerLogger.h"

namespace WPEFramework
{
    namespace Plugin
    {

        DbusConnectionManager::DbusConnectionManager() : connection(nullptr) {
            NMLOG_INFO("DbusConnectionManager created");
        }

        DbusConnectionManager::~DbusConnectionManager() {
            DeinitializeDbusConnection();
        }

        bool DbusConnectionManager::InitializeBusConnection(const std::string& busName) {
            GError* error = nullptr;
            connection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, &error);

            if (!connection) {
                NMLOG_ERROR("Failed to initialize D-Bus connection: %s ", error->message);
                g_error_free(error);
                return false;
            }

            NMLOG_INFO("D-Bus connection initialized successfully for bus: %s", busName.c_str());
            return true;
        }

        void DbusConnectionManager::DeinitializeDbusConnection() {
            if (connection) {
                g_object_unref(connection);
                connection = nullptr;
                NMLOG_INFO("D-Bus connection deinitialized successfully");
            }
        }

        GDBusConnection* DbusConnectionManager::getConnection() const {
            return connection;
        }

        DbusMgr::DbusMgr() : connection(nullptr)
        {
            GError* error = nullptr;
            NMLOG_INFO("DbusMgr");
            connection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
            if (!connection) {
                NMLOG_FATAL("Error connecting to system bus: %s ", error->message);
                g_error_free(error);
            }
        }

        DbusMgr::~DbusMgr() {
            NMLOG_INFO("~DbusMgr");
            if (connection) {
                g_object_unref(connection);
                connection = nullptr;
            }
        }

        GDBusConnection* DbusMgr::getConnection()
        {
            if (connection) {
                return connection;
            }

            GError* error = nullptr;
            connection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
            if (!connection && error) {
                NMLOG_FATAL("Error reconnecting to system bus: %s ", error->message);
                g_error_free(error);
                return nullptr;
            }

            flags = static_cast<GDBusProxyFlags>(G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES | G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START);
            return connection;
        }

        GDBusProxy* DbusMgr::getNetworkManagerProxy()
        {
            GError* error = nullptr;
            nmProxy = NULL;
            nmProxy = g_dbus_proxy_new_sync(
                getConnection(), G_DBUS_PROXY_FLAGS_NONE, NULL, "org.freedesktop.NetworkManager",
                "/org/freedesktop/NetworkManager",
                "org.freedesktop.NetworkManager",
                NULL,
                &error
            );
    
            if (nmProxy == NULL|| error != NULL) {
                g_dbus_error_strip_remote_error(error);
                NMLOG_FATAL("Error creating D-Bus proxy: %s", error->message);
                g_error_free(error);
                return nullptr;
            }

            return nmProxy;
        }

        GDBusProxy* DbusMgr::getNetworkManagerSettingsProxy()
        {
            GError* error = nullptr;
            GDBusProxy* proxy = g_dbus_proxy_new_sync(
                getConnection(), flags, nullptr, "org.freedesktop.NetworkManager",
                "/org/freedesktop/NetworkManager/Settings",
                "org.freedesktop.NetworkManager.Settings",
                nullptr,
                &error
            );

            if (error != NULL) {
                g_dbus_error_strip_remote_error(error);
                NMLOG_FATAL("Error creating proxy: %s", error->message);
                g_clear_error(&error);
                return nullptr;
            }

            return proxy;
        }

        GDBusProxy* DbusMgr::getNetworkManagerDeviceProxy(const char* devicePath)
        {
            GError* error = nullptr;
            GDBusProxy* proxy = g_dbus_proxy_new_sync(
                getConnection(), flags, nullptr, "org.freedesktop.NetworkManager",
                devicePath,
                "org.freedesktop.NetworkManager.Device",
                nullptr,
                &error
            );

            if (error != NULL) {
                g_dbus_error_strip_remote_error(error);
                NMLOG_FATAL("Error creating proxy: %s", error->message);
                g_clear_error(&error);
                return nullptr;
            }

            return proxy;
        }

        GDBusProxy* DbusMgr::getNetworkManagerWirelessProxy(const char* wirelessDevPath)
        {
            GError* error = nullptr;
            GDBusProxy* proxy = g_dbus_proxy_new_sync(
                getConnection(), flags, nullptr, "org.freedesktop.NetworkManager",
                wirelessDevPath,
                "org.freedesktop.NetworkManager.Device.Wireless",
                nullptr,
                &error
            );

            if (error != NULL) {
                g_dbus_error_strip_remote_error(error);
                NMLOG_FATAL("Error creating proxy: %s", error->message);
                g_clear_error(&error);
                return nullptr;
            }

            return proxy;
        }

    } // Plugin
} // WPEFramework

