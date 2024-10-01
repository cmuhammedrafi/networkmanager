#pragma once

#include <gio/gio.h>
#include <iostream>

#include <cstdio>

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
