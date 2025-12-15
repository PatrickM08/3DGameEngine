#include "lua_interface.h"
#include <sol/sol.hpp>

LuaInterface::LuaInterface(ECS& scene)
    : scene(scene) {
    init();
}

int LuaInterface::init() {
    sol::state lua;
    lua.open_libraries(sol::lib::base);
    lua["SpawnSkybox"] = [this]() { scene.useEntityTemplate("skyboxSky"); };
    std::string testLua = std::string(PROJECT_SOURCE_DIR) + "/scripts/main.lua";
    lua.script_file(testLua);
    return 0;
}
