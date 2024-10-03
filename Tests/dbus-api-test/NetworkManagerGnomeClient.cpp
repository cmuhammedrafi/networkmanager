#include "NetworkManagerGnomeClient.h"
#include "dbusConnectionManager.h"

NetworkManagerClient::NetworkManagerClient() {
    NMLOG_INFO("NetworkManagerClient created");
    if (!dbusConnection.InitializeBusConnection("org.freedesktop.NetworkManager")) {
        NMLOG_ERROR("Failed to initialize D-Bus connection for NetworkManager");
    }
}

NetworkManagerClient::~NetworkManagerClient() {
    NMLOG_INFO("NetworkManagerClient destroyed");
}

static void printKeyVariant(const char *setting_name, GVariant *setting)
{
    GVariantIter iter;
    const char  *property_name;
    GVariant    *value;
    char        *printed_value;

    NMLOG_VERBOSE("  %s:", setting_name);
    g_variant_iter_init(&iter, setting);
    while (g_variant_iter_next(&iter, "{&sv}", &property_name, &value)) {
        printed_value = g_variant_print(value, FALSE);
        if (strcmp(printed_value, "[]") != 0)
            NMLOG_VERBOSE("    %s: %s", property_name, printed_value);
        g_free(printed_value);
        g_variant_unref(value);
    }
}

static void fetssidandbssid(GVariant *setting80211, std::string &ssid, std::string &bssid)
{
    GVariantIter iter;
    const char  *property_name;
    GVariant *value;
    char *printed_value;

    g_variant_iter_init(&iter, setting80211);
    while (g_variant_iter_next(&iter, "{&sv}", &property_name, &value)) {

        printed_value = g_variant_print(value, FALSE);
        if (strcmp(property_name, "seen-bssids") == 0) {
            bssid.clear();
            GVariantIter bssid_iter;
            g_variant_iter_init(&bssid_iter, value);
            gchar *bssid_elem;

            if (g_variant_iter_next(&bssid_iter, "s", &bssid_elem)) {
                bssid = bssid_elem;
                NMLOG_VERBOSE("BSSID: %s", bssid.c_str());
            }

            //g_variant_iter_free(&bssid_iter);
        }
        else if (strcmp(property_name, "ssid") == 0) {
            // Decode SSID from GVariant of type "ay"
            gsize ssid_length = 0;
            const guchar *ssid_data = static_cast<const guchar*>(g_variant_get_fixed_array(value, &ssid_length, sizeof(guchar)));
            if (ssid_data && ssid_length > 0 && ssid_length <= 32) {
                ssid.assign(reinterpret_cast<const char*>(ssid_data), ssid_length);
                NMLOG_INFO("SSID: %s", ssid.c_str());
            } else {
                NMLOG_ERROR("Invalid SSID length: %zu (maximum is 32)", ssid_length);
                ssid.clear();
            }
        }
    }

    g_free(printed_value);
    g_variant_unref(value);
}

