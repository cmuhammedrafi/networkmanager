/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdlib.h>
#include <stdio.h>
#include <gio/gio.h>
#include <string.h>
#include <libnm/nm-dbus-interface.h>
#include <thread>
#include <string>
#include <map>

#include "NetworkManagerGdbusEvent.h"
#include "NetworkManagerLogger.h"
#include "NetworkManagerGdbusUtils.h"
#include "INetworkManager.h"

namespace WPEFramework
{
    namespace Plugin
    {

    static NetworkManagerEvents *_NetworkManagerEvents = nullptr;

    const char * nm_state_to_string(NMState state)
    {
        switch (state) {
        case NM_STATE_ASLEEP:
            return "asleep";
        case NM_STATE_CONNECTING:
            return "connecting";
        case NM_STATE_CONNECTED_LOCAL:
            return "connected (local only)";
        case NM_STATE_CONNECTED_SITE:
            return "connected (site only)";
        case NM_STATE_CONNECTED_GLOBAL:
            return "connected";
        case NM_STATE_DISCONNECTING:
            return "disconnecting";
        case NM_STATE_DISCONNECTED:
            return "disconnected";
        case NM_STATE_UNKNOWN:
        default:
            return "unknown";
        }
    }

    static std::string getInterafaceNameFormDevicePath(const char* DevicePath)
    {
        GDBusProxy *deviceProxy = NULL;
        std::string ifname;
        deviceProxy =_NetworkManagerEvents->eventDbus.getNetworkManagerDeviceProxy(DevicePath);
        if (deviceProxy == NULL)
            return ifname;
        GVariant *ifaceVariant = g_dbus_proxy_get_cached_property(deviceProxy, "Interface");
        if (ifaceVariant == NULL) {
            NMLOG_WARNING("Interface property get error");
            return ifname;
        }

        const gchar *ifaceName = g_variant_get_string(ifaceVariant, NULL);
        if(ifaceName != nullptr)
            ifname.assign(ifaceName, 0, 16);

        g_variant_unref(ifaceVariant);
        g_object_unref(deviceProxy);
        return ifname;
    }

    static void lastScanPropertiesChangedCB(GDBusProxy *proxy,
                        GVariant    *changed_properties,
                        const gchar *invalidated_properties[],
                        gpointer     user_data) {

        if (proxy == NULL) {
            NMLOG_FATAL("cb doesn't have proxy ");
            return;
        }

        GVariant *lastScanVariant = g_variant_lookup_value(changed_properties, "LastScan", NULL);
        if (lastScanVariant == NULL)
            return;

        gint64 lastScanTime = g_variant_get_int64(lastScanVariant);
        NMLOG_DEBUG("LastScan value changed: %" G_GINT64_FORMAT, lastScanTime);
        g_variant_unref(lastScanVariant);

        const gchar *object_path = g_dbus_proxy_get_object_path(proxy);
        if (object_path == NULL) {
            NMLOG_WARNING("Failed to get the proxy object path");
            return;
        }

        NMLOG_DEBUG("Proxy object path: %s", object_path);
        NetworkManagerEvents::onAvailableSSIDsCb(object_path);
    }

    static bool subscribeForlastScanPropertyEvent(const gchar *wirelessPath)
    {
        // Todo clean the proxy first
        NMLOG_DEBUG("Monitoring lastScan time property: %s", wirelessPath);
        _NetworkManagerEvents->gdbusProxys.wirelessDevProxy = _NetworkManagerEvents->eventDbus.getNetworkManagerWirelessProxy(wirelessPath);
        if (_NetworkManagerEvents->gdbusProxys.wirelessDevProxy == nullptr) {
            return false;
        }
        g_signal_connect(_NetworkManagerEvents->gdbusProxys.wirelessDevProxy, "g-properties-changed", G_CALLBACK(lastScanPropertiesChangedCB), NULL);
        return true;
    }

    static void primaryConnectionChangedCB(GDBusProxy *proxy,
                      GVariant *changed_properties,
                      GStrv invalidated_properties,
                      gpointer user_data)
    {
        if (changed_properties == NULL) {
            NMLOG_FATAL("cb doesn't have changed_properties ");
            return;
        }
        GVariant *primary_connection_variant = g_variant_lookup_value(changed_properties, "PrimaryConnection", NULL);
        if (primary_connection_variant != NULL)
        {
            const gchar *primary_connection;
            primary_connection = g_variant_get_string(primary_connection_variant, NULL);
            if(primary_connection && g_strcmp0(primary_connection, "/") == 0)
                NMLOG_WARNING("no active primary connection");
            else if(primary_connection)
                NMLOG_INFO("PrimaryConnection changed: %s", primary_connection);
            else
                NMLOG_ERROR("PrimaryConnection changed Null Error");
        }
    }

