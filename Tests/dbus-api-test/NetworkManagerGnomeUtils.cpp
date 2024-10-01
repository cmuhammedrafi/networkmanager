
#include <glib.h>
#include <gio/gio.h>
#include <stdio.h>
#include <string>

#include "NetworkManagerGnomeUtils.h"

// Function to get device path by network interface name
bool GnomeUtils::getDeviceByIpIface(GDBusConnection *connection, const gchar *iface_name, std::string& path)
{
    GError *error = NULL;
    GVariant *result;
    gchar *device_path = NULL;
    bool ret = false;

    // // Connect to the system bus
    // connection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
    // if (error != NULL) {
    //     g_printerr("Error connecting to system bus: %s\n", error->message);
    //     g_error_free(error);
    //     return NULL;
    // }

    result = g_dbus_connection_call_sync(
        connection,
        "org.freedesktop.NetworkManager",               // D-Bus name
        "/org/freedesktop/NetworkManager",              // Object path
        "org.freedesktop.NetworkManager",               // Interface
        "GetDeviceByIpIface",                           // Method name
        g_variant_new("(s)", iface_name),               // Input parameter (the interface name)
        G_VARIANT_TYPE("(o)"),                          // Expected return type (object path)
        G_DBUS_CALL_FLAGS_NONE,
        -1,                                             // Default timeout
        NULL,
        &error
    );

    if (error != NULL) {
        NMLOG_ERROR("calling GetDeviceByIpIface: %s", error->message);
        g_error_free(error);
        return ret;
    }

    g_variant_get(result, "(o)", &device_path);
    if(g_strdup(device_path) != NULL)
    {
        path = std::string(g_strdup(device_path));
        ret = true;
    }

    g_variant_unref(result);
    return ret;
}

static bool get_cached_property_u(GDBusProxy* proxy, const char* propertiy, uint32_t value)
{
    GVariant* result = NULL;
    result = g_dbus_proxy_get_cached_property(proxy, propertiy);
    if (!result) {
        NMLOG_ERROR("Failed to get AP properties");
        g_object_unref(proxy);
        return false;
    }
    g_variant_get(result, "u", &value);
    NMLOG_TRACE("%s: %d", propertiy, value);
    g_variant_unref(result);
    return true;
}

/*
Flags       readable   u
WpaFlags    readable   u
RsnFlags    readable   u
Ssid        readable   ay
Frequency   readable   u
HwAddress   readable   s
Mode        readable   u
MaxBitrate  readable   u
Bandwidth   readable   u
Strength    readable   y
LastSeen    readable   i
*/
bool GnomeUtils::getApDetails(GDBusConnection *connection, const char* apPath, apProperties& apDetails)
{
    char *bssid = NULL;
    uint8_t strength = 0;
    GError* error = NULL;
    GVariant* result = NULL;

    GDBusProxy* proxy  = g_dbus_proxy_new_sync(connection,
                        G_DBUS_PROXY_FLAGS_NONE,
                        NULL,
                        "org.freedesktop.NetworkManager",
                        apPath,
                        "org.freedesktop.NetworkManager.AccessPoint",
                        NULL,
                        &error);

    if (error) {
        NMLOG_ERROR("creating proxy: %s", error->message);
        g_error_free(error);
        return false;
    }

    gsize ssid_length = 0;
    result = g_dbus_proxy_get_cached_property(proxy,"Ssid");
    if (!result) {
        std::cerr << "Failed to get AP properties." << std::endl;
        g_object_unref(proxy);
        return false;
    }
    const guchar *ssid_data = static_cast<const guchar*>(g_variant_get_fixed_array(result, &ssid_length, sizeof(guchar)));
    if (ssid_data && ssid_length > 0 && ssid_length <= 32) {
        apDetails.ssid.assign(reinterpret_cast<const char*>(ssid_data), ssid_length);
        NMLOG_TRACE("SSID: %s", apDetails.ssid.c_str());
    } else {
        NMLOG_ERROR("Invalid SSID length: %zu (maximum is 32)", ssid_length);
        apDetails.ssid="---";
    }
    g_variant_unref(result);

    result = g_dbus_proxy_get_cached_property(proxy,"HwAddress");
    if (!result) {
        std::cerr << "Failed to get AP properties." << std::endl;
        g_object_unref(proxy);
        return false;
    }
    g_variant_get(result, "s", &bssid);
    apDetails.bssid.assign(bssid);
    NMLOG_TRACE("bssid %s", apDetails.bssid.c_str());
    g_variant_unref(result);

    result = g_dbus_proxy_get_cached_property(proxy,"Strength");
    if (!result) {
        std::cerr << "Failed to get AP properties." << std::endl;
        g_object_unref(proxy);
        return false;
    }
    g_variant_get(result, "y", &strength);
    NMLOG_TRACE("strength %d", strength);
    g_variant_unref(result);

    get_cached_property_u(proxy, "Flags", apDetails.flags);
    get_cached_property_u(proxy, "WpaFlags", apDetails.wpaFlags);
    get_cached_property_u(proxy, "RsnFlags", apDetails.rsnFlags);
    get_cached_property_u(proxy, "Mode", apDetails.mode);
    get_cached_property_u(proxy, "Frequency", apDetails.frequency);

    g_object_unref(proxy);

    return true;
}
