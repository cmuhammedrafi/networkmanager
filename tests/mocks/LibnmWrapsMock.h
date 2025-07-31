#pragma once

#include <gmock/gmock.h>
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
        
        // For this example, we always return "wlan0"
        ON_CALL(*this, nm_device_get_iface(::testing::_))
            .WillByDefault(::testing::Return("wlan0"));
    }
    
    virtual ~LibnmWrapsImplMock() = default;

    // Mock methods
    MOCK_METHOD(const char*, nm_device_get_iface, (NMDevice* device), (override));
    
    // Add other mock methods as needed
    // MOCK_METHOD(NMClient*, nm_client_new, (GCancellable* cancellable, GError** error), (override));
    // MOCK_METHOD(const GPtrArray*, nm_client_get_devices, (NMClient* client), (override));
};