    static void deviceAddRemoveCb(GDBusProxy *proxy,
                                            gchar *sender_name,
                                            gchar *signal_name,
                                            GVariant *parameters,
                                            gpointer user_data) {

        if (signal_name == NULL) {
            NMLOG_FATAL("cb doesn't have signal_name ");
            return;
        }

        const gchar *devicePath = NULL;
        std::string ifname;
        if (g_strcmp0(signal_name, "DeviceAdded") == 0 || g_strcmp0(signal_name, "DeviceRemoved") == 0)
        {
            if (g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)) {
                GVariant *first_element = g_variant_get_child_value(parameters, 0);

                if (g_variant_is_of_type(first_element, G_VARIANT_TYPE_OBJECT_PATH)) {
                    devicePath = g_variant_get_string(first_element, NULL);
                    ifname = getInterafaceNameFormDevicePath(devicePath);
                } else {
                    NMLOG_ERROR("varient type not object path %s", g_variant_get_type_string(first_element));
                }
                g_variant_unref(first_element);
            }
            if (g_strcmp0(signal_name, "DeviceAdded") == 0) {
                NMLOG_INFO("Device Added: %s", ifname.c_str());
                if(ifname == GnomeUtils::getWifiIfname() || ifname == GnomeUtils::getEthIfname() )
                {
                    NMLOG_INFO("monitoring device: %s", ifname.c_str());
                    NetworkManagerEvents::onInterfaceStateChangeCb(Exchange::INetworkManager::INTERFACE_ADDED, ifname.c_str());
                    // TODO monitor device 
                    // monitorDevice()
                }
            }
            else if(g_strcmp0(signal_name, "DeviceRemoved")) {
                NMLOG_INFO("Device Removed: %s", ifname.c_str());
                if(ifname == GnomeUtils::getWifiIfname() || ifname == GnomeUtils::getEthIfname() )
                {
                    NMLOG_INFO("monitoring device: %s", ifname.c_str());
                    NetworkManagerEvents::onInterfaceStateChangeCb(Exchange::INetworkManager::INTERFACE_REMOVED, ifname.c_str());
                }
            }
        }
    }

