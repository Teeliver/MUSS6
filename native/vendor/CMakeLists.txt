# Lua 5.2.4 official source; SHA256 recorded in SOURCE.md.
file(GLOB MU_LUA_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/lua-5.2.4/src/*.c")
list(REMOVE_ITEM MU_LUA_SOURCES "${CMAKE_CURRENT_LIST_DIR}/lua-5.2.4/src/lua.c" "${CMAKE_CURRENT_LIST_DIR}/lua-5.2.4/src/luac.c")
add_library(mu_lua52 STATIC ${MU_LUA_SOURCES})
target_compile_definitions(mu_lua52 PRIVATE LUA_USE_MACOSX)
target_include_directories(mu_lua52 PUBLIC "${CMAKE_CURRENT_LIST_DIR}/lua-5.2.4/src")
