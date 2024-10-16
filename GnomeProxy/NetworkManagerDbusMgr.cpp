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

#include "NetworkManagerDbusMgr.h"
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

    } // Plugin
} // WPEFramework

