# Compile the actual C++ gameplay implementation. This OBJECT target is a porting milestone;
# it deliberately does not claim a linked/runnable GameServer before platform adapters exist.
set(MU_EMULATOR_ROOT "${CMAKE_CURRENT_LIST_DIR}/../..")
get_filename_component(MU_EMULATOR_ROOT "${MU_EMULATOR_ROOT}" ABSOLUTE)
get_filename_component(MU_VENDOR_ROOT "${MU_EMULATOR_ROOT}/../../Util" ABSOLUTE)

file(GLOB MU_GAME_ALL_SOURCES CONFIGURE_DEPENDS "${MU_EMULATOR_ROOT}/GameServer/*.cpp")
set(MU_GAME_PLATFORM_SOURCES
    GameServer.cpp GameMain.cpp SocketManager.cpp SocketManagerUdp.cpp Connection.cpp
    QueryManager.cpp ServerDisplayer.cpp MiniDump.cpp Util.cpp HackCheck.cpp QueueTimer.cpp stdafx.cpp)
set(MU_GAME_LOGIC_SOURCES ${MU_GAME_ALL_SOURCES})
foreach(source IN LISTS MU_GAME_PLATFORM_SOURCES)
    list(REMOVE_ITEM MU_GAME_LOGIC_SOURCES "${MU_EMULATOR_ROOT}/GameServer/${source}")
endforeach()

add_library(mu_game_logic OBJECT ${MU_GAME_LOGIC_SOURCES})
target_include_directories(mu_game_logic PRIVATE
    "${MU_EMULATOR_ROOT}/native/include"
    "${MU_VENDOR_ROOT}/lua/include")
target_compile_features(mu_game_logic PRIVATE cxx_std_17)
target_compile_definitions(mu_game_logic PRIVATE
    MU_NATIVE_MACOS=1 GAMESERVER_UPDATE=603 GAMESERVER_LANGUAGE=1
    GAMESERVER_SHOP=3 GAMESERVER_EXTRA=1 ENCRYPT_STATE=1)
target_compile_options(mu_game_logic PRIVATE
    -fms-extensions -iquote "${MU_EMULATOR_ROOT}/GameServer" -Wno-ignored-pragmas -Wno-writable-strings
    -Wno-deprecated-declarations -Wno-return-type-c-linkage)

