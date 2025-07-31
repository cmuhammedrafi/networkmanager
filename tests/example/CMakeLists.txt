set(NM_EXAMPLE_TEST "nm_example_test")
set(NM_EXAMPLE_APP "nm_example")  # Define the app name

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBNM REQUIRED libnm>=1.0)
pkg_check_modules(GLIB REQUIRED glib-2.0>=2.32)

# Include googletest - if not already included at the parent level
if(NOT TARGET gtest_main)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
    )
    FetchContent_MakeAvailable(googletest)
endif()

# Regular executable - uses main.cpp
add_executable(${NM_EXAMPLE_APP}
    ${CMAKE_CURRENT_SOURCE_DIR}/nm_example.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

# Test executable - uses GoogleTest and mocks
add_executable(${NM_EXAMPLE_TEST}
    ${CMAKE_SOURCE_DIR}/tests/mocks/LibnmWraps.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/nm_example.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/nm_mocks.cpp
)

# Set properties for both targets
foreach(target ${NM_EXAMPLE_APP} ${NM_EXAMPLE_TEST})
    set_target_properties(${target} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES
    )
    
    target_include_directories(${target} PRIVATE
        ${LIBNM_INCLUDE_DIRS}
        ${GLIB_INCLUDE_DIRS}
    )
endforeach()

# Regular app gets normal libraries
target_link_libraries(${NM_EXAMPLE_APP} PRIVATE
    ${LIBNM_LIBRARIES}
    ${GLIB_LIBRARIES}
)

# Test gets additional include directories
target_include_directories(${NM_EXAMPLE_TEST} PRIVATE
    ${CMAKE_SOURCE_DIR}/tests/mocks
    ${gtest_SOURCE_DIR}/include
    ${gtest_SOURCE_DIR}/../googlemock/include
)

# Add the wrap option for test binary
target_link_options(${NM_EXAMPLE_TEST} PRIVATE
    -Wl,-wrap,nm_device_get_iface
)

# Test links against mock libraries - Use the target name, not -lgmock_main
target_link_libraries(${NM_EXAMPLE_TEST} PRIVATE
    ${GLIB_LIBRARIES}
    ${LIBNM_LIBRARIES}
    gmock_main
)

# Install both binaries
install(TARGETS 
    ${NM_EXAMPLE_APP}
    ${NM_EXAMPLE_TEST}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)