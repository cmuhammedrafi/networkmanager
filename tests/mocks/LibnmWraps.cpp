#include "LibnmWraps.h"
#include <gmock/gmock.h>

// Wrapper functions that will replace the real NetworkManager functions
extern "C" const char* __wrap_nm_device_get_iface(NMDevice* device) {
    return LibnmWraps::getInstance().nm_device_get_iface(device);
}

// Add other wrappers as needed
// extern "C" NMClient* __wrap_nm_client_new(GCancellable* cancellable, GError** error) {
//     return LibnmWraps::getInstance().nm_client_new(cancellable, error);
// }

// extern "C" const GPtrArray* __wrap_nm_client_get_devices(NMClient* client) {
//     return LibnmWraps::getInstance().nm_client_get_devices(client);
// }

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

// Add implementations for other functions as needed
// NMClient* LibnmWraps::nm_client_new(GCancellable* cancellable, GError** error) {
//     EXPECT_NE(impl, nullptr);
//     return impl->nm_client_new(cancellable, error);
// }

// const GPtrArray* LibnmWraps::nm_client_get_devices(NMClient* client) {
//     EXPECT_NE(impl, nullptr);
//     return impl->nm_client_get_devices(client);
// }