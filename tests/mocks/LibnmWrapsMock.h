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
extern "C" const GPtrArray* __real_nm_client_get_active_connections(NMClient* client);
extern "C" NMSettingIPConfig* __real_nm_connection_get_setting_ip4_config(NMConnection* connection);
extern "C" NMSettingIPConfig* __real_nm_connection_get_setting_ip6_config(NMConnection* connection);
extern "C" const char* __real_nm_setting_ip_config_get_method(NMSettingIPConfig* setting);
extern "C" NMSettingConnection* __real_nm_connection_get_setting_connection(NMConnection* connection);
extern "C" const char* __real_nm_setting_connection_get_interface_name(NMSettingConnection* setting);
extern "C" NMIPConfig* __real_nm_active_connection_get_ip4_config(NMActiveConnection* connection);
extern "C" NMIPConfig* __real_nm_active_connection_get_ip6_config(NMActiveConnection* connection);
extern "C" GPtrArray* __real_nm_ip_config_get_addresses(NMIPConfig* config);
extern "C" const char* __real_nm_ip_address_get_address(NMIPAddress* address);
extern "C" guint __real_nm_ip_address_get_prefix(NMIPAddress* address);
extern "C" const char* __real_nm_ip_config_get_gateway(NMIPConfig* config);
extern "C" const char* const* __real_nm_ip_config_get_nameservers(NMIPConfig* config);
extern "C" NMDhcpConfig* __real_nm_active_connection_get_dhcp4_config(NMActiveConnection* connection);
extern "C" NMDhcpConfig* __real_nm_active_connection_get_dhcp6_config(NMActiveConnection* connection);
extern "C" const char* __real_nm_dhcp_config_get_one_option(NMDhcpConfig* config, const char* option);

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
                return __real_nm_client_new(cancellable, error);
            }));
        ON_CALL(*this, nm_device_get_hw_address(::testing::_))
            .WillByDefault(::testing::Invoke(
            [&](NMDevice* device) -> const char* {
                return __real_nm_device_get_hw_address(device);
            }));
        ON_CALL(*this, nm_client_get_active_connections(::testing::_))
            .WillByDefault(::testing::Invoke(
            [&](NMClient* client) -> const GPtrArray* {
                return __real_nm_client_get_active_connections(client);
            }));
        ON_CALL(*this, nm_connection_get_setting_ip4_config(::testing::_))
            .WillByDefault(::testing::Invoke(
            [&](NMConnection* connection) -> NMSettingIPConfig* {
                return __real_nm_connection_get_setting_ip4_config(connection);
            }));
        ON_CALL(*this, nm_connection_get_setting_ip6_config(::testing::_))
            .WillByDefault(::testing::Invoke(
            [&](NMConnection* connection) -> NMSettingIPConfig* {
                return __real_nm_connection_get_setting_ip6_config(connection);
            }));
        ON_CALL(*this, nm_setting_ip_config_get_method(::testing::_))
            .WillByDefault(::testing::Invoke(
            [&](NMSettingIPConfig* setting) -> const char* {
                return __real_nm_setting_ip_config_get_method(setting);
            }));
        ON_CALL(*this, nm_connection_get_setting_connection(::testing::_))
            .WillByDefault(::testing::Invoke(
            [&](NMConnection* connection) -> NMSettingConnection* {
                return __real_nm_connection_get_setting_connection(connection);
            }));
        ON_CALL(*this, nm_setting_connection_get_interface_name(::testing::_))
            .WillByDefault(::testing::Invoke(
            [&](NMSettingConnection* setting) -> const char* {
                return __real_nm_setting_connection_get_interface_name(setting);
            }));
        ON_CALL(*this, nm_active_connection_get_ip4_config(::testing::_))
            .WillByDefault(::testing::Invoke(
            [&](NMActiveConnection* connection) -> NMIPConfig* {
                return __real_nm_active_connection_get_ip4_config(connection);
            }));
        ON_CALL(*this, nm_active_connection_get_ip6_config(::testing::_))
            .WillByDefault(::testing::Invoke(
            [&](NMActiveConnection* connection) -> NMIPConfig* {
                return __real_nm_active_connection_get_ip6_config(connection);
            }));
        ON_CALL(*this, nm_ip_config_get_addresses(::testing::_))
            .WillByDefault(::testing::Invoke(
            [&](NMIPConfig* config) -> GPtrArray* {
                return __real_nm_ip_config_get_addresses(config);
            }));
        ON_CALL(*this, nm_ip_address_get_address(::testing::_))
            .WillByDefault(::testing::Invoke(
            [&](NMIPAddress* address) -> const char* {
                return __real_nm_ip_address_get_address(address);
            }));
        ON_CALL(*this, nm_ip_address_get_prefix(::testing::_))
            .WillByDefault(::testing::Invoke(
            [&](NMIPAddress* address) -> guint {
                return __real_nm_ip_address_get_prefix(address);
            }));
        ON_CALL(*this, nm_ip_config_get_gateway(::testing::_))
            .WillByDefault(::testing::Invoke(
            [&](NMIPConfig* config) -> const char* {
                return __real_nm_ip_config_get_gateway(config);
            }));
        ON_CALL(*this, nm_ip_config_get_nameservers(::testing::_))
            .WillByDefault(::testing::Invoke(
            [&](NMIPConfig* config) -> const char* const* {
                return __real_nm_ip_config_get_nameservers(config);
            }));
        ON_CALL(*this, nm_active_connection_get_dhcp4_config(::testing::_))
            .WillByDefault(::testing::Invoke(
            [&](NMActiveConnection* connection) -> NMDhcpConfig* {
                return __real_nm_active_connection_get_dhcp4_config(connection);
            }));
        ON_CALL(*this, nm_active_connection_get_dhcp6_config(::testing::_))
            .WillByDefault(::testing::Invoke(
            [&](NMActiveConnection* connection) -> NMDhcpConfig* {
                return __real_nm_active_connection_get_dhcp6_config(connection);
            }));
        ON_CALL(*this, nm_dhcp_config_get_one_option(::testing::_, ::testing::_))
            .WillByDefault(::testing::Invoke(
            [&](NMDhcpConfig* config, const char* option) -> const char* {
                return __real_nm_dhcp_config_get_one_option(config, option);
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
    MOCK_METHOD(const char*, nm_device_get_hw_address, (NMDevice* device), (override));
    MOCK_METHOD(const char*, nm_setting_connection_get_interface_name, (NMSettingConnection* settings), (override));
    MOCK_METHOD(const GPtrArray*, nm_client_get_active_connections, (NMClient* client), (override));
    MOCK_METHOD(NMSettingIPConfig*, nm_connection_get_setting_ip4_config, (NMConnection* connection), (override));
    MOCK_METHOD(NMSettingIPConfig*, nm_connection_get_setting_ip6_config, (NMConnection* connection), (override));
    MOCK_METHOD(const char*, nm_setting_ip_config_get_method, (NMSettingIPConfig* setting), (override));
    MOCK_METHOD(NMSettingConnection*, nm_connection_get_setting_connection, (NMConnection* connection), (override));
    MOCK_METHOD(NMIPConfig*, nm_active_connection_get_ip4_config, (NMActiveConnection* connection), (override));
    MOCK_METHOD(NMIPConfig*, nm_active_connection_get_ip6_config, (NMActiveConnection* connection), (override));
    MOCK_METHOD(NMDhcpConfig*, nm_active_connection_get_dhcp6_config, (NMActiveConnection* connection), (override));
    MOCK_METHOD(GPtrArray*, nm_ip_config_get_addresses, (NMIPConfig* config), (override));
    MOCK_METHOD(const char*, nm_ip_address_get_address, (NMIPAddress* address), (override));
    MOCK_METHOD(guint, nm_ip_address_get_prefix, (NMIPAddress* address), (override));
    MOCK_METHOD(const char*, nm_ip_config_get_gateway, (NMIPConfig* config), (override));
    MOCK_METHOD(const char* const*, nm_ip_config_get_nameservers, (NMIPConfig* config), (override));
    MOCK_METHOD(NMDhcpConfig*, nm_active_connection_get_dhcp4_config, (NMActiveConnection* connection), (override));
    MOCK_METHOD(const char*, nm_dhcp_config_get_one_option, (NMDhcpConfig* config, const char* option), (override));
};

