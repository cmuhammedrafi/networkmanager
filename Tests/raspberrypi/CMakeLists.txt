cmake_minimum_required(VERSION 3.3)

project(rpiTest)

set(RPI_TEST rpi_test)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GIO REQUIRED gio-2.0)
pkg_check_modules(LIBNM REQUIRED libnm)

add_executable(${RPI_TEST})

set_target_properties(${RPI_TEST} PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
)

target_sources(${RPI_TEST} PRIVATE rpiUnitTest.cpp
    ../../NetworkManagerLogger.cpp
    ../../GnomeProxy/NetworkManagerGnomeClient.cpp
    ../../GnomeProxy/NetworkManagerGnomeUtils.cpp
    ../../GnomeProxy/NetworkManagerDbusMgr.cpp
)

target_include_directories(${RPI_TEST} PRIVATE ${GLIB_INCLUDE_DIRS} ${LIBNM_INCLUDE_DIRS} ${GIO_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR})
target_include_directories(${RPI_TEST} PRIVATE ../../GnomeProxy ../../)

install(TARGETS ${RPI_TEST} DESTINATION bin)

target_link_libraries(${RPI_TEST} PRIVATE ${GLIB_LIBRARIES} ${GIO_LIBRARIES} uuid)
