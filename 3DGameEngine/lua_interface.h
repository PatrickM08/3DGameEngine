#include "ecs.h"

class LuaInterface {
public:
    LuaInterface(ECS&);
    int init();

private:
    ECS& scene;
};
