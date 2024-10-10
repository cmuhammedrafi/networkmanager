#include <gio/gio.h>
#include <stdio.h>
// gcc event_monitor.c -o get_interface_name `pkg-config --cflags --libs gio-2.0`
// Function to print individual GVariant properties based on their type


/*
 *
 *
 *      static void onInterfaceStateChangeCb(uint8_t newState, std::string iface); // ReportInterfaceStateChangedEvent
        static void onAddressChangeCb(std::string iface, std::string ipAddress, bool acqired, bool isIPv6); // ReportIPAddressChangedEvent
        static void onActiveInterfaceChangeCb(std::string newInterface); // ReportActiveInterfaceChangedEvent
        static void onAvailableSSIDsCb(NMDeviceWifi *wifiDevice, GParamSpec *pspec, gpointer userData); // ReportAvailableSSIDsEvent
        static void onWIFIStateChanged(uint8_t state); // ReportWiFiStateChangedEvent
*/

#define NMLOG_DEBUG(FMT, ...)   printf("[DEBUG] %s:%d: " FMT "\n", __func__, __LINE__, ##__VA_ARGS__)
#define NMLOG_INFO(FMT, ...)    printf("[INFO] %s:%d: " FMT "\n",  __func__, __LINE__, ##__VA_ARGS__)
#define NMLOG_WARNING(FMT, ...) printf("[WARNING] %s:%d: " FMT "\n",  __func__, __LINE__, ##__VA_ARGS__)
#define NMLOG_ERROR(FMT, ...)   printf("[ERROR] %s:%d: " FMT "\n",  __func__, __LINE__, ##__VA_ARGS__)
#define NMLOG_FATAL(FMT, ...)   printf("[FATAL] %s:%d: " FMT "\n",  __func__, __LINE__, ##__VA_ARGS__)

static char* getInterafaceNameFormDevicePath(const char* object_path)
{
    GError *error = NULL;
    GDBusProxy *device_proxy;
    device_proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                                    G_DBUS_PROXY_FLAGS_NONE,
                                                    NULL, /* GDBusInterfaceInfo */
                                                    "org.freedesktop.NetworkManager", /* bus name */
                                                    object_path, /* object path */
                                                    "org.freedesktop.NetworkManager.Device", /* interface */
                                                    NULL, /* GCancellable */
                                                    &error);

    if (error != NULL)
    {
        g_print("Error creating device proxy: %s\n", error->message);
        g_clear_error(&error);
        return "null";
    }

    GVariant *interface_property = g_dbus_proxy_get_cached_property(device_proxy, "Path");
    if (interface_property)
    {
        const gchar *interface_name = g_variant_get_string(interface_property, NULL);
        if(interface_name == NULL)
        {
            return "null1";
        }
        else if(g_strcmp0(interface_name, "eth0") == 0)
        {
            g_variant_unref(interface_property);
            g_object_unref(device_proxy);
            return "eth0";
        }
        else if (g_strcmp0(interface_name, "wlan0") == 0)
        {
            g_variant_unref(interface_property);
            g_object_unref(device_proxy);
            return "wlan0";
        }
        else
        {
            g_print("interface name %s", interface_name);
            g_variant_unref(interface_property);
            g_object_unref(device_proxy);
        }
    }

    //g_object_unref(device_proxy);
    return "null2";
}

static void deviceSignalCB(GDBusProxy *proxy,
                                        gchar *sender_name,
                                        gchar *signal_name,
                                        GVariant *parameters,
                                        gpointer user_data) {

    if (proxy == NULL) {
        NMLOG_FATAL("cb doesn't have proxy ");
        return;
    }

    if (g_strcmp0(signal_name, "DeviceAdded") != 0 && g_strcmp0(signal_name, "DeviceRemoved") != 0)
    {
       return;
    }

    const char* interface = "NULL  ";
    if (g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)) {
        GVariant *first_element = g_variant_get_child_value(parameters, 0);

        if (g_variant_is_of_type(first_element, G_VARIANT_TYPE_OBJECT_PATH)) {
            const gchar *object_path = g_variant_get_string(first_element, NULL);
            NMLOG_DEBUG("Extracted object path: %s", object_path);
            interface = getInterafaceNameFormDevicePath(object_path);
        } else {
            NMLOG_DEBUG("First element is not of type object path, actual type: %s", g_variant_get_type_string(first_element));
        }
        g_variant_unref(first_element);
    }

    if (g_strcmp0(signal_name, "DeviceAdded") == 0) {
        NMLOG_INFO("Device Added: %s", interface);
    }
    else if(g_strcmp0(signal_name, "DeviceRemoved"))
    {
        NMLOG_INFO("Device Removed: %s", interface);
    }
}

