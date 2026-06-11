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
    test_closed_wallet.cpp
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
    test_read_wallet.cpp
    ${CMAKE_SOURCE_DIR}/readKwallet/src/wallethandle.cpp
)

target_include_directories(test_read_wallet PRIVATE
    ${CMAKE_SOURCE_DIR}/readKwallet/include
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
# Note: no DBUS_SESSION_BUS_ADDRESS override – this test needs the live
# kwalletd5 daemon.  It skips automatically when the daemon is not available.
