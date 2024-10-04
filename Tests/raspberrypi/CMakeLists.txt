cmake_minimum_required(VERSION 3.5)
project(NmDbusClient)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GIO REQUIRED gio-2.0)
pkg_check_modules(LIBNM REQUIRED libnm)

include_directories(
    ${GLIB_INCLUDE_DIRS} 
    ${GIO_INCLUDE_DIRS} 
    ${LIBNM_INCLUDE_DIRS}
    ../../
    ../../GnomeProxy
)

add_executable(
    NmDbusClient 
    rpiUnitTest.cpp             
    ../../NetworkManagerLogger.cpp
    ../../GnomeProxy/NetworkManagerGnomeClient.cpp
    ../../GnomeProxy/NetworkManagerGnomeUtils.cpp
    ../../GnomeProxy/NetworkManagerDbusMgr.cpp
    )

target_include_directories(NmDbusClient PRIVATE ../GnomeProxy)
target_link_libraries(
    NmDbusClient 
    ${GLIB_LIBRARIES}
    ${GIO_LIBRARIES}
)