static bool fetchSSIDFromConnection(GDBusConnection *dbusConn, const std::string& path, std::list<std::string>& ssids)
{
    GError *error = NULL;
    GDBusProxy *ConnProxy = NULL;
    GVariant *settingsProxy= NULL, *connection= NULL, *sCon= NULL;
    bool isFound;
    const char *connId= NULL, *connTyp= NULL, *iface= NULL;

    ConnProxy = g_dbus_proxy_new_sync(dbusConn,
                        G_DBUS_PROXY_FLAGS_NONE,
                        NULL,
                        "org.freedesktop.NetworkManager",
                        path.c_str(),
                        "org.freedesktop.NetworkManager.Settings.Connection",
                        NULL,
                        &error);

    if (error != NULL) {
        NMLOG_ERROR("Failed to create proxy: %s", error->message);
        g_error_free(error);
        return false;
    }

    settingsProxy = g_dbus_proxy_call_sync(ConnProxy,
                            "GetSettings",
                            NULL,
                            G_DBUS_CALL_FLAGS_NONE,
                            -1,
                            NULL,
                            &error);
    if (!settingsProxy) {
        g_dbus_error_strip_remote_error(error);
        NMLOG_WARNING("Failed to get connection settings: %s", error->message);
        g_error_free(error);
    }

    g_variant_get(settingsProxy, "(@a{sa{sv}})", &connection);
    sCon = g_variant_lookup_value(connection, "connection", NULL);
    g_assert(sCon != NULL); // TODO change error return
    G_VARIANT_LOOKUP(sCon, "type", "&s", &connTyp);
    G_VARIANT_LOOKUP(sCon, "interface-name", "&s", &iface);
    NMLOG_VERBOSE("type= %s, iface= %s", connTyp, iface);
    if(strcmp(connTyp,"802-11-wireless") == 0 && strcmp(iface,"wlan0") == 0)
    {
        GVariant *setting = g_variant_lookup_value(connection, "802-11-wireless", NULL);
        if (setting) {
            std::string ssid, bssid;
            fetssidandbssid(setting, ssid, bssid);
            if(!ssid.empty())
                ssids.push_back(ssid);
            g_variant_unref(setting);
        }
    }

    if (sCon)
        g_variant_unref(sCon);
    if (connection)
        g_variant_unref(connection);
    if (settingsProxy)
        g_variant_unref(settingsProxy);
    g_object_unref(ConnProxy);

    return true;
}

bool NetworkManagerClient::getKnownSSIDs(std::list<std::string>& ssids)
{
    std::list<std::string> paths;
    if(!GnomeUtils::getConnectionPaths(dbusConnection.getConnection(), paths))
    {
        NMLOG_ERROR("Connection path fetch failed");
        return false;
    }

    for (const std::string& path : paths) {
        NMLOG_VERBOSE("connection path %s", path.c_str());
        fetchSSIDFromConnection(dbusConnection.getConnection(), path, ssids);
    }

    if(ssids.empty())
         return false;
    return true;
}

bool NetworkManagerClient::getConnectedSSID()
{
    std::string wifiDevicePath;
    GError* error = NULL;
    GDBusProxy* wProxy = NULL;
    GVariant* result = NULL;
    gchar *activeApPath = NULL;
    bool ret = false;
    NMDeviceState state;

    if(GnomeUtils::getDeviceState(dbusConnection.getConnection(), "wlan0", state) && state < NM_DEVICE_STATE_DISCONNECTED)
    {
        NMDeviceStateReason StateReason;
        GnomeUtils::getDeviceStateReason(dbusConnection.getConnection(), "wlan0", StateReason);
        NMLOG_ERROR("wifi device state is invallied");
        return false;
    }

    if(!GnomeUtils::getDeviceByIpIface(dbusConnection.getConnection(),"wlan0", wifiDevicePath))
    {
        NMLOG_ERROR("no wifi device found");
        return false;
    }

    //TODO check active connection path and return properties
    wProxy = g_dbus_proxy_new_sync(dbusConnection.getConnection(),
                            G_DBUS_PROXY_FLAGS_NONE,
                            NULL,
                            "org.freedesktop.NetworkManager",
                            wifiDevicePath.c_str(),
                            "org.freedesktop.NetworkManager.Device.Wireless",
                            NULL, // GCancellable
                            &error);

    if (error) {
        NMLOG_ERROR("Error creating proxy: %s", error->message);
        g_error_free(error);
        return false;
    }

    result = g_dbus_proxy_get_cached_property(wProxy, "ActiveAccessPoint");
    if (!result) {
        NMLOG_ERROR("Failed to get ActiveAccessPoint property.");
        g_object_unref(wProxy);
        return false;
    }

    g_variant_get(result, "o", &activeApPath);
    if(g_strdup(activeApPath) != NULL && g_strcmp0(activeApPath, "/") != 0)
    {
        NMLOG_TRACE("ActiveAccessPoint property path %s", activeApPath);
        apProperties apDetails;
        if(GnomeUtils::getApDetails(dbusConnection.getConnection(), g_strdup(activeApPath), apDetails))
        {
            NMLOG_INFO("getApDetails success");
            ret = true;
        }
    }
    else
        NMLOG_ERROR("active access point not found");

    g_variant_unref(result);
    g_object_unref(wProxy);
    return ret;
}

