Embedding Bob: Public API Guide
================================

This document explains how to embed the Bob interpreter in your C++ application, register custom modules, and control sandbox policies.

Quick Start
-----------

```cpp
#include "cli/bob.h"
#include "ModuleRegistry.h"

int main() {
  Bob bob;

  // Optional: configure policies or modules before first use
  bob.setBuiltinModulePolicy(true);                 // allow builtin modules (default)
  bob.setBuiltinModuleDenyList({/* e.g., "sys" */});

  // Register a custom builtin module called "demo"
  bob.registerModule("demo", [](ModuleRegistry::ModuleBuilder& m) {
    m.fn("hello", [](std::vector<Value> args, int, int) -> Value {
      std::string who = (args.size() >= 1 && args[0].isString()) ? args[0].asString() : "world";
      return Value(std::string("hello ") + who);
    });
    m.val("meaning", Value(42.0));
  });

  // Evaluate code from a string
  bob.evalString("import demo; print(demo.hello(\"Bob\"));", "<host>");

  // Evaluate a file (imports inside resolve relative to the file's directory)
  bob.evalFile("script.bob");
}
```

API Overview
------------

Bob exposes a single high-level object with a minimal, consistent API. It self-manages an internal interpreter and applies configuration on first use.

- Program execution
  - `bool evalString(const std::string& code, const std::string& filename = "<eval>")`
  - `bool evalFile(const std::string& path)`
  - `void runFile(const std::string& path)` (CLI convenience – delegates to `evalFile`)
  - `void runPrompt()` (interactive CLI – delegates each line to `evalString`)

- Module registration and sandboxing
  - `void registerModule(const std::string& name, std::function<void(ModuleRegistry::ModuleBuilder&)> init)`
  - `void setBuiltinModulePolicy(bool allow)`
  - `void setBuiltinModuleAllowList(const std::vector<std::string>& allowed)`
  - `void setBuiltinModuleDenyList(const std::vector<std::string>& denied)`

- Global environment helpers
  - `bool defineGlobal(const std::string& name, const Value& value)`
  - `bool tryGetGlobal(const std::string& name, Value& out) const`

All configuration calls are safe to use before any evaluation – they are queued and applied automatically when the interpreter is first created.

Registering Custom Builtin Modules
----------------------------------

Use the builder convenience to create a module:

```cpp
bob.registerModule("raylib", [](ModuleRegistry::ModuleBuilder& m) {
  m.fn("init", [](std::vector<Value> args, int line, int col) -> Value {
    // call into your library here; validate args, return Value
    return NONE_VALUE;
  });
  m.val("VERSION", Value(std::string("5.0")));
});
```

At runtime:

```bob
import raylib;
raylib.init();
print(raylib.VERSION);
```

Notes
-----

- Modules are immutable, first-class objects. `type(module)` is "module" and `toString(module)` prints `<module 'name'>`.
- Reassigning a module binding or setting module properties throws an error.

Builtin Modules and Sandboxing
------------------------------

- Builtin modules (e.g., `sys`) are registered during interpreter construction.
- File imports are always resolved relative to the importing file's directory.
- Policies:
  - `setBuiltinModulePolicy(bool allow)` – enable/disable all builtin modules.
  - `setBuiltinModuleAllowList(vector<string>)` – allow only listed modules (deny others).
  - `setBuiltinModuleDenyList(vector<string>)` – explicitly deny listed modules.
- Denied/disabled modules are cloaked: `import name` reports "Module not found".

Error Reporting
---------------

- `evalString`/`evalFile` set file context for error reporting so line/column references point to the real source.
- Both return `true` on success and `false` if execution failed (errors are reported via the internal error reporter).

CLI vs Embedding
----------------

- CLI builds include `main.cpp` (entry point), which uses `Bob::runFile` or `Bob::runPrompt`.
- Embedded hosts do not use `main.cpp`; instead they instantiate `Bob` and call `evalString`/`evalFile` directly.



