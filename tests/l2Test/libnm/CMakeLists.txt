#############################################################################
# Copyright 2024 RDK Management
# Licensed under the Apache License, Version 2.0
#############################################################################
message ("building libnm test")

set(NM_LIBNM_PROXY_L2_TEST "libnm_proxy_l2_test")

find_package(CURL)
find_package(PkgConfig REQUIRED)
find_package(${NAMESPACE}Core REQUIRED)
find_package(${NAMESPACE}Plugins REQUIRED)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(LIBNM REQUIRED libnm)
pkg_check_modules(GIO REQUIRED gio-2.0)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
FetchContent_MakeAvailable(googletest)

message("networkmanager building with libnm")
add_executable(${NM_LIBNM_PROXY_L2_TEST}
    ${CMAKE_SOURCE_DIR}/tests/l2Test/libnm/l2_test_libnmproxy.cpp
    ${CMAKE_SOURCE_DIR}/tests/mocks/thunder/Module.cpp
    ${CMAKE_SOURCE_DIR}/tests/mocks/Wraps.cpp
    ${CMAKE_SOURCE_DIR}/tests/mocks/LibnmWraps.cpp
    ${CMAKE_SOURCE_DIR}/plugin/NetworkManager.cpp
    ${CMAKE_SOURCE_DIR}/plugin/NetworkManagerLogger.cpp
    ${CMAKE_SOURCE_DIR}/plugin/NetworkManagerJsonRpc.cpp
    ${CMAKE_SOURCE_DIR}/plugin/NetworkManagerImplementation.cpp
    ${CMAKE_SOURCE_DIR}/plugin/NetworkManagerConnectivity.cpp
    ${CMAKE_SOURCE_DIR}/plugin/NetworkManagerStunClient.cpp
    ${CMAKE_SOURCE_DIR}/plugin/gnome/NetworkManagerGnomeProxy.cpp
    ${CMAKE_SOURCE_DIR}/plugin/gnome/NetworkManagerGnomeWIFI.cpp
    ${CMAKE_SOURCE_DIR}/plugin/gnome/NetworkManagerGnomeEvents.cpp
    ${CMAKE_SOURCE_DIR}/plugin/gnome/NetworkManagerGnomeUtils.cpp
    ${CMAKE_SOURCE_DIR}/plugin/NetworkManagerSecretAgent.cpp
    ${PROXY_STUB_SOURCES}
)

set_target_properties(${NM_LIBNM_PROXY_L2_TEST} PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
)

target_compile_options(${NM_LIBNM_PROXY_L2_TEST} PRIVATE -Wall -include ${CMAKE_SOURCE_DIR}/interface/INetworkManager.h)

include_directories(${PROJECT_SOURCE_DIR}/interface)
include_directories(${PROJECT_SOURCE_DIR}/plugin/rdk)
include_directories(${PROJECT_SOURCE_DIR}/plugin)
include_directories(${PROJECT_SOURCE_DIR}/legacy)
include_directories(${PROJECT_SOURCE_DIR}/tests/mocks)
include_directories(${PROJECT_SOURCE_DIR}/tests/mocks/thunder)
include_directories(${PROJECT_SOURCE_DIR}/tools/upnp/)

target_include_directories(${NM_LIBNM_PROXY_L2_TEST} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GLIB_INCLUDE_DIRS}
    ${LIBNM_INCLUDE_DIRS}
    ${GIO_INCLUDE_DIRS}
    ${gtest_SOURCE_DIR}/include
    ${gtest_SOURCE_DIR}/../googlemock/include
)

target_link_options(${NM_LIBNM_PROXY_L2_TEST} PRIVATE
    -g
    -Wl,-wrap,system
    -Wl,-wrap,popen
    -Wl,-wrap,syslog
    -Wl,-wrap,pclose
    -Wl,-wrap,getmntent
    -Wl,-wrap,setmntent
    -Wl,-wrap,v_secure_popen
    -Wl,-wrap,v_secure_pclose
    -Wl,-wrap,v_secure_system
    -Wl,-wrap,nm_device_get_iface
    -Wl,-wrap,nm_client_get_primary_connection
    -Wl,-wrap,nm_active_connection_get_connection
    -Wl,-wrap,nm_connection_get_interface_name
    -Wl,-wrap,nm_device_get_iface
    -Wl,-wrap,nm_client_get_device_by_iface
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")

target_link_libraries(${NM_LIBNM_PROXY_L2_TEST} PRIVATE
    gmock_main
    ${NAMESPACE}Core::${NAMESPACE}Core
    ${NAMESPACE}Plugins::${NAMESPACE}Plugins
    ${CURL_LIBRARIES}
    ${GLIB_LIBRARIES}
    ${LIBNM_LIBRARIES}
    ${GIO_LIBRARIES}
)

install(TARGETS ${NM_LIBNM_PROXY_L2_TEST} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
