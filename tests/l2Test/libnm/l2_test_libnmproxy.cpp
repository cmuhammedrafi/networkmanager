/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2024 RDK Management
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
**/
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <vector>
#include <cstdio>

#include "FactoriesImplementation.h"
#include "WrapsMock.h"
#include "LibnmWrapsMock.h"
#include "ServiceMock.h"
#include "ThunderPortability.h"
#include "COMLinkMock.h"
#include "WorkerPoolImplementation.h"
#include "NetworkManagerImplementation.h"
#include "NetworkManagerRDKProxy.h"
#include "NetworkManagerLogger.h"
#include "NetworkManager.h"
#include <libnm/NetworkManager.h>

using namespace WPEFramework;
using ::testing::NiceMock;

class NetworkManagerTest : public ::testing::Test {
protected:
    Core::ProxyType<Plugin::NetworkManager> plugin;
    Core::JSONRPC::Handler& handler;
    DECL_CORE_JSONRPC_CONX connection;
    Core::JSONRPC::Message message;
    string response;

    WrapsImplMock *p_wrapsImplMock = nullptr;
    LibnmWrapsImplMock *p_libnmWrapsImplMock = nullptr;
    Core::ProxyType<Plugin::NetworkManagerImplementation> NetworkManagerImpl;

    NiceMock<COMLinkMock> comLinkMock;
    NiceMock<ServiceMock> service;
    PLUGINHOST_DISPATCHER* dispatcher;
    Core::ProxyType<WorkerPoolImplementation> workerPool;
    NiceMock<FactoriesImplementation> factoriesImplementation;

    NetworkManagerTest()
        : plugin(Core::ProxyType<Plugin::NetworkManager>::Create())
        , handler(*(plugin))
        , INIT_CONX(1, 0)
        , workerPool(Core::ProxyType<WorkerPoolImplementation>::Create(2, Core::Thread::DefaultStackSize(), 16))
    {
        // Initialize libnmWrapsImplMock
        p_libnmWrapsImplMock = new NiceMock <LibnmWrapsImplMock>;
        LibnmWraps::setImpl(p_libnmWrapsImplMock);
        // Initialize WrapsImplMock
        p_wrapsImplMock = new NiceMock <WrapsImplMock>;
        Wraps::setImpl(p_wrapsImplMock);
        ON_CALL(service, COMLink())
        .WillByDefault(::testing::Invoke(
              [this]() {
                    return &comLinkMock;
                }));

        ON_CALL(service, ConfigLine())
            .WillByDefault(::testing::Return(
                "{"
                " \"locator\":\"libWPEFrameworkNetworkManager.so\"," 
                " \"classname\":\"NetworkManager\"," 
                " \"callsign\":\"org.rdk.NetworkManager\"," 
                " \"startuporder\":55," 
                " \"autostart\":false," 
                " \"configuration\":{"
                "  \"root\":{"
                "   \"outofprocess\":true,"
                "   \"locator\":\"libWPEFrameworkNetworkManagerImpl.so\""
                "  },"
                "  \"connectivity\":{"
                "   \"endpoint_1\":\"http://clients3.google.com/generate_204\","
                "   \"interval\":60"
                "  },"
                "  \"stun\":{"
                "   \"endpoint\":\"stun.l.google.com\","
                "   \"port\":19302,"
                "   \"interval\":30"
                "  },"
                "  \"loglevel\":4"
                " }"
                "}"
            ));

        ON_CALL(comLinkMock, Instantiate(::testing::_, ::testing::_, ::testing::_))
            .WillByDefault(::testing::Invoke(
                    [&](const RPC::Object& object, const uint32_t waitTime, uint32_t& connectionId) {
                        NetworkManagerImpl = Core::ProxyType<Plugin::NetworkManagerImplementation>::Create();
                        return &NetworkManagerImpl;
                }));

        PluginHost::IFactories::Assign(&factoriesImplementation);

        Core::IWorkerPool::Assign(&(*workerPool));
        workerPool->Run();

        dispatcher = static_cast<PLUGINHOST_DISPATCHER*>(
        plugin->QueryInterface(PLUGINHOST_DISPATCHER_ID));
        dispatcher->Activate(&service);
        response = plugin->Initialize(&service);
        EXPECT_EQ(string(""), response);
        NetworkManagerLogger::SetLevel(static_cast<NetworkManagerLogger::LogLevel>(NetworkManagerLogger::DEBUG_LEVEL)); // Set log level to DEBUG_LEVEL
    }

    virtual void SetUp() override
    {
        // remove any previous connectivity endpoints file
    }

    virtual ~NetworkManagerTest() override
    {
        plugin->Deinitialize(&service);

        dispatcher->Deactivate();
        dispatcher->Release();

        Core::IWorkerPool::Assign(nullptr);
        workerPool.Release();

        Wraps::setImpl(nullptr);
        if (p_wrapsImplMock != nullptr)
        {
            delete p_wrapsImplMock;
            p_wrapsImplMock = nullptr;
        }

        LibnmWraps::setImpl(nullptr);
        if (p_libnmWrapsImplMock != nullptr)
        {
            delete p_libnmWrapsImplMock;
            p_libnmWrapsImplMock = nullptr;
        }
    }
};

TEST_F(NetworkManagerTest, RegisteredMethods)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("SetLogLevel")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("GetLogLevel")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("GetAvailableInterfaces")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("GetPrimaryInterface")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("GetInterfaceState")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("SetInterfaceState")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("GetIPSettings")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("SetIPSettings")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("StartWiFiScan")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("StopWiFiScan")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("GetKnownSSIDs")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("AddToKnownSSIDs")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("RemoveKnownSSID")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("WiFiConnect")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("WiFiDisconnect")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("GetConnectedSSID")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("StartWPS")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("StopWPS")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("GetWifiState")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("GetWiFiSignalQuality")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("GetSupportedSecurityModes")));
}

