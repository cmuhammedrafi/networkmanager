#include "dbusConnectionManager.h"

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
