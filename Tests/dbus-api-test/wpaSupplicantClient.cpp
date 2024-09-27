#include "wpaSupplicantClient.h"

WpaSupplicantClient::WpaSupplicantClient() {
    NMLOG_INFO("WpaSupplicantClient created");
    if (!dbusConnection.InitializeBusConnection("fi.w1.wpa_supplicant1")) {
        NMLOG_ERROR("Failed to initialize D-Bus connection for wpa_supplicant");
    }
}

WpaSupplicantClient::~WpaSupplicantClient() {
    NMLOG_INFO("WpaSupplicantClient destroyed");
}

std::string WpaSupplicantClient::getWifiStatus() {
    GVariant* result = nullptr;
    std::string wifiStatus;

    if (dbusConnection.getConnection()) {
        // Prepare DBus method call
        GError* error = nullptr;
        result = g_dbus_connection_call_sync(
            dbusConnection.getConnection(),
            "fi.w1.wpa_supplicant1",
            "/fi/w1/wpa_supplicant1",
            "fi.w1.wpa_supplicant1.Interface",
            "GetStatus",
            nullptr,
            G_VARIANT_TYPE("(a{sv})"),
            G_DBUS_CALL_FLAGS_NONE,
            -1,
            nullptr,
            &error);

        if (error) {
            NMLOG_ERROR("Failed to get Wi-Fi status: %s", error->message);
            g_error_free(error);
            return "";
        }

        // Extract Wi-Fi status from result
        if (result) {
            gchar* status;
            g_variant_get(result, "(&s)", &status);
            wifiStatus = status;
            g_variant_unref(result);
            NMLOG_INFO("Wi-Fi status: %s", wifiStatus.c_str());
        }
    } else {
        NMLOG_WARNING("No valid D-Bus connection to wpa_supplicant");
    }

    return wifiStatus;
}