    static void deviceStateChangedCB(GDBusProxy *proxy,
                                        gchar *sender_name,
                                        gchar *signal_name,
                                        GVariant *parameters,
                                        gpointer user_data) {

        if (signal_name == NULL || g_strcmp0(signal_name, "StateChanged") != 0) {
            return;
        }

        const gchar *device_path = g_dbus_proxy_get_object_path(proxy);
        std::string ifname = getInterafaceNameFormDevicePath(device_path);
        if(ifname.empty())
        {
            NMLOG_WARNING("interface name null");
            return;
        }
        guint32 oldState, newState, stateReason;
        g_variant_get(parameters, "(uuu)", &newState, &oldState, &stateReason);
        NMDeviceState deviceState = static_cast<NMDeviceState>(newState);
        NMDeviceStateReason deviceStateReason = static_cast<NMDeviceStateReason>(stateReason);

        NMLOG_DEBUG("%s: Old State: %u, New State: %u, Reason: %u", ifname.c_str(), oldState, newState, stateReason);
        if(ifname == GnomeUtils::getWifiIfname())
        {
            std::string wifiState;
            switch (deviceStateReason)
            {
                case NM_DEVICE_STATE_REASON_SUPPLICANT_AVAILABLE:
                    wifiState = "WIFI_STATE_UNINSTALLED";
                    NetworkManagerEvents::onWIFIStateChanged(Exchange::INetworkManager::WIFI_STATE_UNINSTALLED, wifiState);
                    break;
                case NM_DEVICE_STATE_REASON_SSID_NOT_FOUND:
                    wifiState = "WIFI_STATE_SSID_NOT_FOUND";
                    NetworkManagerEvents::onWIFIStateChanged(Exchange::INetworkManager::WIFI_STATE_SSID_NOT_FOUND, wifiState);
                    break;
                case NM_DEVICE_STATE_REASON_SUPPLICANT_TIMEOUT:         // supplicant took too long to authenticate
                case NM_DEVICE_STATE_REASON_NO_SECRETS:
                    wifiState = "WIFI_STATE_AUTHENTICATION_FAILED";
                    NetworkManagerEvents::onWIFIStateChanged(Exchange::INetworkManager::WIFI_STATE_AUTHENTICATION_FAILED, wifiState);
                    break;
                case NM_DEVICE_STATE_REASON_SUPPLICANT_FAILED:          //  802.1x supplicant failed
                    wifiState = "WIFI_STATE_ERROR";
                    NetworkManagerEvents::onWIFIStateChanged(Exchange::INetworkManager::WIFI_STATE_ERROR, wifiState);
                    break;
                case NM_DEVICE_STATE_REASON_SUPPLICANT_CONFIG_FAILED:   // 802.1x supplicant configuration failed
                    wifiState = "WIFI_STATE_CONNECTION_INTERRUPTED";
                    NetworkManagerEvents::onWIFIStateChanged(Exchange::INetworkManager::WIFI_STATE_CONNECTION_INTERRUPTED, wifiState);
                    break;
                case NM_DEVICE_STATE_REASON_SUPPLICANT_DISCONNECT:      // 802.1x supplicant disconnected
                    wifiState = "WIFI_STATE_INVALID_CREDENTIALS";
                    NetworkManagerEvents::onWIFIStateChanged(Exchange::INetworkManager::WIFI_STATE_INVALID_CREDENTIALS, wifiState);
                    break;
                default:
                {
                    switch (deviceState)
                    {
                    case NM_DEVICE_STATE_UNKNOWN:
                        wifiState = "WIFI_STATE_UNINSTALLED";
                        NetworkManagerEvents::onWIFIStateChanged(Exchange::INetworkManager::WIFI_STATE_UNINSTALLED, wifiState);
                        break;
                    case NM_DEVICE_STATE_UNMANAGED:
                        wifiState = "WIFI_STATE_DISABLED";
                        NetworkManagerEvents::onWIFIStateChanged(Exchange::INetworkManager::WIFI_STATE_DISABLED, wifiState);
                        break;
                    case NM_DEVICE_STATE_UNAVAILABLE:
                    case NM_DEVICE_STATE_DISCONNECTED:
                        wifiState = "WIFI_STATE_DISCONNECTED";
                        NetworkManagerEvents::onWIFIStateChanged(Exchange::INetworkManager::WIFI_STATE_DISCONNECTED, wifiState);
                        break;
                    case NM_DEVICE_STATE_PREPARE:
                        wifiState = "WIFI_STATE_PAIRING";
                        NetworkManagerEvents::onWIFIStateChanged(Exchange::INetworkManager::WIFI_STATE_PAIRING, wifiState);
                        break;
                    case NM_DEVICE_STATE_CONFIG:
                        wifiState = "WIFI_STATE_CONNECTING";
                        NetworkManagerEvents::onWIFIStateChanged(Exchange::INetworkManager::WIFI_STATE_CONNECTING, wifiState);
                        break;
                    case NM_DEVICE_STATE_IP_CHECK:
                        NetworkManagerEvents::onInterfaceStateChangeCb(Exchange::INetworkManager::INTERFACE_ACQUIRING_IP,"wlan0");
                        break;
                    case NM_DEVICE_STATE_ACTIVATED:
                        wifiState = "WIFI_STATE_CONNECTED";
                        NetworkManagerEvents::onWIFIStateChanged(Exchange::INetworkManager::WIFI_STATE_CONNECTED, wifiState);
                        break;
                    case NM_DEVICE_STATE_DEACTIVATING:
                        wifiState = "WIFI_STATE_CONNECTION_LOST";
                        NetworkManagerEvents::onWIFIStateChanged(Exchange::INetworkManager::WIFI_STATE_CONNECTION_LOST, wifiState);
                        break;
                    case NM_DEVICE_STATE_FAILED:
                        wifiState = "WIFI_STATE_CONNECTION_FAILED";
                        NetworkManagerEvents::onWIFIStateChanged(Exchange::INetworkManager::WIFI_STATE_CONNECTION_FAILED, wifiState);
                        break;
                    case NM_DEVICE_STATE_NEED_AUTH:
                        //NetworkManagerEvents::onWIFIStateChanged(Exchange::INetworkManager::WIFI_STATE_CONNECTION_INTERRUPTED);
                        //wifiState = "WIFI_STATE_CONNECTION_INTERRUPTED";
                        break;
                    default:
                        wifiState = "Un handiled";
                    }
                }
            }
        } 
        else if(ifname == GnomeUtils::getEthIfname())
        {
            switch (deviceState)
            {
                case NM_DEVICE_STATE_UNKNOWN:
                case NM_DEVICE_STATE_UNMANAGED:
                    NetworkManagerEvents::onInterfaceStateChangeCb(Exchange::INetworkManager::INTERFACE_DISABLED, "eth0");
                break;
                case NM_DEVICE_STATE_UNAVAILABLE:
                case NM_DEVICE_STATE_DISCONNECTED:
                    NetworkManagerEvents::onInterfaceStateChangeCb(Exchange::INetworkManager::INTERFACE_LINK_DOWN, "eth0");
                break;
                case NM_DEVICE_STATE_PREPARE:
                    NetworkManagerEvents::onInterfaceStateChangeCb(Exchange::INetworkManager::INTERFACE_LINK_UP, "eth0");
                break;
                case NM_DEVICE_STATE_IP_CONFIG:
                    NetworkManagerEvents::onInterfaceStateChangeCb(Exchange::INetworkManager::INTERFACE_ACQUIRING_IP,"eth0");
                case NM_DEVICE_STATE_NEED_AUTH:
                case NM_DEVICE_STATE_SECONDARIES:
                case NM_DEVICE_STATE_ACTIVATED:
                case NM_DEVICE_STATE_DEACTIVATING:
                default:
                    NMLOG_WARNING("Unhandiled state change eth0");
            }
        }
        
    }

