#include "nm_example.h"
#include <iostream>
#include <NetworkManager.h>

NetworkManagerExample::NetworkManagerExample() {
    // Initialize if needed in older GLib versions
    #if !GLIB_CHECK_VERSION(2, 36, 0)
    g_type_init();
    #endif
}

NetworkManagerExample::~NetworkManagerExample() {
    // Cleanup if needed
}

std::vector<std::string> NetworkManagerExample::getInterfaceNames() {
    std::vector<std::string> interfaceNames;
    GError *error = NULL;
    
    // Create a new NM client instance
    NMClient *client = nm_client_new(NULL, &error);
    if (!client) {
        std::cerr << "Failed to create NM client: " << (error ? error->message : "unknown error") << std::endl;
        if (error)
            g_error_free(error);
        return interfaceNames;
    }

    
    // Get all devices
    const GPtrArray *devices = nm_client_get_devices(client);
    if (devices) {
        for (guint i = 0; i < devices->len; i++) {
            // Get each device and its interface name
            NMDevice *device = (NMDevice *)g_ptr_array_index(devices, i);
            if (device) {
                const char *iface = nm_device_get_iface(device);
                if (iface) {
                    std::cout << "Device " << i                                                                                                                                                                                                         << ": " << iface << std::endl;
                    // Store the interface name in the vector
                    interfaceNames.push_back(std::string(iface));
                }
            }
            else {
                std::cerr << "Device at index " << i << " is NULL." << std::endl;
            }
        }
    }
    else {
        std::cerr << "No devices found." << std::endl;
    }
    
    // Clean up
    g_object_unref(client);
    
    return interfaceNames;
}

std::vector<NetworkManagerExample::InterfaceDetails> NetworkManagerExample::GetAvailableInterfaces() {
    std::vector<InterfaceDetails> interfaceList;
    GError *error = NULL;
    
    // Create a new NM client instance
    NMClient *client = nm_client_new(NULL, &error);
    if (!client) {
        std::cerr << "Failed to create NM client: " << (error ? error->message : "unknown error") << std::endl;
        if (error)
            g_error_free(error);
        return interfaceList;
    }
    
    // Get all devices
    const GPtrArray *devices = nm_client_get_devices(client);
    if (devices) {
        for (guint i = 0; i < devices->len; i++) {
            NMDevice *device = (NMDevice *)g_ptr_array_index(devices, i);
            if (device) {
                const char* ifacePtr = nm_device_get_iface(device);
                if (ifacePtr == nullptr)
                    continue;
                
                std::string ifaceStr = ifacePtr;
                
                InterfaceDetails interface;
                interface.name = ifaceStr;
                
                // Get MAC address
                const char* hwAddr = nm_device_get_hw_address(device);
                interface.mac = hwAddr ? hwAddr : "";
                
                // Get device state
                NMDeviceState deviceState = nm_device_get_state(device);
                interface.enabled = (deviceState >= NM_DEVICE_STATE_UNAVAILABLE) ? true : false;
                interface.connected = (deviceState > NM_DEVICE_STATE_DISCONNECTED && 
                                      deviceState < NM_DEVICE_STATE_DEACTIVATING) ? true : false;
                
                // Determine device type
                NMDeviceType deviceType = nm_device_get_device_type(device);
                if (deviceType == NM_DEVICE_TYPE_WIFI) {
                    interface.type = "WIFI";
                } else if (deviceType == NM_DEVICE_TYPE_ETHERNET) {
                    interface.type = "ETHERNET";
                } else {
                    interface.type = "OTHER";
                }
                
                interfaceList.push_back(interface);
            }
        }
    }
    
    // Clean up
    g_object_unref(client);
    
    return interfaceList;
}