static void deviceStateChangedCB(GDBusProxy *proxy,
                                        gchar *sender_name,
                                        gchar *signal_name,
                                        GVariant *parameters,
                                        gpointer user_data) {

    //const gchar *device_path = g_dbus_proxy_get_object_path(proxy);
    char* ifname = "null";// getInterafaceNameFormDeviceProxy(proxy);
    if (g_strcmp0(signal_name, "StateChanged") == 0) {
        guint32 old_state, new_state, reason;
        g_variant_get(parameters, "(uuu)", &new_state, &old_state, &reason);
        NMLOG_INFO("%s: Old State: %u, New State: %u, Reason: %u", ifname, old_state, new_state, reason);
    }
}

static void primaryConnectionChanged(GDBusProxy *proxy,
                      GVariant *changed_properties,
                      GStrv invalidated_properties,
                      gpointer user_data)
{
    GVariant *primary_connection_variant = g_variant_lookup_value(changed_properties, "PrimaryConnection", NULL);
    if (primary_connection_variant != NULL)
    {
        // Process the new value of the 'PrimaryConnection' property
        const gchar *primary_connection;
        primary_connection = g_variant_get_string(primary_connection_variant, NULL);
        NMLOG_INFO("PrimaryConnection changed: %s", primary_connection);
    }
}

static void print_variant_value(const gchar *key, GVariant *value) {
    GVariantClass type = g_variant_classify(value);

    g_print("%s: ", key);  // Print the property name

    switch (type) {
        case G_VARIANT_CLASS_BOOLEAN: {
            gboolean boolean_value = g_variant_get_boolean(value);
            g_print("%s\n", boolean_value ? "true" : "false");
            break;
        }
        case G_VARIANT_CLASS_STRING: {
            const gchar *string_value = g_variant_get_string(value, NULL);
            g_print("%s\n", string_value);
            break;
        }
        case G_VARIANT_CLASS_UINT32: {
            guint32 uint_value = g_variant_get_uint32(value);
            g_print("%u\n", uint_value);
            break;
        }
        case G_VARIANT_CLASS_OBJECT_PATH: {
            const gchar *object_path = g_variant_get_string(value, NULL);
            g_print("%s\n", object_path);
            break;
        }
        case G_VARIANT_CLASS_VARIANT: {
            g_print("--\n");  // Print placeholder for complex variant values
            break;
        }
        case G_VARIANT_CLASS_ARRAY: {
            GVariantIter iter;
            GVariant *child;
            g_print("\n[\n");
            g_variant_iter_init(&iter, value);

            // For arrays, print each element
            while ((child = g_variant_iter_next_value(&iter))) {
                GVariantClass child_type = g_variant_classify(child);
                if (child_type == G_VARIANT_CLASS_OBJECT_PATH) {
                    g_print("%s \n", g_variant_get_string(child, NULL));
                } else if (child_type == G_VARIANT_CLASS_UINT32) {
                    g_print("%u \n", g_variant_get_uint32(child));
                } else if (child_type == G_VARIANT_CLASS_STRING) {
                    g_print("%s \n", g_variant_get_string(child, NULL));
                } else {
                    g_print("unsupported_value ");
                }
                g_variant_unref(child);
            }
            g_print("]\n");
            break;
        }
        default:
            g_print(" *** \n");
    }
}

// General function to print all properties in changed_properties
static void print_all_properties(GVariant *changed_properties) {
    GVariantIter *iter;
    gchar *key;
    GVariant *value;

    // Iterate over the dictionary of properties (a{sv})
    g_variant_get(changed_properties, "a{sv}", &iter);
    while (g_variant_iter_loop(iter, "{&sv}", &key, &value)) {
        print_variant_value(key, value);  // Print each key-value pair
    }
    g_variant_iter_free(iter);
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
    NMLOG_INFO("LastScan value changed: %" G_GINT64_FORMAT, lastScanTime);
    //print_all_properties(changed_properties);

}

static void on_settings_signal_received (GDBusProxy *proxy, 
                                        gchar *sender_name,
                                        gchar *signal_name,
                                        GVariant *parameters,
                                        gpointer user_data) {

    if (g_strcmp0(signal_name, "NewConnection") == 0) {
        g_print("NewConnection signal received\n");
        g_print("Parameters: %s\n", g_variant_print(parameters, TRUE));
    } else if (g_strcmp0(signal_name, "ConnectionRemoved") == 0) {
        g_print("ConnectionRemoved signal received\n");
        g_print("Parameters: %s\n", g_variant_print(parameters, TRUE));
    } else {
        g_print("Other signal from Settings: %s\n", signal_name);
    }
}

