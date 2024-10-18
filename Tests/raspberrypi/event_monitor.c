#include <gio/gio.h>
#include <stdio.h>

// Function to print individual GVariant properties based on their type
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


static void on_wireless_properties_changed(GDBusProxy *proxy,
                      GVariant    *changed_properties,
                      const gchar *invalidated_properties[],
                      gpointer     user_data) {

    g_print("wireless properties: \n");
    print_all_properties(changed_properties);
}

static void on_networkmanager_properties_changed(GDBusProxy *proxy,
                      GVariant    *changed_properties,
                      const gchar *invalidated_properties[],
                      gpointer     user_data) {


    g_print("networkmanager:\n");

    print_all_properties(changed_properties);
}

static void on_device_added(GDBusProxy *proxy, 
                                        gchar *sender_name,
                                        gchar *signal_name,
                                        GVariant *parameters,
                                        gpointer user_data) {
    g_print("Device Added signal received\n");
    // Parse and handle parameters (device information)
    g_print("Parameters: %s\n", g_variant_print(parameters, TRUE));
}

static void on_device_removed(GDBusProxy *proxy, 
                                        gchar *sender_name,
                                        gchar *signal_name,
                                        GVariant *parameters,
                                        gpointer user_data) {
    g_print("Device Removed signal received\n");
    // Parse and handle parameters (device information)
    g_print("Parameters: %s\n", g_variant_print(parameters, TRUE));
}

static void on_device_state_changed(GDBusProxy *proxy,
                                        gchar *sender_name,
                                        gchar *signal_name,
                                        GVariant *parameters,
                                        gpointer user_data) {

    const gchar *device_path = g_dbus_proxy_get_object_path(proxy);

    if (g_strcmp0(signal_name, "StateChanged") == 0) {
        guint32 old_state, new_state, reason;
        g_variant_get(parameters, "(uuu)", &new_state, &old_state, &reason);

        g_print("StateChanged signal received %s\n", device_path);
        g_print("Old State: %u, New State: %u, Reason: %u\n", old_state, new_state, reason);
    } else {
        g_print("Other signal received: %s\n", signal_name);
    }
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

static void monitor_device(const gchar *device_path) 
{
    GDBusProxy *device_proxy;
    GError *error = NULL;

    // Create a proxy for the device
    device_proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                                 G_DBUS_PROXY_FLAGS_NONE,
                                                 NULL,
                                                 "org.freedesktop.NetworkManager", /* Bus name */
                                                 device_path, /* Object path */
                                                 "org.freedesktop.NetworkManager.Device", /* Interface name */
                                                 NULL,
                                                 &error);

    if (error != NULL) {
        g_print("Error creating proxy for device %s: %s\n", device_path, error->message);
        g_clear_error(&error);
        return;
    }

    g_signal_connect(device_proxy, "g-signal", G_CALLBACK(on_device_state_changed), NULL);
    g_print("Monitoring device: %s\n", device_path);

    GVariant *ip4_config = g_dbus_proxy_get_cached_property(device_proxy, "Ip4Config");
    if (ip4_config) {
        const gchar *ip4_config_path = g_variant_get_string(ip4_config, NULL);
        //monitor_ip4(ip4_config_path);
    }

    GVariant *ip6_config = g_dbus_proxy_get_cached_property(device_proxy, "Ip6Config");
    if (ip6_config) {
        const gchar *ip6_config_path = g_variant_get_string(ip6_config, NULL);
        //monitor_ip6(ip6_config_path);
    }
}

int main(int argc, char *argv[]) 
{
    GMainLoop *loop;
    GError *error = NULL;

   GDBusProxy* nmProxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                          G_DBUS_PROXY_FLAGS_NONE,
                                          NULL, /* GDBusInterfaceInfo */
                                          "org.freedesktop.NetworkManager", /* Bus name */
                                          "/org/freedesktop/NetworkManager", /* Object path */
                                          "org.freedesktop.NetworkManager", /* Interface name */
                                          NULL, /* GCancellable */
                                          &error);

    if (nmProxy == NULL) {
        g_printerr("Failed to create proxy: %s\n", error->message);
        g_error_free(error);
        return 1;
    }

    g_signal_connect(nmProxy, "g-signal", G_CALLBACK(on_device_added), NULL);
    g_signal_connect(nmProxy, "g-signal", G_CALLBACK(on_device_removed), NULL);

    GVariant *devices = g_dbus_proxy_get_cached_property(nmProxy, "Devices");
    if (devices != NULL) {
        GVariantIter iter;
        const gchar *device_path;

        g_variant_iter_init(&iter, devices);
        while (g_variant_iter_loop(&iter, "&o", &device_path)) {
            monitor_device(device_path);
        }
        g_variant_unref(devices);
    }

    GDBusProxy *SettingsProxy = NULL;
    SettingsProxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                                   G_DBUS_PROXY_FLAGS_NONE,
                                                   NULL,
                                                   "org.freedesktop.NetworkManager", /* Bus name */
                                                   "/org/freedesktop/NetworkManager/Settings", /* Object path */
                                                   "org.freedesktop.NetworkManager.Settings", /* Interface name */
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
                                             "org.freedesktop.NetworkManager", /* Bus name */
                                             "/org/freedesktop/NetworkManager", /* Object path */
                                             "org.freedesktop.NetworkManager", /* Interface name */
                                             NULL,
                                             &error);

    if (nmPropertyproxy == NULL) {
        g_printerr("Failed to create NetworkManager proxy: %s\n", error->message);
        g_error_free(error);
        return 1;
    }

    g_signal_connect(nmPropertyproxy, "g-properties-changed", G_CALLBACK(on_networkmanager_properties_changed), NULL);


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

    g_signal_connect(wirelessProxy, "g-properties-changed", G_CALLBACK(on_wireless_properties_changed), NULL);

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
