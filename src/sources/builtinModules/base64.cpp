#include "base64_module.h"
#include "Interpreter.h"
#include <string>

static const char* B64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static std::string b64encode(const std::string& in){
    std::string out; out.reserve(((in.size()+2)/3)*4);
    int val=0, valb=-6;
    for (unsigned char c : in){
        val = (val<<8) + c;
        valb += 8;
        while (valb >= 0){ out.push_back(B64[(val>>valb)&0x3F]); valb -= 6; }
    }
    if (valb>-6) out.push_back(B64[((val<<8)>>(valb+8))&0x3F]);
    while (out.size()%4) out.push_back('=');
    return out;
}

static std::string b64decode(const std::string& in){
    std::vector<int> T(256,-1); for (int i=0;i<64;i++) T[(unsigned char)B64[i]]=i;
    std::string out; out.reserve((in.size()*3)/4);
    int val=0, valb=-8;
    for (unsigned char c : in){ if (T[c]==-1) break; val=(val<<6)+T[c]; valb+=6; if (valb>=0){ out.push_back(char((val>>valb)&0xFF)); valb-=8; } }
    return out;
}

void registerBase64Module(Interpreter& interpreter) {
    interpreter.registerModule("base64", [](Interpreter::ModuleBuilder& m) {
        m.fn("encode", [](std::vector<Value> a, int, int) -> Value {
            if (a.size()!=1 || !a[0].isString()) return NONE_VALUE;
            return Value(b64encode(a[0].asString()));
        });
        m.fn("decode", [](std::vector<Value> a, int, int) -> Value {
            if (a.size()!=1 || !a[0].isString()) return NONE_VALUE;
            return Value(b64decode(a[0].asString()));
        });
    });
}