    static void onConnectionSignalReceivedCB (GDBusProxy *proxy, 
                                        gchar *sender_name,
                                        gchar *signal_name,
                                        GVariant *parameters,
                                        gpointer user_data) {

        if (g_strcmp0(signal_name, "NewConnection") == 0) {
            NMLOG_INFO("new Connection added success");
            NMLOG_DEBUG("Parameters: %s", g_variant_print(parameters, TRUE));
        } else if (g_strcmp0(signal_name, "ConnectionRemoved") == 0) {
            NMLOG_INFO("connection remove success");
            NMLOG_DEBUG("Parameters: %s", g_variant_print(parameters, TRUE));
        }
    }

    static void addressChangeCb(GDBusProxy *proxy,
                      GVariant *changed_properties,
                      GStrv invalidated_properties,
                      gpointer user_data)
    {
        if (changed_properties == NULL) {
            NMLOG_FATAL("cb doesn't have changed_properties ");
            return;
        }
         GVariant *AddressDataVariant = g_variant_lookup_value(changed_properties, "AddressData", NULL);
         if (AddressDataVariant != NULL)
         {
        //     const gchar *primary_connection;
        //     primary_connection = g_variant_get_string(primary_connection_variant, NULL);
        //     if(primary_connection && g_strcmp0(primary_connection, "/") == 0)
        //         NMLOG_WARNING("no active primary connection");
        //     else if(primary_connection)
        //         NMLOG_INFO("PrimaryConnection changed: %s", primary_connection);
        //     else
        //         NMLOG_ERROR("PrimaryConnection changed Null Error");
         }
    }

    static void monitorDevice(const gchar *devicePath, GDBusProxy *deviceProxy) 
    {
        std::string ifname = getInterafaceNameFormDevicePath(devicePath);
        if(ifname.empty())
        {
            NMLOG_WARNING("interface is missing for the device(%s)", devicePath);
            return;
        }

        if(ifname!=GnomeUtils::getEthIfname() && ifname!=GnomeUtils::getWifiIfname())
        {
            NMLOG_WARNING("interface is not eth0/wlan0: %s", ifname.c_str());
            return;
        }

        deviceProxy =_NetworkManagerEvents->eventDbus.getNetworkManagerDeviceProxy(devicePath);
        g_signal_connect(deviceProxy, "g-signal", G_CALLBACK(deviceStateChangedCB), NULL);
        NMLOG_DEBUG("Monitoring device: %s", devicePath);
        if(ifname == GnomeUtils::getWifiIfname())
            subscribeForlastScanPropertyEvent(devicePath);

        const gchar *ipv4ConfigPath = NULL;
        const gchar *ipv6ConfigPath = NULL;
        GVariant *ip4_config = g_dbus_proxy_get_cached_property(deviceProxy, "Ip4Config");
        if (ip4_config) {
            ipv4ConfigPath = g_variant_get_string(ip4_config, NULL);
            NMLOG_DEBUG("Monitoring ip4_config_path: %s", ipv4ConfigPath);
        }

        GVariant *ip6_config = g_dbus_proxy_get_cached_property(deviceProxy, "Ip6Config");
        if (ip6_config) {
            ipv6ConfigPath = g_variant_get_string(ip6_config, NULL);
            NMLOG_DEBUG("Monitoring ip6_config_path: %s", ipv6ConfigPath);
        }

        if(ipv4ConfigPath)
        {
            GDBusProxy *ipV4Proxy = _NetworkManagerEvents->eventDbus.getNetworkManagerIpv4Proxy(ipv4ConfigPath);
            g_signal_connect(ipV4Proxy, "g-properties-changed", G_CALLBACK(addressChangeCb), NULL);
        }

        if(ipv6ConfigPath)
        {
            GDBusProxy *ipV6Proxy = _NetworkManagerEvents->eventDbus.getNetworkManagerIpv6Proxy(ipv6ConfigPath);
            g_signal_connect(ipV6Proxy, "g-properties-changed", G_CALLBACK(addressChangeCb), NULL);
        }
    }

