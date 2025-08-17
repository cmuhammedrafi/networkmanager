#pragma once

#include <gmock/gmock.h>
#include <NetworkManager.h>
#include "LibnmWraps.h"


extern "C" const char* __real_nm_device_get_iface(NMDevice* device);
extern "C" NMActiveConnection* __real_nm_client_get_primary_connection(NMClient *client);
extern "C" NMRemoteConnection* __real_nm_active_connection_get_connection(NMActiveConnection *connection);
extern "C" const char* __real_nm_connection_get_interface_name(NMRemoteConnection *connection);
extern "C" NMDevice* __real_nm_client_get_device_by_iface(NMClient *client, const char *iface);
extern "C" NMDeviceState __real_nm_device_get_state(NMDevice *device);
extern "C" const GPtrArray* __real_nm_client_get_devices(NMClient* client);
extern "C" NMClient* __real_nm_client_new(GCancellable* cancellable, GError** error);
extern "C" const char* __real_nm_device_get_hw_address(NMDevice* device);

class LibnmWrapsImplMock : public LibnmWrapsImpl {
public:
    LibnmWrapsImplMock() : LibnmWrapsImpl() {

        ON_CALL(*this, nm_device_get_iface(::testing::_))
            .WillByDefault(::testing::Invoke(
            [&](NMDevice* device) -> const char* {
                return __real_nm_device_get_iface(device);
            }));
        ON_CALL(*this, nm_client_get_device_by_iface(::testing::_, ::testing::_))
            .WillByDefault(::testing::Invoke(
            [&](NMClient* client, const char* iface) -> NMDevice* {
                return __real_nm_client_get_device_by_iface(client, iface);
            }));
        ON_CALL(*this, nm_device_get_state(::testing::_))
            .WillByDefault(::testing::Invoke(
            [&](NMDevice* device) -> NMDeviceState {
                return __real_nm_device_get_state(device);
            }));
        ON_CALL(*this, nm_client_get_primary_connection(::testing::_))
            .WillByDefault(::testing::Invoke(
            [&](NMClient* client) -> NMActiveConnection* {
                return __real_nm_client_get_primary_connection(client);
            }));
        ON_CALL(*this, nm_active_connection_get_connection(::testing::_))
            .WillByDefault(::testing::Invoke(
            [&](NMActiveConnection* connection) -> NMRemoteConnection* {
                return __real_nm_active_connection_get_connection(connection);
            }));
        ON_CALL(*this, nm_connection_get_interface_name(::testing::_))
            .WillByDefault(::testing::Invoke(
            [&](NMRemoteConnection* connection) -> const char* {
                return __real_nm_connection_get_interface_name(connection);
            }));
        ON_CALL(*this, nm_client_get_devices(::testing::_))
            .WillByDefault(::testing::Invoke(
            [&](NMClient* client) -> const GPtrArray* {
                return __real_nm_client_get_devices(client);
            }));
        ON_CALL(*this, nm_client_new(::testing::_, ::testing::_))
            .WillByDefault(::testing::Invoke(
            [&](GCancellable* cancellable, GError** error) -> NMClient* {
                NMClient* client = __real_nm_client_new(cancellable, error);
                s_realClient = client;
                return client;
            }));
        ON_CALL(*this, nm_device_get_hw_address(::testing::_))
            .WillByDefault(::testing::Invoke(
            [&](NMDevice* device) -> const char* {
                return __real_nm_device_get_hw_address(device);
            }));
    }

    virtual ~LibnmWrapsImplMock() = default;

    // Mock methods
    MOCK_METHOD(const char*, nm_device_get_iface, (NMDevice* device), (override));
    MOCK_METHOD(NMDevice*, nm_client_get_device_by_iface, (NMClient *client, const char *iface), (override));
    MOCK_METHOD(NMDeviceState, nm_device_get_state, (NMDevice *device), (override));
    MOCK_METHOD(NMActiveConnection*, nm_client_get_primary_connection, (NMClient *client), (override));
    MOCK_METHOD(NMRemoteConnection*, nm_active_connection_get_connection, (NMActiveConnection *connection), (override));
    MOCK_METHOD(const char*, nm_connection_get_interface_name, (NMRemoteConnection *connection), (override));
    MOCK_METHOD(NMClient*, nm_client_new, (GCancellable* cancellable, GError** error), (override));
    MOCK_METHOD(const GPtrArray*, nm_client_get_devices, (NMClient* client), (override));
    MOCK_METHOD(const GPtrArray*, nm_client_get_active_connections, (NMClient* client), ());
    MOCK_METHOD(const char*, nm_device_get_hw_address, (NMDevice* device), ());
    MOCK_METHOD(int, nm_device_get_type, (NMDevice* device), ());
    MOCK_METHOD(const GPtrArray*, nm_ip_config_get_addresses, (void* ip_config), ());
    MOCK_METHOD(const char*, nm_ip_config_get_gateway, (void* ip_config), ());
    MOCK_METHOD(char**, nm_ip_config_get_nameservers, (void* ip_config), ());
    MOCK_METHOD(void*, nm_active_connection_get_ip4_config, (NMActiveConnection* conn), ());
    MOCK_METHOD(void*, nm_active_connection_get_ip6_config, (NMActiveConnection* conn), ());
    MOCK_METHOD(void*, nm_active_connection_get_dhcp4_config, (NMActiveConnection* conn), ());
    MOCK_METHOD(void*, nm_active_connection_get_dhcp6_config, (NMActiveConnection* conn), ());
    MOCK_METHOD(const char*, nm_dhcp_config_get_one_option, (void* dhcp_config, const char* opt), ());
    MOCK_METHOD(void*, nm_connection_get_setting_connection, (void* connection), ());
    MOCK_METHOD(const char*, nm_setting_connection_get_interface_name, (void* settings), ());
    MOCK_METHOD(void*, nm_connection_get_setting_ip4_config, (void* connection), ());
    MOCK_METHOD(const char*, nm_setting_ip_config_get_method, (void* ip_config), ());

    static NMClient* s_realClient;
    static NMClient* getSavedRealNMClient() { return s_realClient; }
};

NMClient* LibnmWrapsImplMock::s_realClient = nullptr;