static void monitorDevice(const gchar *devicePath, GDBusProxy *deviceProxy) 
{
    GError *error = NULL;

    deviceProxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                                 G_DBUS_PROXY_FLAGS_NONE,
                                                 NULL,
                                                 "org.freedesktop.NetworkManager",
                                                 devicePath,
                                                 "org.freedesktop.NetworkManager.Device",
                                                 NULL,
                                                 &error);

    if (error != NULL) {
        NMLOG_FATAL("Error creating proxy for device %s: %s", devicePath, error->message);
        g_clear_error(&error);
        return;
    }

    GVariant *ifaceVariant = g_dbus_proxy_get_cached_property(deviceProxy, "Interface");
    if (ifaceVariant == NULL)
    {
        NMLOG_WARNING("interface property get error");
        return;
    }

    const gchar *ifaceName = g_variant_get_string(ifaceVariant, NULL);
    if(ifaceName && (g_strcmp0(ifaceName, "eth0") == 0 || g_strcmp0(ifaceName, "wlan0") == 0))
    {
        g_signal_connect(deviceProxy, "g-signal", G_CALLBACK(deviceStateChangedCB), NULL);
        NMLOG_DEBUG("Monitoring device: %s", devicePath);

        GVariant *ip4_config = g_dbus_proxy_get_cached_property(deviceProxy, "Ip4Config");
        if (ip4_config) {
            const gchar *ip4_config_path = g_variant_get_string(ip4_config, NULL);
            //monitor_ip4(ip4_config_path);
        }

        GVariant *ip6_config = g_dbus_proxy_get_cached_property(deviceProxy, "Ip6Config");
        if (ip6_config) {
            const gchar *ip6_config_path = g_variant_get_string(ip6_config, NULL);
            //monitor_ip6(ip6_config_path);
        }
    }
    else
    {
        NMLOG_WARNING("no monitoring interface not eth0/wlan0: %s", ifaceName);
    }
}

int main(int argc, char *argv[]) 
{
    GMainLoop *loop;
    GError *error = NULL;

   GDBusProxy* nmProxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                          G_DBUS_PROXY_FLAGS_NONE,
                                          NULL,
                                          "org.freedesktop.NetworkManager",
                                          "/org/freedesktop/NetworkManager",
                                          "org.freedesktop.NetworkManager",
                                          NULL,
                                          &error);

    if (nmProxy == NULL) {
        g_printerr("Failed to create proxy: %s\n", error->message);
        g_error_free(error);
        return 1;
    }

    g_signal_connect(nmProxy, "g-signal", G_CALLBACK(deviceSignalCB), NULL);

    GVariant *devices = g_dbus_proxy_get_cached_property(nmProxy, "Devices");
    if (devices != NULL) {
        GVariantIter iter;
        const gchar *devicePath;

        g_variant_iter_init(&iter, devices);
        while (g_variant_iter_loop(&iter, "&o", &devicePath)) {
            GDBusProxy *deviceProxy;
            monitorDevice(devicePath, deviceProxy);
        }
        g_variant_unref(devices);
    }

    GDBusProxy *SettingsProxy = NULL;
    SettingsProxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                                   G_DBUS_PROXY_FLAGS_NONE,
                                                   NULL,
                                                   "org.freedesktop.NetworkManager",
                                                   "/org/freedesktop/NetworkManager/Settings",
                                                   "org.freedesktop.NetworkManager.Settings",
                                                   NULL,
                                                   &error);
    if (SettingsProxy == NULL) {
        g_printerr("Failed to create Settings proxy: %s\n", error->message);
        g_error_free(error);
        return 1;
    }

    g_signal_connect(SettingsProxy, "g-signal", G_CALLBACK(on_settings_signal_received), NULL);
    GDBusProxy *nmPropertyproxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                             G_DBUS_PROXY_FLAGS_NONE,
                                             NULL,
                                             "org.freedesktop.NetworkManager", 
                                             "/org/freedesktop/NetworkManager",
                                             "org.freedesktop.NetworkManager",
                                             NULL,
                                             &error);

    if (nmPropertyproxy == NULL) {
        g_printerr("Failed to create NetworkManager proxy: %s\n", error->message);
        g_error_free(error);
        return 1;
    }

    g_signal_connect(nmPropertyproxy, "g-properties-changed", G_CALLBACK(primaryConnectionChanged), NULL);


    GDBusProxy *wirelessProxy = NULL;
    const gchar *wireless_path = "/org/freedesktop/NetworkManager/Devices/3";
    wirelessProxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                          G_DBUS_PROXY_FLAGS_NONE,
                                          NULL,
                                          "org.freedesktop.NetworkManager", /* Bus name */
                                          wireless_path, /* Object path */
                                          "org.freedesktop.NetworkManager.Device.Wireless", /* Interface name */
                                          NULL,
                                          &error);

    if (error != NULL) {
        g_print("Error creating proxy: %s\n", error->message);
        g_clear_error(&error);
        return 1;
    }

    g_signal_connect(wirelessProxy, "g-properties-changed", G_CALLBACK(lastScanPropertiesChangedCB), NULL);

    loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);

    // Clean up
    g_object_unref(nmProxy);
    g_object_unref(SettingsProxy);
    g_object_unref(nmPropertyproxy);
    g_object_unref(wirelessProxy);

    g_main_loop_unref(loop);

    return 0;
}
