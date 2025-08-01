#include "LibnmWraps.h"
#include <gmock/gmock.h>

// Wrapper functions that will replace the real NetworkManager functions
extern "C" const char* __wrap_nm_device_get_iface(NMDevice* device) {
    return LibnmWraps::getInstance().nm_device_get_iface(device);
}

extern "C" NMClient* __wrap_nm_client_new(GCancellable* cancellable, GError** error) {
    return LibnmWraps::getInstance().nm_client_new(cancellable, error);
}

extern "C" const GPtrArray* __wrap_nm_client_get_devices(NMClient* client) {
    return LibnmWraps::getInstance().nm_client_get_devices(client);
}

// Add these wrapper functions:

extern "C" const char* __wrap_nm_device_get_hw_address(NMDevice* device) {
    return LibnmWraps::getInstance().nm_device_get_hw_address(device);
}

extern "C" NMDeviceState __wrap_nm_device_get_state(NMDevice* device) {
    return LibnmWraps::getInstance().nm_device_get_state(device);
}

extern "C" NMDeviceType __wrap_nm_device_get_device_type(NMDevice* device) {
    return LibnmWraps::getInstance().nm_device_get_device_type(device);
}

// Initialize static member
LibnmWrapsImpl* LibnmWraps::impl = nullptr;

LibnmWraps::LibnmWraps() {}

void LibnmWraps::setImpl(LibnmWrapsImpl* newImpl) {
    // Handles both resetting 'impl' to nullptr and assigning a new value
    EXPECT_TRUE((nullptr == impl) || (nullptr == newImpl));
    impl = newImpl;
}

LibnmWraps& LibnmWraps::getInstance() {
    static LibnmWraps instance;
    return instance;
}

const char* LibnmWraps::nm_device_get_iface(NMDevice* device) {
    EXPECT_NE(impl, nullptr);
    return impl->nm_device_get_iface(device);
}

NMClient* LibnmWraps::nm_client_new(GCancellable* cancellable, GError** error) {
    EXPECT_NE(impl, nullptr);
    return impl->nm_client_new(cancellable, error);
}

const GPtrArray* LibnmWraps::nm_client_get_devices(NMClient* client) {
    EXPECT_NE(impl, nullptr);
    return impl->nm_client_get_devices(client);
}

// And add the implementation functions:

const char* LibnmWraps::nm_device_get_hw_address(NMDevice* device) {
    EXPECT_NE(impl, nullptr);
    return impl->nm_device_get_hw_address(device);
}

NMDeviceState LibnmWraps::nm_device_get_state(NMDevice* device) {
    EXPECT_NE(impl, nullptr);
    return impl->nm_device_get_state(device);
}

NMDeviceType LibnmWraps::nm_device_get_device_type(NMDevice* device) {
    EXPECT_NE(impl, nullptr);
    return impl->nm_device_get_device_type(device);
}