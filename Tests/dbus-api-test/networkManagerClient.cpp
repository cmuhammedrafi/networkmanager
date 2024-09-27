#include "networkManagerClient.h"
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
                NMLOG_TRACE("BSSID: %s", bssid.c_str());
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

static bool fetchSSIDFromConnection(const char *path, std::list<std::string>& ssids)
{
    GError *error = NULL;
    GDBusProxy *ConnProxy = NULL;
    GVariant *settingsProxy= NULL, *connection= NULL, *s_con= NULL;
    bool isFound;
    const char *connId= NULL, *connTyp= NULL, *iface= NULL;
    ConnProxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                    G_DBUS_PROXY_FLAGS_NONE,
                                    NULL,
                                    "org.freedesktop.NetworkManager",
                                    path,
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
    s_con = g_variant_lookup_value(connection, "connection", NULL);
    g_assert(s_con != NULL);
    isFound = g_variant_lookup(s_con, "type", "&s", &connTyp);
    g_assert(isFound);
    isFound = g_variant_lookup(s_con, "interface-name", "&s", &iface);
    g_assert(isFound);

    // NMLOG_TRACE("type= %s, iface= %s", connTyp, iface);
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

    if (s_con)
        g_variant_unref(s_con);
    if (connection)
        g_variant_unref(connection);
    if (settingsProxy)
        g_variant_unref(settingsProxy);
    g_object_unref(ConnProxy);

    return true;
}

static bool fetchConnectionPaths(GDBusProxy *proxy, std::list<std::string>& pathsList)
{
    GVariant *listProxy = NULL;
    GError *error = NULL;
    char **paths = NULL;
    listProxy = g_dbus_proxy_call_sync(proxy,
                                 "ListConnections",
                                 NULL,
                                 G_DBUS_CALL_FLAGS_NONE,
                                 -1,
                                 NULL,
                                 &error);
    if(listProxy == NULL)
    {
        if (!error) {
            NMLOG_ERROR("ListConnections failed: %s", error->message);
            g_error_free(error);
            return false;
        }
        else
            NMLOG_ERROR("ListConnections proxy failed no error message");
    }

    g_variant_get(listProxy, "(^ao)", &paths);
    g_variant_unref(listProxy);

    if(paths == NULL)
    {
        NMLOG_WARNING("no connection path available");
        return false;
    }

    for (int i = 0; paths[i]; i++) {
        //NMLOG_INFO("Connection path: %s", paths[i]);
        pathsList.push_back(paths[i]);
    }

    if(pathsList.empty())
        return false;

    return true;
}

bool NetworkManagerClient::getKnownSSIDs(std::list<std::string>& ssids)
{
    GDBusProxy *sProxy= NULL;
    GError *error= NULL;
    std::list<std::string> paths;

    sProxy = g_dbus_proxy_new_sync(dbusConnection.getConnection(),
                              G_DBUS_PROXY_FLAGS_NONE,
                              NULL,
                              "org.freedesktop.NetworkManager",
                              "/org/freedesktop/NetworkManager/Settings",
                              "org.freedesktop.NetworkManager.Settings",
                              NULL, // GCancellable
                              &error);

    if(sProxy == NULL)
    {
        if (error != NULL) {
            NMLOG_ERROR("Failed to create proxy: %s", error->message);
            g_error_free(error);
        }
        return false;
    }

    if(fetchConnectionPaths(sProxy, paths))
    {
        for (const std::string& path : paths) {
            NMLOG_TRACE("connection path %s", path.c_str());
            fetchSSIDFromConnection(path.c_str(), ssids);
        }
    }

    g_object_unref(sProxy);

    if(ssids.empty())
         return false;
    return true;
}