TEST_F(NetworkManagerTest, GetPrimaryInterface)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("GetPrimaryInterface"), _T(""), response));
    EXPECT_EQ(response, _T("{\"interface\":\"wlan0\",\"success\":true}"));
}

TEST_F(NetworkManagerTest, GetPrimaryInterface2)
{
    NMActiveConnection* dummyActiveConnection = reinterpret_cast<NMActiveConnection*>(0x12345678);
    EXPECT_CALL(*p_libnmWrapsImplMock, nm_client_get_primary_connection(::testing::_))
        .WillOnce(::testing::Return(nullptr));

    // EXPECT_CALL(*p_libnmWrapsImplMock, nm_client_get_device_by_iface(client, ifname.c_str()))
    //     .WillRepeatedly(::testing::Return(dummyDevice));

    // EXPECT_CALL(*p_libnmWrapsImplMock, nm_device_get_state(dummyDevice))
    //     .WillRepeatedly(::testing::Return(NM_DEVICE_STATE_ACTIVATED));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("GetPrimaryInterface"), _T(""), response));
    EXPECT_EQ(response, _T("{\"interface\":\"wlan0\",\"success\":true}"));

    EXPECT_CALL(*p_libnmWrapsImplMock, nm_client_get_primary_connection(::testing::_))
        .WillRepeatedly(::testing::Return(dummyActiveConnection));

    EXPECT_CALL(*p_libnmWrapsImplMock, nm_active_connection_get_connection(::testing::_))
        .WillRepeatedly(::testing::Return(nullptr));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("GetPrimaryInterface"), _T(""), response));
    EXPECT_EQ(response, _T("{\"interface\":\"wlan0\",\"success\":true}"));
}

TEST_F(NetworkManagerTest, GetInterfaceState_Failed)
{
    // Mock nm_client_get_devices to return our fake array
    EXPECT_CALL(*p_libnmWrapsImplMock, nm_client_get_devices(::testing::_))
        .WillRepeatedly(::testing::Return(nullptr));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("GetInterfaceState"), _T("{\"interface\":\"wlan0\"}"), response));
    EXPECT_EQ(response, _T("{\"success\":false}"));
}

TEST_F(NetworkManagerTest, GetInterfaceState_WifiEth)
{
    // Create a GPtrArray with one valid device pointer
    GPtrArray* fakeDevices = g_ptr_array_new();

    NMDevice *deviceDummy = static_cast<NMDevice*>(g_object_new(NM_TYPE_DEVICE_ETHERNET, NULL));
    g_ptr_array_add(fakeDevices, deviceDummy);

    EXPECT_CALL(*p_libnmWrapsImplMock, nm_client_get_devices(::testing::_))
        .WillRepeatedly(::testing::Return(fakeDevices));

    EXPECT_CALL(*p_libnmWrapsImplMock, nm_device_get_iface(::testing::_))
        .WillOnce(::testing::Return("wlan0"))
        .WillOnce(::testing::Return("eth0"));

    EXPECT_CALL(*p_libnmWrapsImplMock, nm_device_get_state(::testing::_))
        .WillOnce(::testing::Return(NM_DEVICE_STATE_ACTIVATED))
        .WillOnce(::testing::Return(NM_DEVICE_STATE_UNMANAGED)); // disabled

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("GetInterfaceState"), _T("{\"interface\":\"wlan0\"}"), response));
    EXPECT_EQ(response, _T("{\"enabled\":true,\"success\":true}"));

    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("GetInterfaceState"), _T("{\"interface\":\"eth0\"}"), response));
    EXPECT_EQ(response, _T("{\"enabled\":false,\"success\":true}"));

    // Clean up
    g_object_unref(deviceDummy);
    g_ptr_array_free(fakeDevices, TRUE);
}



// TEST_F(NetworkManagerTest, GetInterfaceState_EthDisabled)
// {
//     // Create a GPtrArray with one valid device pointer
//     GPtrArray* fakeDevices = g_ptr_array_new();
//     NMDevice* fakeDevice = reinterpret_cast<NMDevice*>(0xDEADBEEF);
//     g_ptr_array_add(fakeDevices, fakeDevice); // This sets len=1 and pdata[0]=fakeDevice

//     // Mock nm_client_get_devices to return our fake array
//     EXPECT_CALL(*p_libnmWrapsImplMock, nm_client_get_devices(::testing::_))
//         .WillRepeatedly(::testing::Return(fakeDevices));

//     // Mock nm_device_get_iface to return "eth0"
//     EXPECT_CALL(*p_libnmWrapsImplMock, nm_device_get_iface(::testing::_))
//         .WillRepeatedly(::testing::Return("eth0"));

//     // Mock nm_device_get_state to return NM_DEVICE_STATE_ACTIVATED
//     EXPECT_CALL(*p_libnmWrapsImplMock, nm_device_get_state(::testing::_))
//         .WillRepeatedly(::testing::Return(NMDeviceState::NM_DEVICE_STATE_ACTIVATED));

//     EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("GetInterfaceState"), _T("{\"interface\":\"eth0\"}"), response));
//     EXPECT_EQ(response, _T("{\"enabled\":false,\"success\":true}"));

//     // Clean up
//     g_ptr_array_free(fakeDevices, TRUE);
// }