    void* NetworkManagerEvents::networkMangerEventMonitor(void *arg)
    {
        if(arg == NULL)
        {
            NMLOG_FATAL("function argument error: nm event monitor failed");
            return nullptr;
        }

        NMEvents *nmEvents = static_cast<NMEvents *>(arg);
        GDBusProxy *nmProxy = _NetworkManagerEvents->eventDbus.getNetworkManagerProxy();
        if (nmProxy == NULL) {
            return nullptr;
        }

        g_signal_connect(nmProxy, "g-signal", G_CALLBACK(deviceAddRemoveCb), NULL);
        g_signal_connect(nmProxy, "g-properties-changed", G_CALLBACK(primaryConnectionChangedCB), NULL);

        GVariant *devices = g_dbus_proxy_get_cached_property(nmProxy, "Devices");
        if (devices != NULL) {
            GVariantIter iter;
            const gchar *devicePath;

            g_variant_iter_init(&iter, devices);
            while (g_variant_iter_loop(&iter, "&o", &devicePath)) {
                GDBusProxy *deviceProxy = nullptr;
                monitorDevice(devicePath, deviceProxy);
            }
            g_variant_unref(devices);
        }

        GDBusProxy *SettingsProxy = NULL;
        SettingsProxy = _NetworkManagerEvents->eventDbus.getNetworkManagerSettingsProxy();
        if (SettingsProxy == NULL) {
            NMLOG_WARNING("Settings proxy failed no connection event registerd");
        }
        else
            g_signal_connect(SettingsProxy, "g-signal", G_CALLBACK(onConnectionSignalReceivedCB), NULL);

        NMLOG_INFO("registered all networkmnager dbus events");
        g_main_loop_run(nmEvents->loop);
        g_object_unref(nmProxy);
        return nullptr;
    }

    bool NetworkManagerEvents::startNetworkMangerEventMonitor()
    {
        NMLOG_DEBUG("starting gnome event monitor...");

        if(!isEventThrdActive) {
            isEventThrdActive = true;
            eventThrdID = g_thread_new("nm_event_thrd", NetworkManagerEvents::networkMangerEventMonitor, &nmEvents);
        }
        return true;
    }

    void NetworkManagerEvents::stopNetworkMangerEventMonitor()
    {

        if (nmEvents.loop != NULL) {
            g_main_loop_quit(nmEvents.loop);
        }
        if (eventThrdID) {
            g_thread_join(eventThrdID);
            eventThrdID = NULL;
            NMLOG_WARNING("gnome event monitor stoped");
        }
        isEventThrdActive = false;
    }

    NetworkManagerEvents::~NetworkManagerEvents()
    {
        NMLOG_INFO("~NetworkManagerEvents");
        stopNetworkMangerEventMonitor();
    }

    NetworkManagerEvents* NetworkManagerEvents::getInstance()
    {
        static NetworkManagerEvents instance;
        return &instance;
    }

    NetworkManagerEvents::NetworkManagerEvents()
    {
        NMLOG_DEBUG("NetworkManagerEvents");
        nmEvents.loop = g_main_loop_new(NULL, FALSE);
        if(nmEvents.loop == NULL) {
            NMLOG_FATAL("GMain loop failed Fatal Error: Event will not work");
            return;
        }
        _NetworkManagerEvents = this;
        startNetworkMangerEventMonitor();
    }

