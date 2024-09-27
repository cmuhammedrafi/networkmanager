#pragma once

#include <gio/gio.h>
#include <iostream>

#include <cstdio>


// Define log levels
enum LogLevel {
    LOG_LEVEL_ERROR = 0,
    LOG_LEVEL_WARN = 1,
    LOG_LEVEL_INFO = 2,
    LOG_LEVEL_TRACE = 3,
    LOG_LEVEL_VERBOSE = 4
};

// Set the current log level (modify as needed)
static const int currentLogLevel = LOG_LEVEL_INFO;

#define NMLOG_ERROR(fmt, ...) \
    do { \
        if (currentLogLevel >= LOG_LEVEL_ERROR) { \
            fprintf(stderr, "[ERROR] [%s:%d] " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__); \
        } \
    } while(0)

#define NMLOG_WARNING(fmt, ...) \
    do { \
        if (currentLogLevel >= LOG_LEVEL_WARN) { \
            printf("[WARN] [%s:%d] " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__); \
        } \
    } while(0)

#define NMLOG_INFO(fmt, ...) \
    do { \
        if (currentLogLevel >= LOG_LEVEL_INFO) { \
            printf("[INFO] " fmt "\n", ##__VA_ARGS__); \
        } \
    } while(0)

#define NMLOG_TRACE(fmt, ...) \
    do { \
        if (currentLogLevel >= LOG_LEVEL_TRACE) { \
            printf("[TRACE] [%s:%d] " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__); \
        } \
    } while (0)
    

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
