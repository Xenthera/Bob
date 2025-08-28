#include "register.h"
#include "sys.h"
#include "os.h"
#include "eval.h"
#include "io.h"
#include "time_module.h"
#include "rand.h"
#include "math_module.h"
#include "path_module.h"
#include "base64_module.h"
#include "terminal.h"

void registerAllBuiltinModules(Interpreter& interpreter) {
    registerSysModule(interpreter);
    registerOsModule(interpreter);
    registerEvalModule(interpreter);
    registerIoModule(interpreter);
    registerTimeModule(interpreter);
    registerRandModule(interpreter);
    registerMathModule(interpreter);
    registerPathModule(interpreter);
    registerBase64Module(interpreter);
    registerTerminalModule(interpreter);
    // registerJsonModule(interpreter); // deferred pending extensive testing
}


