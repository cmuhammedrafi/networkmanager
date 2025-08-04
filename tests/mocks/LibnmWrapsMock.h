#pragma once

#include <gmock/gmock.h>
#include <NetworkManager.h>
#include "LibnmWraps.h"

// Access to real functions if needed
extern "C" const char* __real_nm_device_get_iface(NMDevice* device);

class LibnmWrapsImplMock : public LibnmWrapsImpl {
public:
    LibnmWrapsImplMock() : LibnmWrapsImpl() {
        // Default behavior could call real functions if needed
        // ON_CALL(*this, nm_device_get_iface(::testing::_))
        //     .WillByDefault(::testing::Invoke(
        //         [&](NMDevice* device) -> const char* {
        //             return __real_nm_device_get_iface(device);
        //         }));
        
        // For this example, we always return "wlan"
        ON_CALL(*this, nm_device_get_iface(::testing::_))
            .WillByDefault(::testing::Return("wlan"));
            
        // Set default behaviors for new functions
        ON_CALL(*this, nm_connection_get_interface_name(::testing::_))
            .WillByDefault(::testing::Return("wlan"));
    }
    
    virtual ~LibnmWrapsImplMock() = default;

    // Mock methods
    MOCK_METHOD(const char*, nm_device_get_iface, (NMDevice* device), (override));
    MOCK_METHOD(NMDevice*, nm_client_get_device_by_iface, (NMClient *client, const char *iface), (override));
    MOCK_METHOD(NMDeviceState, nm_device_get_state, (NMDevice *device), (override));
    
    // New mock methods
    MOCK_METHOD(NMActiveConnection*, nm_client_get_primary_connection, (NMClient *client), (override));
    MOCK_METHOD(NMRemoteConnection*, nm_active_connection_get_connection, (NMActiveConnection *connection), (override));
    MOCK_METHOD(const char*, nm_connection_get_interface_name, (NMRemoteConnection *connection), (override)); // Changed to NMRemoteConnection*

    // Add other mock methods as needed
    // MOCK_METHOD(NMClient*, nm_client_new, (GCancellable* cancellable, GError** error), (override));
    // MOCK_METHOD(const GPtrArray*, nm_client_get_devices, (NMClient* client), (override));
};