    /* Gnome networkmanger events call backs */
    void NetworkManagerEvents::onActiveInterfaceChangeCb(std::string newIface)
    {
        static std::string oldIface = "Unknown";
        if(oldIface != newIface)
        {
            NMLOG_INFO("old interface - %s new interface - %s", oldIface.c_str(), newIface.c_str());
            oldIface = newIface;
        }
    }

    void NetworkManagerEvents::onInterfaceStateChangeCb(Exchange::INetworkManager::InterfaceState newState, std::string iface)
    {
        std::string state;
        switch (newState)
        {
            case Exchange::INetworkManager::INTERFACE_ADDED:
                state = "INTERFACE_ADDED";
                break;
            case Exchange::INetworkManager::INTERFACE_LINK_UP:
                state = "INTERFACE_LINK_UP";
                break;
            case Exchange::INetworkManager::INTERFACE_LINK_DOWN:
                state = "INTERFACE_LINK_DOWN";
                break;
            case Exchange::INetworkManager::INTERFACE_ACQUIRING_IP:
                state = "INTERFACE_ACQUIRING_IP";
                break;
            case Exchange::INetworkManager::INTERFACE_REMOVED:
                state = "INTERFACE_REMOVED";
                break;
            case Exchange::INetworkManager::INTERFACE_DISABLED:
                state = "INTERFACE_DISABLED";
                break;
            default:
                state = "Unknown";
        }
        NMLOG_DEBUG("%s interface state changed - %s", iface.c_str(), state.c_str());
    }

    void NetworkManagerEvents::onWIFIStateChanged(Exchange::INetworkManager::WiFiState state, std::string& wifiStateStr)
    {
        NMLOG_DEBUG("wifi state changed: %d ; NM wifi: %s", state, wifiStateStr.c_str());
    }

    void NetworkManagerEvents::onAddressChangeCb(std::string iface, std::string ipAddress, bool acqired, bool isIPv6)
    {
        NMLOG_INFO("iface:%s - ipaddress:%s - %s - isIPv6:%s", iface.c_str(), ipAddress.c_str(), acqired?"acquired":"lost", isIPv6?"true":"false");
    }

    void NetworkManagerEvents::onAvailableSSIDsCb(const char* wifiDevicePath)
    {
        if(_NetworkManagerEvents->doScanNotify == false) {
           return;
        }

        NMLOG_INFO("wifi scanning completed ...");
        //_NetworkManagerEvents->doScanNotify = false;
        GDBusProxy* wProxy = nullptr;
        GError* error = nullptr;
        wProxy = _NetworkManagerEvents->eventDbus.getNetworkManagerWirelessProxy(wifiDevicePath);

        if(wProxy == NULL)
            return;
        GVariant* result = g_dbus_proxy_call_sync(wProxy, "GetAllAccessPoints", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
        if (error) {
            NMLOG_ERROR("Error creating proxy: %s", error->message);
            g_error_free(error);
            g_object_unref(wProxy);
            return;
        }

        GVariantIter* iter;
        const gchar* apPath;
        JsonArray ssidList = JsonArray();
        bool oneSSIDFound = false;

        g_variant_get(result, "(ao)", &iter);
        while (g_variant_iter_loop(iter, "o", &apPath)) {
            Exchange::INetworkManager::WiFiSSIDInfo wifiInfo;
            // NMLOG_DEBUG("Access Point Path: %s", apPath);
            if(GnomeUtils::getApDetails(_NetworkManagerEvents->eventDbus.getConnection(), apPath, wifiInfo))
            {
                JsonObject ssidObj;
                if(GnomeUtils::convertSsidInfoToJsonObject(wifiInfo, ssidObj))
                {
                    ssidList.Add(ssidObj);
                    oneSSIDFound = true;
                }
            }

        }

        if(oneSSIDFound) {
            std::string ssids;
            ssidList.ToString(ssids);
            NMLOG_DEBUG("scanned ssids: %s", ssids.c_str());
        }

        g_variant_iter_free(iter);
        g_variant_unref(result);
        g_object_unref(wProxy);
    }

    void NetworkManagerEvents::setwifiScanOptions(bool doNotify)
    {
        doScanNotify.store(doNotify);
        if(!doScanNotify)
            NMLOG_DEBUG("stoped periodic wifi scan result notify");
    }

    }   // Plugin
}   // WPEFramework
