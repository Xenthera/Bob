#include "register.h"
#include "sys.h"

void registerAllBuiltinModules(Interpreter& interpreter) {
    registerSysModule(interpreter);
}


