set(MU_CONNECT_SOURCE "${CMAKE_CURRENT_LIST_DIR}/../../ConnectServer")
add_library(mu_connect_core STATIC
  "${MU_CONNECT_SOURCE}/ConnectServerProtocol.cpp"
  "${MU_CONNECT_SOURCE}/ServerList.cpp"
  "${MU_CONNECT_SOURCE}/MemScript.cpp"
  "${MU_CONNECT_SOURCE}/BlackList.cpp"
  "${MU_CONNECT_SOURCE}/IpManager.cpp"
  "${MU_CONNECT_SOURCE}/ClientManager.cpp"
  "${MU_CONNECT_SOURCE}/Util.cpp"
  socket_manager.cpp socket_manager_udp.cpp log.cpp)
target_include_directories(mu_connect_core PUBLIC "${MU_CONNECT_SOURCE}" "${CMAKE_CURRENT_LIST_DIR}")
target_compile_definitions(mu_connect_core PUBLIC MU_NATIVE_MACOS=1 CONNECTSERVER_UPDATE=603 CONNECTSERVER_LANGUAGE=1)
target_compile_features(mu_connect_core PUBLIC cxx_std_17)
target_compile_options(mu_connect_core PRIVATE -Wno-writable-strings)
target_link_libraries(mu_connect_core PUBLIC mu_platform)
add_executable(ConnectServer-native main.cpp)
target_compile_options(ConnectServer-native PRIVATE -Wno-writable-strings)
target_link_libraries(ConnectServer-native PRIVATE mu_connect_core)
if(BUILD_TESTING)
  add_executable(mu-connect-tests tests.cpp)
  target_compile_options(mu-connect-tests PRIVATE -Wno-writable-strings)
  target_compile_definitions(mu-connect-tests PRIVATE MU_CONNECT_TESTDATA="${CMAKE_CURRENT_LIST_DIR}/testdata")
  target_link_libraries(mu-connect-tests PRIVATE mu_connect_core)
  add_test(NAME native-connect-core COMMAND mu-connect-tests)
  add_test(NAME native-connect-preflight COMMAND ConnectServer-native --config "${CMAKE_CURRENT_LIST_DIR}/testdata" --check-config)
  add_test(NAME native-connect-auto-preflight COMMAND ConnectServer-native --check-config)
  set_tests_properties(native-connect-auto-preflight PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
endif()
