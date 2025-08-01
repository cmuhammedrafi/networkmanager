#pragma once

#include <gmock/gmock.h>
#include "LibnmWraps.h"

// Access to real functions if needed
extern "C" const char* __real_nm_device_get_iface(NMDevice* device);

class LibnmWrapsImplMock : public LibnmWrapsImpl {
public:
    LibnmWrapsImplMock() : LibnmWrapsImpl() {
        // Example default behavior for iface
        ON_CALL(*this, nm_device_get_iface(::testing::_))
            .WillByDefault(::testing::Return("wlan0"));
    }

    virtual ~LibnmWrapsImplMock() = default;

    // Mock methods for all required NetworkManager functions
    MOCK_METHOD(const char*, nm_device_get_iface, (NMDevice* device), (override));
    MOCK_METHOD(const char*, nm_device_get_hw_address, (NMDevice* device), (override));
    MOCK_METHOD(NMDeviceState, nm_device_get_state, (NMDevice* device), (override));
    MOCK_METHOD(NMDeviceType, nm_device_get_device_type, (NMDevice* device), (override));
    MOCK_METHOD(NMClient*, nm_client_new, (GCancellable* cancellable, GError** error), (override));
    MOCK_METHOD(const GPtrArray*, nm_client_get_devices, (NMClient* client), (override));
};