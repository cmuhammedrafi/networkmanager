#pragma once

#include <NetworkManager.h>
#include <libnm/NetworkManager.h>
#include <glib-object.h>
#include <string>

class LibnmWrapsImpl {
public:
    virtual ~LibnmWrapsImpl() = default;

    // NetworkManager library functions to mock
    virtual const char* nm_device_get_iface(NMDevice* device) = 0;
    virtual NMDevice* nm_client_get_device_by_iface(NMClient *client, const char *iface) = 0;
    virtual NMDeviceState nm_device_get_state(NMDevice *device) = 0;
    
    // New functions to mock
    virtual NMActiveConnection* nm_client_get_primary_connection(NMClient *client) = 0;
    virtual NMRemoteConnection* nm_active_connection_get_connection(NMActiveConnection *connection) = 0;
    virtual const char* nm_connection_get_interface_name(NMRemoteConnection *connection) = 0; // Changed to NMRemoteConnection*

    // virtual NMClient* nm_client_new(GCancellable* cancellable, GError** error) = 0;
    virtual const GPtrArray* nm_client_get_devices(NMClient* client) = 0;
};

class LibnmWraps {
protected:
    static LibnmWrapsImpl* impl;

public:
    LibnmWraps();
    LibnmWraps(const LibnmWraps &obj) = delete;
    static void setImpl(LibnmWrapsImpl* newImpl);
    static LibnmWraps& getInstance();

    // NetworkManager library functions
    static const char* nm_device_get_iface(NMDevice* device);
    static NMDevice* nm_client_get_device_by_iface(NMClient *client, const char *iface);
    static NMDeviceState nm_device_get_state(NMDevice *device);
    
    // New functions
    static NMActiveConnection* nm_client_get_primary_connection(NMClient *client);
    static NMRemoteConnection* nm_active_connection_get_connection(NMActiveConnection *connection);
    static const char* nm_connection_get_interface_name(NMRemoteConnection *connection); // Changed to NMRemoteConnection*
    
    // Add other NetworkManager functions as needed
    // static NMClient* nm_client_new(GCancellable* cancellable, GError** error);
    static const GPtrArray* nm_client_get_devices(NMClient* client);
};