#
# Copyright 2026 Christoph Pospiech
#
# SPDX-License-Identifier: GPL-3.0-or-later
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
######################################
# Unit tests for WalletHandle        #
#                                    #
# Uses Qt Test (Qt5::Test).          #
# Compiles wallethandle.cpp directly #
# into the test executable.          #
######################################

find_package(Qt5 REQUIRED COMPONENTS DBus Test Widgets)

# KF5 imported targets (KF5::Wallet, KF5::CoreAddons) are created
# globally when readKwallet is configured first.  Guard against
# standalone or out-of-order builds by finding them if missing.
if(NOT TARGET KF5::Wallet)
    find_package(ECM REQUIRED NO_MODULE)
    list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})
    find_package(KF5 REQUIRED COMPONENTS CoreAddons Wallet)
endif()

set(CMAKE_AUTOMOC ON)

add_executable(test_closed_wallet
    src/test_closed_wallet.cpp
    ${CMAKE_SOURCE_DIR}/readKwallet/src/wallethandle.cpp
)

target_include_directories(test_closed_wallet PRIVATE
    ${CMAKE_SOURCE_DIR}/readKwallet/include
)

target_compile_features(test_closed_wallet PRIVATE cxx_std_14)

target_link_libraries(test_closed_wallet
    Qt5::Core
    Qt5::Widgets
    Qt5::Test
    KF5::CoreAddons
    KF5::Wallet
)

add_test(NAME closed_wallet COMMAND test_closed_wallet)

# Prevent kwalletd from being reachable during tests.
# Without a valid DBus session bus, KWallet::Wallet::openWallet()
# returns nullptr immediately instead of showing a GUI dialog asking
# the user to create a wallet.
set_tests_properties(closed_wallet PROPERTIES
    ENVIRONMENT "DBUS_SESSION_BUS_ADDRESS=unix:path=/tmp/test_closed_wallet_no_dbus"
)

######################################
# read_wallet: integration test that #
# imports and reads a real wallet.   #
######################################

add_executable(test_read_wallet
    src/test_read_wallet.cpp
    include/test_read_wallet.hpp
    ${CMAKE_SOURCE_DIR}/readKwallet/src/wallethandle.cpp
)

target_include_directories(test_read_wallet PRIVATE
    ${CMAKE_SOURCE_DIR}/readKwallet/include
    ${CMAKE_SOURCE_DIR}/tests/wallethandle/include
)

target_compile_features(test_read_wallet PRIVATE cxx_std_14)

# Exposes the path to tests/test_data inside the source tree.
target_compile_definitions(test_read_wallet PRIVATE
    KWALLET_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../test_data"
)

target_link_libraries(test_read_wallet
    Qt5::Core
    Qt5::DBus
    Qt5::Widgets
    Qt5::Test
    KF5::CoreAddons
    KF5::Wallet
)

add_test(NAME read_wallet COMMAND test_read_wallet)

# This test needs the live kwalletd5 daemon and the test wallet archive to run.
# It returns 222 when prerequisites are not met, causing CTest to mark this test as skipped.
set_property(
    TEST read_wallet
    PROPERTY SKIP_RETURN_CODE 222
)

######################################
# pybind module to create tests      #
######################################

# See https://pybind11.readthedocs.io/en/stable/compiling.html
# for details on pybind11_add_module.

pybind11_add_module( test_wallet MODULE
    src/test_read_wallet.cpp
    include/test_read_wallet.hpp
    src/wrap_pybind_test_wallet.cpp
    ${CMAKE_SOURCE_DIR}/readKwallet/src/wallethandle.cpp
)

target_include_directories(test_wallet PRIVATE
    ${CMAKE_SOURCE_DIR}/readKwallet/include
    ${CMAKE_SOURCE_DIR}/tests/wallethandle/include
)

target_compile_features(test_wallet PRIVATE cxx_std_14)

# BUILDING_PYBIND_MODULE suppresses QTEST_GUILESS_MAIN in test_read_wallet.cpp.
# KWALLET_TEST_DATA_DIR mirrors the definition used by the test_read_wallet
# executable so that testDataArchive() resolves to the same archive path.
target_compile_definitions(test_wallet PRIVATE
    BUILDING_PYBIND_MODULE
    KWALLET_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../test_data"
)

target_link_libraries(test_wallet PRIVATE
    Qt5::Core
    Qt5::DBus
    Qt5::Widgets
    Qt5::Test
    KF5::CoreAddons
    KF5::Wallet
)

add_custom_command(TARGET test_wallet POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:test_wallet>
        "${CMAKE_SOURCE_DIR}/$<TARGET_FILE_NAME:test_wallet>"
)

######################################
# run_read_wallet_py.py: Python test  #
######################################

add_test(
    NAME read_wallet_py
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/run_read_wallet_py.py
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

set_property(
    TEST read_wallet_py
    PROPERTY ENVIRONMENT
    PYTHONPATH=${CMAKE_SOURCE_DIR}:${CMAKE_CURRENT_SOURCE_DIR}:$ENV{PYTHONPATH}
)

# This test needs the live kwalletd5 daemon and the test wallet archive to run.
# It returns 222 when prerequisites are not met, causing CTest to mark this test as skipped.
set_property(
    TEST read_wallet_py
    PROPERTY SKIP_RETURN_CODE 222
)
