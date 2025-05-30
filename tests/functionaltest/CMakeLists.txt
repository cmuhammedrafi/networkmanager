#############################################################################
# If not stated otherwise in this file or this component's LICENSE file the
# following copyright and licenses apply:
#
# Copyright 2024 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#############################################################################
message ("functional l3 test binary buidlt here")

set(functional_l3_tests "functional_l3_tests")

find_package(${NAMESPACE}Core REQUIRED)

add_executable(${functional_l3_tests}
    ${CMAKE_SOURCE_DIR}/tests/unittest/functional_l3_tests.cpp
)

set_target_properties(${functional_l3_tests} PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
)

target_include_directories(${functional_l3_tests} PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${functional_l3_tests}
    ${NAMESPACE}Core::${NAMESPACE}Core
    ${NAMESPACE}Plugins::${NAMESPACE}Plugins
)

install(TARGETS ${functional_l3_tests} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
