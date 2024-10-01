cmake_minimum_required(VERSION 3.5)
project(NmDbusClient)

set(CMAKE_CXX_STANDARD 11)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GIO REQUIRED gio-2.0)

include_directories(
    ${GLIB_INCLUDE_DIRS}
    ${GIO_INCLUDE_DIRS}
    /usr/include/NetworkManager/libnm
)

# Force the compiler to include utils.h automatically
add_compile_options(-include ${CMAKE_SOURCE_DIR}/utils.h)

add_executable(
    NmDbusClient 
    main.cpp
    dbusConnectionManager.cpp 
    NetworkManagerGnomeClient.cpp 
    NetworkManagerGnomeUtils.cpp
)

target_link_libraries(
    NmDbusClient 
    ${GLIB_LIBRARIES}
    ${GIO_LIBRARIES}
)