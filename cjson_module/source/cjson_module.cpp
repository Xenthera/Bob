#include "BobModuleSDK.h"
#include <cjson/cJSON.h>
#include <iostream>
#include <string>
#include <vector>

class CJsonModule : public ModuleDef {
private:
    std::string name;

    // Helper function to convert cJSON to Bob Value
    Value jsonToValue(cJSON* item) {
        if (!item) return Value();
        
        if (cJSON_IsString(item)) {
            return Value(cJSON_GetStringValue(item));
        } else if (cJSON_IsNumber(item)) {
            return Value(cJSON_GetNumberValue(item));
        } else if (cJSON_IsBool(item)) {
            return Value(cJSON_IsTrue(item) != 0);
        } else if (cJSON_IsNull(item)) {
            return Value();
        } else if (cJSON_IsArray(item)) {
            std::vector<Value> array;
            int size = cJSON_GetArraySize(item);
            for (int i = 0; i < size; i++) {
                cJSON* child = cJSON_GetArrayItem(item, i);
                array.push_back(jsonToValue(child));
            }
            return Value(array);
        } else if (cJSON_IsObject(item)) {
            std::unordered_map<std::string, Value> obj;
            cJSON* child = item->child;
            while (child) {
                obj[child->string] = jsonToValue(child);
                child = child->next;
            }
            return Value(obj);
        }
        return Value();
    }

    // Helper function to convert Bob Value to cJSON
    cJSON* valueToJson(const Value& value) {
        if (value.type == ValueType::VAL_STRING) {
            return cJSON_CreateString(value.string_value.c_str());
        } else if (value.type == ValueType::VAL_NUMBER) {
            return cJSON_CreateNumber(value.number);
        } else if (value.type == ValueType::VAL_INTEGER) {
            return cJSON_CreateNumber(static_cast<double>(value.integer));
        } else if (value.type == ValueType::VAL_BOOLEAN) {
            return cJSON_CreateBool(value.boolean);
        } else if (value.type == ValueType::VAL_ARRAY && value.array_value) {
            cJSON* array = cJSON_CreateArray();
            for (const auto& item : *value.array_value) {
                cJSON_AddItemToArray(array, valueToJson(item));
            }
            return array;
        } else if (value.type == ValueType::VAL_DICT && value.dict_value) {
            cJSON* obj = cJSON_CreateObject();
            for (const auto& pair : *value.dict_value) {
                cJSON_AddItemToObject(obj, pair.first.c_str(), valueToJson(pair.second));
            }
            return obj;
        }
        return cJSON_CreateNull();
    }

public:
    CJsonModule() : name("cjson") {}

    std::string getName() const override {
        return name;
    }

    void registerModule(class Interpreter& interpreter) override {
        interpreter.registerModule("cjson", [this](Interpreter::ModuleBuilder& m) {
            m.val("version", Value("1.0.0"));
            m.val("description", Value("cJSON library bindings for Bob"));
            
            m.fn("parse", [this](std::vector<Value> args, int line, int column) -> Value {
                if (args.size() != 1 || args[0].type != ValueType::VAL_STRING) {
                    throw std::runtime_error("parse() requires one string argument");
                }
                
                cJSON* json = cJSON_Parse(args[0].string_value.c_str());
                if (!json) {
                    throw std::runtime_error("Invalid JSON: " + std::string(cJSON_GetErrorPtr()));
                }
                
                Value result = jsonToValue(json);
                cJSON_Delete(json);
                return result;
            });
            
            m.fn("stringify", [this](std::vector<Value> args, int line, int column) -> Value {
                if (args.size() != 1) {
                    throw std::runtime_error("stringify() requires one argument");
                }
                
                cJSON* json = valueToJson(args[0]);
                char* jsonStr = cJSON_Print(json);
                std::string result(jsonStr);
                free(jsonStr);
                cJSON_Delete(json);
                return Value(result);
            });
            
            m.fn("isValid", [this](std::vector<Value> args, int line, int column) -> Value {
                if (args.size() != 1 || args[0].type != ValueType::VAL_STRING) {
                    throw std::runtime_error("isValid() requires one string argument");
                }
                
                cJSON* json = cJSON_Parse(args[0].string_value.c_str());
                bool isValid = (json != nullptr);
                if (json) cJSON_Delete(json);
                return Value(isValid);
            });
        });
    }
};

extern "C" ModuleDef* createModule() {
    return new CJsonModule();
}