if(BUILD_TESTING)
    add_executable(mu_game_effect_contract_test
        "${CMAKE_CURRENT_LIST_DIR}/effect_contract_test.cpp"
        "${MU_EMULATOR_ROOT}/GameServer/Effect.cpp")
    target_include_directories(mu_game_effect_contract_test PRIVATE
        "${MU_EMULATOR_ROOT}/native/include"
        "${MU_VENDOR_ROOT}/lua/include")
    target_compile_features(mu_game_effect_contract_test PRIVATE cxx_std_17)
    target_compile_definitions(mu_game_effect_contract_test PRIVATE
        MU_NATIVE_MACOS=1 GAMESERVER_UPDATE=603 GAMESERVER_LANGUAGE=1
        GAMESERVER_SHOP=3 GAMESERVER_EXTRA=1 ENCRYPT_STATE=1)
    target_compile_options(mu_game_effect_contract_test PRIVATE -fms-extensions -iquote "${MU_EMULATOR_ROOT}/GameServer")
    if(TARGET mu_platform)
        target_link_libraries(mu_game_effect_contract_test PRIVATE mu_platform)
    endif()
    add_test(NAME mu_game_effect_contract COMMAND mu_game_effect_contract_test)
    target_compile_options(mu_game_effect_contract_test PRIVATE -UNDEBUG)
    add_executable(mu_game_random_contract_test random_pointer_contract_test.cpp "${MU_EMULATOR_ROOT}/GameServer/RandomManager.cpp")
    target_include_directories(mu_game_random_contract_test PRIVATE "${MU_EMULATOR_ROOT}/native/include" "${MU_VENDOR_ROOT}/lua/include")
    target_compile_options(mu_game_random_contract_test PRIVATE -UNDEBUG -iquote "${MU_EMULATOR_ROOT}/GameServer")
    target_compile_definitions(mu_game_random_contract_test PRIVATE GAMESERVER_UPDATE=603 GAMESERVER_LANGUAGE=1 GAMESERVER_SHOP=3 GAMESERVER_EXTRA=1 ENCRYPT_STATE=1)
    target_link_libraries(mu_game_random_contract_test PRIVATE mu_platform)
    add_test(NAME mu_game_random_contract COMMAND mu_game_random_contract_test)
    add_executable(mu_game_abi_contract_test abi_contract_test.cpp)
    target_compile_options(mu_game_abi_contract_test PRIVATE -iquote "${MU_EMULATOR_ROOT}/GameServer")
    target_include_directories(mu_game_abi_contract_test PRIVATE "${MU_VENDOR_ROOT}/lua/include")
    target_compile_definitions(mu_game_abi_contract_test PRIVATE GAMESERVER_UPDATE=603 GAMESERVER_LANGUAGE=1 GAMESERVER_SHOP=3 GAMESERVER_EXTRA=1 ENCRYPT_STATE=1)
    target_link_libraries(mu_game_abi_contract_test PRIVATE mu_platform)
    add_test(NAME mu_game_abi_contract COMMAND mu_game_abi_contract_test)
    add_executable(mu_game_timer_contract_test timer_contract_test.cpp queue_timer.cpp)
    target_link_libraries(mu_game_timer_contract_test PRIVATE mu_platform)
    target_compile_options(mu_game_timer_contract_test PRIVATE -UNDEBUG)
    add_test(NAME mu_game_timer_contract COMMAND mu_game_timer_contract_test)
    add_executable(mu_game_callback_tokens_test callback_tokens_test.cpp callback_tokens.cpp)
    target_compile_options(mu_game_callback_tokens_test PRIVATE -UNDEBUG)
    add_test(NAME mu_game_callback_tokens COMMAND mu_game_callback_tokens_test)
    add_executable(mu_game_cash_callback_abi_test cash_callback_abi_test.cpp)
    target_compile_options(mu_game_cash_callback_abi_test PRIVATE -iquote "${MU_EMULATOR_ROOT}/GameServer")
    target_include_directories(mu_game_cash_callback_abi_test PRIVATE "${MU_VENDOR_ROOT}/lua/include")
    target_compile_definitions(mu_game_cash_callback_abi_test PRIVATE GAMESERVER_UPDATE=603 GAMESERVER_LANGUAGE=1 GAMESERVER_SHOP=3 GAMESERVER_EXTRA=1 ENCRYPT_STATE=1)
    target_link_libraries(mu_game_cash_callback_abi_test PRIVATE mu_platform)
    add_test(NAME mu_game_cash_callback_abi COMMAND mu_game_cash_callback_abi_test)
    add_executable(mu_game_packet_crypto_test packet_crypto_test.cpp
        "${MU_EMULATOR_ROOT}/GameServer/PacketManager.cpp" "${MU_EMULATOR_ROOT}/GameServer/HackCheck.cpp" "${MU_EMULATOR_ROOT}/GameServer/ServerInfo.cpp")
    target_compile_options(mu_game_packet_crypto_test PRIVATE -UNDEBUG -ffunction-sections -fdata-sections -iquote "${MU_EMULATOR_ROOT}/GameServer")
    target_include_directories(mu_game_packet_crypto_test PRIVATE "${MU_VENDOR_ROOT}/lua/include")
    target_compile_definitions(mu_game_packet_crypto_test PRIVATE GAMESERVER_UPDATE=603 GAMESERVER_LANGUAGE=1 GAMESERVER_SHOP=3 GAMESERVER_EXTRA=1 ENCRYPT_STATE=1)
    target_link_options(mu_game_packet_crypto_test PRIVATE -Wl,-dead_strip)
    target_link_libraries(mu_game_packet_crypto_test PRIVATE mu_platform)
    set(MU_TEST_SERVER_KEY_DIR "/Users/angelale/Downloads/MuServer/Data/Hack" CACHE PATH "Directory containing real server Enc2.dat/Dec1.dat")
    set(MU_TEST_CLIENT_KEY_DIR "/Users/angelale/Downloads/MuClientEX603/Data" CACHE PATH "Directory containing real client Enc1.dat/Dec2.dat")
    if(EXISTS "${MU_TEST_SERVER_KEY_DIR}/Enc2.dat" AND EXISTS "${MU_TEST_CLIENT_KEY_DIR}/Enc1.dat")
        add_test(NAME mu_game_packet_crypto COMMAND mu_game_packet_crypto_test "${MU_TEST_SERVER_KEY_DIR}" "${MU_TEST_CLIENT_KEY_DIR}")
    endif()
endif()

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/socket_manager.cpp" AND TARGET mu_lua52)
    add_executable(GameServer-native main.cpp display.cpp connection.cpp socket_manager.cpp socket_manager_udp.cpp queue_timer.cpp callback_tokens.cpp native_cash_callbacks.cpp
        "${MU_EMULATOR_ROOT}/GameServer/GameMain.cpp" "${MU_EMULATOR_ROOT}/GameServer/Util.cpp" "${MU_EMULATOR_ROOT}/GameServer/HackCheck.cpp")
    target_compile_options(GameServer-native PRIVATE -iquote "${MU_EMULATOR_ROOT}/GameServer")
    target_compile_definitions(GameServer-native PRIVATE GAMESERVER_UPDATE=603 GAMESERVER_LANGUAGE=1 GAMESERVER_SHOP=3 GAMESERVER_EXTRA=1 ENCRYPT_STATE=1)
    target_link_libraries(GameServer-native PRIVATE mu_game_logic mu_platform mu_mysql mu_lua52)
endif()

# Dependencies at link time (Lua5.2, MySQL, platform transport/timers) are intentionally
# not represented by fake success/no-op functions. Root native CMake supplies real adapters.
set(MU_GAME_PLATFORM_REPLACEMENTS "${MU_GAME_PLATFORM_SOURCES}" CACHE INTERNAL
    "Original GameServer sources requiring native runtime adapters")

if(BUILD_TESTING)
 add_executable(mu_game_book_requirements_test book_requirements_test.cpp)
 target_include_directories(mu_game_book_requirements_test PRIVATE "${MU_EMULATOR_ROOT}/native/include")
 target_compile_features(mu_game_book_requirements_test PRIVATE cxx_std_17)
 target_compile_options(mu_game_book_requirements_test PRIVATE -UNDEBUG)
 add_test(NAME mu_game_book_requirements COMMAND mu_game_book_requirements_test)
endif()
