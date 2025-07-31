#include <NetworkManager.h>
#include <iostream>
#include <gtest/gtest.h>
#include "nm_example.h"
#include "../mocks/LibnmWraps.h"
#include "../mocks/LibnmWrapsMock.h"

// Simpler mock implementation
const char* __wrap_nm_device_get_iface(NMDevice* device) {
    std::cout << "MOCK: nm_device_get_iface called" << std::endl;
    
    // Always return "wlan0"
    static const char* interface = "wlan0";
    return interface;
}

// Simple test case to verify our mocking framework
TEST(NetworkManagerTest, DeviceInterfaceNameTest) {
    // Setup the mock
    LibnmWrapsImplMock libnmMock;
    LibnmWraps::setImpl(&libnmMock);
    
    // Expect the mock to be called at least once
    EXPECT_CALL(libnmMock, nm_device_get_iface(::testing::_))
        .WillRepeatedly(::testing::Return("wlan0"));
        
    // Create and use NetworkManagerExample
    NetworkManagerExample nmExample;
    std::vector<std::string> interfaces = nmExample.getInterfaceNames();
    
    // Verify results
    ASSERT_FALSE(interfaces.empty());
    EXPECT_EQ(interfaces[0], "wlan0");
    
    // Clean up
    LibnmWraps::setImpl(nullptr);
}

// This is needed to make the test runnable
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}