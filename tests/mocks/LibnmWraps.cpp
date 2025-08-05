#include "LibnmWraps.h"
#include <gmock/gmock.h>

// Wrapper functions that will replace the real NetworkManager functions
extern "C" const char* __wrap_nm_device_get_iface(NMDevice* device) {
    return LibnmWraps::getInstance().nm_device_get_iface(device);
}

extern "C" NMActiveConnection* __wrap_nm_client_get_primary_connection(NMClient *client) {
    return LibnmWraps::getInstance().nm_client_get_primary_connection(client);
}

extern "C" NMRemoteConnection* __wrap_nm_active_connection_get_connection(NMActiveConnection *connection) {
    return LibnmWraps::getInstance().nm_active_connection_get_connection(connection);
}

extern "C" const char* __wrap_nm_connection_get_interface_name(NMRemoteConnection *connection) { // Changed to NMRemoteConnection*
    return LibnmWraps::getInstance().nm_connection_get_interface_name(connection);
}

extern "C" NMDevice* __wrap_nm_client_get_device_by_iface(NMClient *client, const char *iface) {
    return LibnmWraps::getInstance().nm_client_get_device_by_iface(client, iface);
}

extern "C" NMDeviceState __wrap_nm_device_get_state(NMDevice *device) {
    return LibnmWraps::getInstance().nm_device_get_state(device);
}

// Add wrapper function
extern "C" const GPtrArray* __wrap_nm_client_get_devices(NMClient* client) {
    return LibnmWraps::getInstance().nm_client_get_devices(client);
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

NMActiveConnection* LibnmWraps::nm_client_get_primary_connection(NMClient *client) {
    EXPECT_NE(impl, nullptr);
    return impl->nm_client_get_primary_connection(client);
}

NMRemoteConnection* LibnmWraps::nm_active_connection_get_connection(NMActiveConnection *connection) {
    EXPECT_NE(impl, nullptr);
    return impl->nm_active_connection_get_connection(connection);
}

const char* LibnmWraps::nm_connection_get_interface_name(NMRemoteConnection *connection) { // Changed to NMRemoteConnection*
    EXPECT_NE(impl, nullptr);
    return impl->nm_connection_get_interface_name(connection);
}

NMDevice* LibnmWraps::nm_client_get_device_by_iface(NMClient *client, const char *iface) {
    EXPECT_NE(impl, nullptr);
    return impl->nm_client_get_device_by_iface(client, iface);
}

NMDeviceState LibnmWraps::nm_device_get_state(NMDevice *device) {
    EXPECT_NE(impl, nullptr);
    return impl->nm_device_get_state(device);
}

// Add implementation
const GPtrArray* LibnmWraps::nm_client_get_devices(NMClient* client) {
    EXPECT_NE(impl, nullptr);
    return impl->nm_client_get_devices(client);
}