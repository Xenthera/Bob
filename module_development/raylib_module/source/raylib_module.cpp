#include "BobModuleSDK.h"
#include <raylib.h>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>

class RaylibModule : public ModuleDef {
private:
    std::string name;

public:
    RaylibModule() : name("raylib") {}

    std::string getName() const override {
        return name;
    }

    void registerModule(class Interpreter& interpreter) override {
        interpreter.registerModule("raylib", [this](Interpreter::ModuleBuilder& m) {
            // Constants
            m.val("version", Value("1.0.0"));
            m.val("description", Value("Raylib graphics library bindings for Bob"));
            
            // Color class constructor
            m.class_("Color", [](std::vector<Value> args) -> Value {
                unsigned char r = 255, g = 255, b = 255, a = 255;
                
                if (args.size() >= 1) r = static_cast<unsigned char>(args[0].asInteger());
                if (args.size() >= 2) g = static_cast<unsigned char>(args[1].asInteger());
                if (args.size() >= 3) b = static_cast<unsigned char>(args[2].asInteger());
                if (args.size() >= 4) a = static_cast<unsigned char>(args[3].asInteger());
                
                // Create a normal Bob class instance with the same properties as the C++ Color struct
                std::unordered_map<std::string, Value> colorDict;
                colorDict["__class"] = Value("raylib::Color");
                colorDict["r"] = Value(static_cast<long long>(r));
                colorDict["g"] = Value(static_cast<long long>(g));
                colorDict["b"] = Value(static_cast<long long>(b));
                colorDict["a"] = Value(static_cast<long long>(a));
                
                return Value(colorDict);
            });
            
            // Vector2 class constructor
            m.class_("Vector2", [](std::vector<Value> args) -> Value {
                float x = 0.0f, y = 0.0f;
                
                if (args.size() >= 1) x = static_cast<float>(args[0].asNumber());
                if (args.size() >= 2) y = static_cast<float>(args[1].asNumber());
                
                // Create a normal Bob class instance with the same properties as the C++ Vector2 struct
                std::unordered_map<std::string, Value> vectorDict;
                vectorDict["__class"] = Value("raylib::Vector2");
                vectorDict["x"] = Value(x);
                vectorDict["y"] = Value(y);
                
                return Value(vectorDict);
            });
            
            // Window management functions
            m.fn("initWindow", [](std::vector<Value> args, int line, int column) -> Value {
                if (args.size() != 3 || args[0].type != ValueType::VAL_INTEGER || 
                    args[1].type != ValueType::VAL_INTEGER || args[2].type != ValueType::VAL_STRING) {
                    throw std::runtime_error("initWindow(width, height, title) requires two integers and a string");
                }
                
                int width = static_cast<int>(args[0].integer);
                int height = static_cast<int>(args[1].integer);
                const std::string& title = args[2].string_value;
                
                // Disable vsync to allow FPS control
                SetConfigFlags(0); // Clear all flags, including vsync
                
                InitWindow(width, height, title.c_str());
                
                                return Value();
            });
            
            m.fn("closeWindow", [](std::vector<Value> args, int line, int column) -> Value {
                CloseWindow();
                return Value();
            });
            
            m.fn("windowShouldClose", [](std::vector<Value> args, int line, int column) -> Value {
                return Value(WindowShouldClose());
            });
            
            m.fn("getScreenWidth", [](std::vector<Value> args, int line, int column) -> Value {
                return Value(static_cast<long long>(GetScreenWidth()));
            });
            
            m.fn("getScreenHeight", [](std::vector<Value> args, int line, int column) -> Value {
                return Value(static_cast<long long>(GetScreenHeight()));
            });
            
            m.fn("isKeyPressed", [](std::vector<Value> args, int line, int column) -> Value {
                if (args.size() != 1 || !args[0].isInteger()) {
                    throw std::runtime_error("isKeyPressed(key) requires one integer");
                }
                int key = static_cast<int>(args[0].asInteger());
                return Value(IsKeyPressed(key));
            });
            
            m.fn("beginDrawing", [](std::vector<Value> args, int line, int column) -> Value {
                BeginDrawing();
                return Value();
            });
            
            m.fn("endDrawing", [](std::vector<Value> args, int line, int column) -> Value {
                EndDrawing();
                return Value();
            });
            
            // FPS control functions
            m.fn("setTargetFPS", [](std::vector<Value> args, int line, int column) -> Value {
                if (args.size() != 1 || !args[0].isNumeric()) {
                    throw std::runtime_error("setTargetFPS(fps) requires a number");
                }
                
                int fps = static_cast<int>(args[0].asInteger());
                SetTargetFPS(fps);
                return Value();
            });
            
            m.fn("getFPS", [](std::vector<Value> args, int line, int column) -> Value {
                return Value(static_cast<long long>(GetFPS()));
            });
            
            // Add sleep function to help with FPS control
            m.fn("sleep", [](std::vector<Value> args, int line, int column) -> Value {
                if (args.size() != 1 || !args[0].isNumeric()) {
                    throw std::runtime_error("sleep(milliseconds) requires a number");
                }
                
                int milliseconds = static_cast<int>(args[0].asNumber());
                std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
                return Value();
            });
            
            m.fn("clearBackground", [](std::vector<Value> args, int line, int column) -> Value {
                if (args.size() != 1 || !args[0].isDict()) {
                    throw std::runtime_error("clearBackground(color) requires a color object");
                }
                
                // Convert Bob Color class to C++ Color struct
                const auto& colorDict = args[0].asDict();
                ::Color color;
                color.r = static_cast<unsigned char>(colorDict.at("r").asInteger());
                color.g = static_cast<unsigned char>(colorDict.at("g").asInteger());
                color.b = static_cast<unsigned char>(colorDict.at("b").asInteger());
                color.a = static_cast<unsigned char>(colorDict.at("a").asInteger());
                
                ClearBackground(color);
                return Value();
            });
            
            // Drawing functions
            m.fn("drawText", [](std::vector<Value> args, int line, int column) -> Value {
                if (args.size() < 3 || !args[0].isString() || 
                    !args[1].isNumeric() || !args[2].isNumeric()) {
                    throw std::runtime_error("drawText(text, x, y, [size], [color]) requires string and two numbers");
                }
                
                const std::string& text = args[0].asString();
                int x = static_cast<int>(args[1].asInteger());
                int y = static_cast<int>(args[2].asInteger());
                int fontSize = 20; // default
                ::Color color = WHITE; // default
                
                if (args.size() > 3 && args[3].isInteger()) {
                    fontSize = static_cast<int>(args[3].asInteger());
                }
                
                if (args.size() > 4 && args[4].isDict()) {
                    const auto& colorDict = args[4].asDict();
                    color.r = static_cast<unsigned char>(colorDict.at("r").asInteger());
                    color.g = static_cast<unsigned char>(colorDict.at("g").asInteger());
                    color.b = static_cast<unsigned char>(colorDict.at("b").asInteger());
                    color.a = static_cast<unsigned char>(colorDict.at("a").asInteger());
                }
                
                DrawText(text.c_str(), x, y, fontSize, color);
                return Value();
            });
            
            m.fn("drawCircle", [](std::vector<Value> args, int line, int column) -> Value {
                if (args.size() < 3 || !args[0].isNumeric() || 
                    !args[1].isNumeric() || !args[2].isNumeric()) {
                    throw std::runtime_error("drawCircle(x, y, radius, [color]) requires three numbers");
                }
                
                int x = static_cast<int>(args[0].asNumber());
                int y = static_cast<int>(args[1].asNumber());
                float radius = static_cast<float>(args[2].asNumber());
                
                ::Color color = WHITE; // default
                
                if (args.size() > 3 && args[3].isDict()) {
                    const auto& colorDict = args[3].asDict();
                    color.r = static_cast<unsigned char>(colorDict.at("r").asInteger());
                    color.g = static_cast<unsigned char>(colorDict.at("g").asInteger());
                    color.b = static_cast<unsigned char>(colorDict.at("b").asInteger());
                    color.a = static_cast<unsigned char>(colorDict.at("a").asInteger());
                }
                
                DrawCircle(x, y, radius, color);
                return Value();
            });
            
            // Color constants
            std::unordered_map<std::string, Value> whiteDict;
            whiteDict["__class"] = Value("raylib::Color");
            whiteDict["r"] = Value(static_cast<long long>(WHITE.r));
            whiteDict["g"] = Value(static_cast<long long>(WHITE.g));
            whiteDict["b"] = Value(static_cast<long long>(WHITE.b));
            whiteDict["a"] = Value(static_cast<long long>(WHITE.a));
            m.val("WHITE", Value(whiteDict));
            
            std::unordered_map<std::string, Value> blackDict;
            blackDict["__class"] = Value("raylib::Color");
            blackDict["r"] = Value(static_cast<long long>(BLACK.r));
            blackDict["g"] = Value(static_cast<long long>(BLACK.g));
            blackDict["b"] = Value(static_cast<long long>(BLACK.b));
            blackDict["a"] = Value(static_cast<long long>(BLACK.a));
            m.val("BLACK", Value(blackDict));
            
            std::unordered_map<std::string, Value> redDict;
            redDict["__class"] = Value("raylib::Color");
            redDict["r"] = Value(static_cast<long long>(RED.r));
            redDict["g"] = Value(static_cast<long long>(RED.g));
            redDict["b"] = Value(static_cast<long long>(RED.b));
            redDict["a"] = Value(static_cast<long long>(RED.a));
            m.val("RED", Value(redDict));
            
            std::unordered_map<std::string, Value> greenDict;
            greenDict["__class"] = Value("raylib::Color");
            greenDict["r"] = Value(static_cast<long long>(GREEN.r));
            greenDict["g"] = Value(static_cast<long long>(GREEN.g));
            greenDict["b"] = Value(static_cast<long long>(GREEN.b));
            greenDict["a"] = Value(static_cast<long long>(GREEN.a));
            m.val("GREEN", Value(greenDict));
            
            std::unordered_map<std::string, Value> blueDict;
            blueDict["__class"] = Value("raylib::Color");
            blueDict["r"] = Value(static_cast<long long>(BLUE.r));
            blueDict["g"] = Value(static_cast<long long>(BLUE.g));
            blueDict["b"] = Value(static_cast<long long>(BLUE.b));
            blueDict["a"] = Value(static_cast<long long>(BLUE.a));
            m.val("BLUE", Value(blueDict));
        });
    }
};

extern "C" ModuleDef* createModule() {
    return new RaylibModule();
}
