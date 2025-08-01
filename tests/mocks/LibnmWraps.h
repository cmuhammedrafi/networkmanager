#pragma once

#include <NetworkManager.h>
#include <glib-object.h>
#include <string>

class LibnmWrapsImpl {
public:
    virtual ~LibnmWrapsImpl() = default;

    // NetworkManager library functions to mock
    virtual const char* nm_device_get_iface(NMDevice* device) = 0;
    virtual const char* nm_device_get_hw_address(NMDevice* device) = 0;
    virtual NMDeviceState nm_device_get_state(NMDevice* device) = 0;
    virtual NMDeviceType nm_device_get_device_type(NMDevice* device) = 0;
    virtual NMClient* nm_client_new(GCancellable* cancellable, GError** error) = 0;
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
    static const char* nm_device_get_hw_address(NMDevice* device);
    static NMDeviceState nm_device_get_state(NMDevice* device);
    static NMDeviceType nm_device_get_device_type(NMDevice* device);
    static NMClient* nm_client_new(GCancellable* cancellable, GError** error);
    static const GPtrArray* nm_client_get_devices(NMClient* client);
};