bool NetworkManagerClient::getavilableSSID(std::list<std::string>& ssids)
{
    // TODO Wifi device state fix it
    GError* error = nullptr;
    GDBusProxy* wProxy = nullptr;
    std::string wifiDevicePath;

    if(!GnomeUtils::getDeviceByIpIface(dbusConnection.getConnection(),"wlan0", wifiDevicePath))
    {
        NMLOG_ERROR("no wifi device found");
        return false;
    }

    wProxy = g_dbus_proxy_new_sync(dbusConnection.getConnection(),
                            G_DBUS_PROXY_FLAGS_NONE,
                            NULL,
                            "org.freedesktop.NetworkManager",
                            wifiDevicePath.c_str(),
                            "org.freedesktop.NetworkManager.Device.Wireless",
                            NULL,
                            &error);

    if (error) {
        NMLOG_ERROR("Error creating proxy: %s", error->message);
        g_error_free(error);
        return false;
    }

    GVariant* result = g_dbus_proxy_call_sync(wProxy, "GetAccessPoints", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
    // TODO change to GetAllAccessPoints to include hidden ssid 
    if (error) {
        NMLOG_ERROR("Error creating proxy: %s", error->message);
        g_error_free(error);
        g_object_unref(wProxy);
        return false;
    }

    GVariantIter* iter;
    const gchar* apPath;
    g_variant_get(result, "(ao)", &iter);

    while (g_variant_iter_loop(iter, "o", &apPath)) {
        apProperties apDetails;
        NMLOG_VERBOSE("Access Point Path: %s", apPath);
        if(!GnomeUtils::getApDetails(dbusConnection.getConnection(), apPath, apDetails))
        {
            NMLOG_WARNING("getApDetails failed");
        }
    }

    g_variant_iter_free(iter);
    g_variant_unref(result);
    g_object_unref(wProxy);

    return true;
}

bool NetworkManagerClient::startWifiScanning(const std::string& ssid)
{
    GError* error = NULL;
    GDBusProxy* wProxy = NULL;
    std::string wifiDevicePath;
    if(!GnomeUtils::getDeviceByIpIface(dbusConnection.getConnection(),"wlan0", wifiDevicePath))
    {
        NMLOG_ERROR("no wifi device found");
        return false;
    }

    wProxy = g_dbus_proxy_new_sync(dbusConnection.getConnection(),
                        G_DBUS_PROXY_FLAGS_NONE,
                        NULL,
                        "org.freedesktop.NetworkManager",
                        wifiDevicePath.c_str(),
                        "org.freedesktop.NetworkManager.Device.Wireless",
                        NULL,
                        &error);

    if (error) {
        NMLOG_ERROR("Error creating proxy: %s", error->message);
        g_error_free(error);
        return false;
    }

    GVariant *options = NULL;
    if (!ssid.empty()) {
        NMLOG_INFO("staring wifi scanning .. %s", ssid.c_str());
        GVariantBuilder builder, array_builder;
        g_variant_builder_init(&builder, G_VARIANT_TYPE_VARDICT);
        g_variant_builder_init(&array_builder, G_VARIANT_TYPE("aay"));
        g_variant_builder_add(&array_builder, "@ay",
                            g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE, (const guint8 *) ssidReq.c_str(), ssidReq.length(), 1)
                            );
        g_variant_builder_add(&builder, "{sv}", "ssids", g_variant_builder_end(&array_builder));
        g_variant_builder_add(&builder, "{sv}", "hidden", g_variant_new_boolean(TRUE));
        options = g_variant_builder_end(&builder);
    }

    g_dbus_proxy_call_sync(wProxy, "RequestScan", g_variant_new("(a{sv})", options), G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
    if (error) {
        NMLOG_ERROR("Error RequestScan: %s", error->message);
        g_error_free(error);
        g_object_unref(wProxy);
        return false;
    }

    if(options)
        g_variant_unref(options);
    g_object_unref(wProxy);

     return true;
 }
