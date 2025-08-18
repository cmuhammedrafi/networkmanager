#pragma once

#include <NetworkManager.h>
#include <libnm/NetworkManager.h>
#include <glib-object.h>
#include <string>

class LibnmWrapsImpl {
public:
    virtual ~LibnmWrapsImpl() = default;

    virtual const char* nm_device_get_iface(NMDevice* device) = 0;
    virtual NMDevice* nm_client_get_device_by_iface(NMClient *client, const char *iface) = 0;
    virtual NMDeviceState nm_device_get_state(NMDevice *device) = 0;
    virtual NMActiveConnection* nm_client_get_primary_connection(NMClient *client) = 0;
    virtual NMRemoteConnection* nm_active_connection_get_connection(NMActiveConnection *connection) = 0;
    virtual const char* nm_connection_get_interface_name(NMRemoteConnection *connection) = 0;
    virtual NMClient* nm_client_new(GCancellable* cancellable, GError** error) = 0;
    virtual const GPtrArray* nm_client_get_devices(NMClient* client) = 0;
    virtual const char* nm_device_get_hw_address(NMDevice* device) = 0;
    virtual const GPtrArray* nm_client_get_active_connections(NMClient* client) = 0;
    virtual NMSettingIPConfig* nm_connection_get_setting_ip4_config(NMConnection* connection) = 0;
    virtual NMSettingIPConfig* nm_connection_get_setting_ip6_config(NMConnection* connection) = 0;
    virtual const char* nm_setting_ip_config_get_method(NMSettingIPConfig* setting) = 0;
    virtual NMSettingConnection* nm_connection_get_setting_connection(NMConnection* connection) = 0;
    virtual const char* nm_setting_connection_get_interface_name(NMSettingConnection* setting) = 0;
    virtual NMIPConfig* nm_active_connection_get_ip4_config(NMActiveConnection* connection) = 0;
    virtual NMIPConfig* nm_active_connection_get_ip6_config(NMActiveConnection* connection) = 0;
    virtual GPtrArray* nm_ip_config_get_addresses(NMIPConfig* config) = 0;
    virtual const char* nm_ip_address_get_address(NMIPAddress* address) = 0;
    virtual guint nm_ip_address_get_prefix(NMIPAddress* address) = 0;
    virtual const char* nm_ip_config_get_gateway(NMIPConfig* config) = 0;
    virtual const char* const* nm_ip_config_get_nameservers(NMIPConfig* config) = 0;
    virtual NMDhcpConfig* nm_active_connection_get_dhcp4_config(NMActiveConnection* connection) = 0;
    virtual NMDhcpConfig* nm_active_connection_get_dhcp6_config(NMActiveConnection* connection) = 0;
    virtual const char* nm_dhcp_config_get_one_option(NMDhcpConfig* config, const char* option) = 0;
};

class LibnmWraps {
protected:
    static LibnmWrapsImpl* impl;

public:
    LibnmWraps();
    LibnmWraps(const LibnmWraps &obj) = delete;
    static void setImpl(LibnmWrapsImpl* newImpl);
    static LibnmWraps& getInstance();

    static const char* nm_device_get_iface(NMDevice* device);
    static NMDevice* nm_client_get_device_by_iface(NMClient *client, const char *iface);
    static NMDeviceState nm_device_get_state(NMDevice *device);
    static NMActiveConnection* nm_client_get_primary_connection(NMClient *client);
    static NMRemoteConnection* nm_active_connection_get_connection(NMActiveConnection *connection);
    static const char* nm_connection_get_interface_name(NMRemoteConnection *connection);
    static NMClient* nm_client_new(GCancellable* cancellable, GError** error);
    static const GPtrArray* nm_client_get_devices(NMClient* client);
    static const char* nm_device_get_hw_address(NMDevice* device);
    static const GPtrArray* nm_client_get_active_connections(NMClient* client);
    static NMSettingIPConfig* nm_connection_get_setting_ip4_config(NMConnection* connection);
    static NMSettingIPConfig* nm_connection_get_setting_ip6_config(NMConnection* connection);
    static const char* nm_setting_ip_config_get_method(NMSettingIPConfig* setting);
    static NMSettingConnection* nm_connection_get_setting_connection(NMConnection* connection);
    static const char* nm_setting_connection_get_interface_name(NMSettingConnection* setting);
    static NMIPConfig* nm_active_connection_get_ip4_config(NMActiveConnection* connection);
    static NMIPConfig* nm_active_connection_get_ip6_config(NMActiveConnection* connection);
    static GPtrArray* nm_ip_config_get_addresses(NMIPConfig* config);
    static const char* nm_ip_address_get_address(NMIPAddress* address);
    static guint nm_ip_address_get_prefix(NMIPAddress* address);
    static const char* nm_ip_config_get_gateway(NMIPConfig* config);
    static const char* const* nm_ip_config_get_nameservers(NMIPConfig* config);
    static NMDhcpConfig* nm_active_connection_get_dhcp4_config(NMActiveConnection* connection);
    static NMDhcpConfig* nm_active_connection_get_dhcp6_config(NMActiveConnection* connection);
    static const char* nm_dhcp_config_get_one_option(NMDhcpConfig* config, const char* option);
};