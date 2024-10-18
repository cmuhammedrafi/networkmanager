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

#include "NetworkManagerGnomeEvent.h"
#include "NetworkManagerLogger.h"
#include "NetworkManagerGnomeUtils.h"
#include "INetworkManager.h"

namespace WPEFramework
{
    namespace Plugin
    {

    static NetworkManagerEvents *_NetworkManagerEvents = nullptr;

    static const char * nm_state_to_string(NMState state)
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

    static void on_signal(GDBusProxy *proxy,
            char       *sender_name,
            char       *signal_name,
            GVariant   *parameters,
            gpointer    user_data)
    {
        guint32 new_state;

        /* Print all signals */
        char *parameters_str;
        parameters_str = g_variant_print (parameters, TRUE);
        g_print (" *** Received Signal: %s: %s\n", signal_name, parameters_str);
        g_free (parameters_str);

        /* We are only interested in "StateChanged" signal */
        if (strcmp(signal_name, "StateChanged") == 0) {
            GVariant *tmp = g_variant_get_child_value(parameters, 0);
            new_state     = g_variant_get_uint32(tmp);
            g_variant_unref(tmp);
            g_print("NetworkManager state is: (%d) %s\n",
                    new_state,
                    nm_state_to_string((NMState) new_state));
        }
    }


    static void on_property_changed(GDBusConnection *connection, 
                                const gchar *sender_name, 
                                const gchar *signal_name, 
                                GVariant *parameters, 
                                gpointer user_data) {
        const gchar *property_name;
        GVariant *value;

        // Unpack the parameters
        g_variant_get(parameters, "(sv)", &property_name, &value);

        // Print the property change
        g_print("Property changed: %s\n", property_name);

        if (g_strcmp0(property_name, "State") == 0) {
            // If the property is 'State', print its new value
            guint32 state;
            g_variant_get(value, "u", &state);
            g_print("New State: %u\n", state);
        }

        g_variant_unref(value);
    }


    void* NetworkManagerEvents::networkMangerEventMonitor(void *arg)
    {
        if(arg == NULL)
        {
            NMLOG_FATAL("function argument error: nm event monitor failed");
            return nullptr;
        }

        NMEvents *nmEvents = static_cast<NMEvents *>(arg);
        GDBusProxy *proxy = _NetworkManagerEvents->eventDbus.getNetworkManagerProxy();
        if (proxy == NULL) {
            return nullptr;
        }

        g_signal_connect(proxy, "notify::DeviceAdded", G_CALLBACK(on_signal), NULL);
        //g_signal_connect(_NetworkManagerEvents->eventDbus.getConnection(), "signal", G_CALLBACK(on_property_changed), NULL);

       // g_bus_add_match(_NetworkManagerEvents->eventDbus.getConnection(), 
         //           "type='signal',interface='org.freedesktop.NetworkManager',member='PropertiesChanged'",
          //          NULL);
        // g_signal_connect (nmEvents->client, "notify::" NM_CLIENT_NM_RUNNING,G_CALLBACK (managerRunningCb), nmEvents);
         //g_signal_connect(nmEvents->client, "notify::" NM_CLIENT_STATE, G_CALLBACK (clientStateChangedCb),nmEvents);
        // g_signal_connect(nmEvents->client, "notify::" NM_CLIENT_PRIMARY_CONNECTION, G_CALLBACK(primaryConnectionCb), nmEvents);

        NMLOG_INFO("registered all networkmnager dbus events");
        g_main_loop_run(nmEvents->loop);
        g_object_unref(proxy);
        return nullptr;
    }

    bool NetworkManagerEvents::startNetworkMangerEventMonitor()
    {
        NMLOG_DEBUG("starting gnome event monitor... ?");

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

    void NetworkManagerEvents::onInterfaceStateChangeCb(uint8_t newState, std::string iface)
    {
        std::string state = "";
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

    void NetworkManagerEvents::onWIFIStateChanged(uint8_t state)
    {
        NMLOG_DEBUG("onWIFIStateChanged state changed - %d", state);
    }

    void NetworkManagerEvents::onAddressChangeCb(std::string iface, std::string ipAddress, bool acqired, bool isIPv6)
    {
        NMLOG_INFO("iface:%s - ipaddress:%s - %s - isIPv6:%s", iface.c_str(), ipAddress.c_str(), acqired?"acquired":"lost", isIPv6?"true":"false");
    }

    void NetworkManagerEvents::onAvailableSSIDsCb(void *wifiDevice, GParamSpec *pspec, gpointer userData)
    {
        NMLOG_DEBUG("wifi scanning completed ...");

        if(_NetworkManagerEvents->doScanNotify) {
            _NetworkManagerEvents->doScanNotify = false;
        }
    }

    void NetworkManagerEvents::setwifiScanOptions(bool doNotify)
    {
        doScanNotify.store(doNotify);
        if(!doScanNotify)
            NMLOG_DEBUG("stoped periodic wifi scan result notify");
    }

    }   // Plugin
}   // WPEFramework
