cmake_minimum_required(VERSION 3.22)
set(BIN QoreWallet)
project(${BIN} VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Gui Core)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

set(SOURCES
    src/main.cpp
    src/bitcoingui.cpp
    src/column.cpp
    src/row.cpp
    src/controller.cpp
    src/initexecutor.cpp
    src/panel.cpp
    src/config.cpp
    src/wallet.cpp

)

# Configure Bitcoin Core options to enable wallet but disable unnecessary binaries
set(BUILD_DAEMON ON CACHE BOOL "Disable building bitcoind")
set(BUILD_GUI OFF CACHE BOOL "Disable building bitcoin-qt")
set(BUILD_CLI OFF CACHE BOOL "Disable building bitcoin-cli")
set(BUILD_TX OFF CACHE BOOL "Disable building bitcoin-tx")
set(BUILD_UTIL OFF CACHE BOOL "Disable building bitcoin-util")
set(BUILD_TESTS OFF CACHE BOOL "Disable tests")
set(ENABLE_WALLET ON CACHE BOOL "Enable wallet support")
set(WITH_SQLITE ON CACHE BOOL "Enable SQLite wallet support")
set(WITH_BDB OFF CACHE BOOL "Disable Berkeley DB wallet support")
set(ENABLE_EXTERNAL_SIGNER OFF CACHE BOOL "Disable external signer support")

add_subdirectory(bitcoin)
set_target_properties(core_interface PROPERTIES AUTOMOC ON AUTOUIC OFF)
set_target_properties(bitcoin_node PROPERTIES AUTOMOC ON AUTOUIC OFF)
set_target_properties(bitcoin_wallet PROPERTIES AUTOMOC ON AUTOUIC OFF)

qt_add_executable(${BIN}
    ${SOURCES}
)

add_dependencies(${BIN} 
    core_interface
    bitcoin_node
    bitcoin_wallet
    bitcoin_common
)

target_include_directories(${BIN}
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/bitcoin/src>
)

target_link_libraries(${BIN} 
    PRIVATE 
    Qt6::Gui 
    Qt6::Core 
    Qt6::Widgets 
    core_interface 
    bitcoin_node 
    bitcoin_wallet
    bitcoin_common
)

set_target_properties(${BIN} PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

