#pragma once

// @stubgen:include <com/IIteratorType.h>
#include <iostream>
#include <string>
#include <list>
using namespace std;
namespace WPEFramework
{
    namespace Exchange
    {
        namespace INetworkManager
        {

            enum InterfaceType : uint8_t
            {
                INTERFACE_TYPE_ETHERNET,
                INTERFACE_TYPE_WIFI,
                INTERFACE_TYPE_P2P,
            };

            struct InterfaceDetails {
                string m_type;
                string m_name;
                string m_mac;
                bool   m_isEnabled;
                bool   m_isConnected;
            };

            enum IPAddressType : uint8_t
            {
                IP_ADDRESS_V4,
                IP_ADDRESS_V6,
                IP_ADDRESS_BOTH
            };

            struct IPAddressInfo {
                string m_ipAddrType;
                bool m_autoConfig;
                string m_dhcpServer;
                string m_v6LinkLocal;
                string m_ipAddress;
                uint32_t m_prefix;
                string m_gateway;
                string m_primaryDns;
                string m_secondaryDns;
            };

            // Define the RPC methods
            enum InternetStatus : uint8_t
            {
                INTERNET_NOT_AVAILABLE,
                INTERNET_LIMITED,
                INTERNET_CAPTIVE_PORTAL,
                INTERNET_FULLY_CONNECTED,
                INTERNET_UNKNOWN,
            };

            enum WiFiFrequency : uint8_t
            {
                WIFI_FREQUENCY_WHATEVER,
                WIFI_FREQUENCY_2_4_GHZ,
                WIFI_FREQUENCY_5_GHZ,
                WIFI_FREQUENCY_6_GHZ
            };

            enum WiFiWPS : uint8_t
            {
                WIFI_WPS_PBC,
                WIFI_WPS_PIN,
                WIFI_WPS_SERIALIZED_PIN
            };

            enum WIFISecurityMode : uint8_t
            {
                WIFI_SECURITY_NONE,
                WIFI_SECURITY_WEP_64,
                WIFI_SECURITY_WEP_128,
                WIFI_SECURITY_WPA_PSK_TKIP,
                WIFI_SECURITY_WPA_PSK_AES,
                WIFI_SECURITY_WPA2_PSK_TKIP,
                WIFI_SECURITY_WPA2_PSK_AES,
                WIFI_SECURITY_WPA_ENTERPRISE_TKIP,
                WIFI_SECURITY_WPA_ENTERPRISE_AES,
                WIFI_SECURITY_WPA2_ENTERPRISE_TKIP,
                WIFI_SECURITY_WPA2_ENTERPRISE_AES,
                WIFI_SECURITY_WPA_WPA2_PSK,
                WIFI_SECURITY_WPA_WPA2_ENTERPRISE,
                WIFI_SECURITY_WPA3_PSK_AES,
                WIFI_SECURITY_WPA3_SAE
            };

            struct WiFiScanResults {
                    string           m_ssid;
                    WIFISecurityMode m_securityMode;
                    string           m_signalStrength;
                    double           m_frequency;
            };

            struct WiFiConnectTo {
                    string           m_ssid;
                    string           m_passphrase;
                    WIFISecurityMode m_securityMode;
                    string           m_identity;
                    string           m_caCert;
                    string           m_clientCert;
                    string           m_privateKey;
                    string           m_privateKeyPasswd;
                    bool             m_persistSSIDInfo;
            };

            struct WiFiSSIDInfo {
                    string             m_ssid;
                    string             m_bssid;
                    WIFISecurityMode   m_securityMode;
                    string             m_signalStrength;
                    double             m_frequency;
                    string             m_rate;
                    string             m_noise;
            };

            struct WIFISecurityModeInfo {
                WIFISecurityMode m_securityMode;
                string           m_securityModeText;
            };

            enum WiFiSignalQuality : uint8_t
            {
                WIFI_SIGNAL_DISCONNECTED,
                WIFI_SIGNAL_WEAK,
                WIFI_SIGNAL_FAIR,
                WIFI_SIGNAL_GOOD,
                WIFI_SIGNAL_EXCELLENT
            };

            enum NMLogging : uint8_t
            {
                LOG_LEVEL_FATAL,
                LOG_LEVEL_ERROR,
                LOG_LEVEL_WARNING,
                LOG_LEVEL_INFO,
                LOG_LEVEL_VERBOSE,
                LOG_LEVEL_TRACE
            };

           // The state of the interface 
            enum InterfaceState : uint8_t
            {
                INTERFACE_ADDED,
                INTERFACE_LINK_UP,
                INTERFACE_LINK_DOWN,
                INTERFACE_ACQUIRING_IP,
                INTERFACE_REMOVED,
                INTERFACE_DISABLED
            };

            enum WiFiState : uint8_t
            {
                WIFI_STATE_UNINSTALLED,
                WIFI_STATE_DISABLED,
                WIFI_STATE_DISCONNECTED,
                WIFI_STATE_PAIRING,
                WIFI_STATE_CONNECTING,
                WIFI_STATE_CONNECTED,
                WIFI_STATE_SSID_NOT_FOUND,
                WIFI_STATE_SSID_CHANGED,
                WIFI_STATE_CONNECTION_LOST,
                WIFI_STATE_CONNECTION_FAILED,
                WIFI_STATE_CONNECTION_INTERRUPTED,
                WIFI_STATE_INVALID_CREDENTIALS,
                WIFI_STATE_AUTHENTICATION_FAILED,
                WIFI_STATE_ERROR,
                WIFI_STATE_INVALID
            };
        }
    }
}
