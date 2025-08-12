#include "json.h"
#include "Interpreter.h"
#include <string>
#include <cctype>

// Minimal JSON parser/stringifier (numbers, strings, booleans, null, arrays, objects)
namespace {
    struct Cursor { const std::string* s; size_t i = 0; };
    void skipWs(Cursor& c){ while (c.i < c.s->size() && std::isspace(static_cast<unsigned char>((*c.s)[c.i]))) ++c.i; }
    bool match(Cursor& c, char ch){ skipWs(c); if (c.i < c.s->size() && (*c.s)[c.i]==ch){ ++c.i; return true;} return false; }
    std::string parseString(Cursor& c){
        if (!match(c,'"')) return {};
        std::string out; while (c.i < c.s->size()){
            char ch = (*c.s)[c.i++];
            if (ch=='"') break;
            if (ch=='\\' && c.i < c.s->size()){
                char e = (*c.s)[c.i++];
                switch(e){ case '"': out+='"'; break; case '\\': out+='\\'; break; case '/': out+='/'; break; case 'b': out+='\b'; break; case 'f': out+='\f'; break; case 'n': out+='\n'; break; case 'r': out+='\r'; break; case 't': out+='\t'; break; default: out+=e; }
            } else out+=ch;
        }
        return out;
    }
    double parseNumber(Cursor& c){ skipWs(c); size_t start=c.i; while (c.i<c.s->size() && (std::isdigit((*c.s)[c.i])||(*c.s)[c.i]=='-'||(*c.s)[c.i]=='+'||(*c.s)[c.i]=='.'||(*c.s)[c.i]=='e'||(*c.s)[c.i]=='E')) ++c.i; return std::stod(c.s->substr(start,c.i-start)); }
    Value parseValue(Cursor& c);
    Value parseArray(Cursor& c){
        match(c,'['); std::vector<Value> arr; skipWs(c); if (match(c,']')) return Value(arr);
        while (true){ arr.push_back(parseValue(c)); skipWs(c); if (match(c,']')) break; match(c,','); }
        return Value(arr);
    }
    Value parseObject(Cursor& c){
        match(c,'{'); std::unordered_map<std::string,Value> obj; skipWs(c); if (match(c,'}')) return Value(obj);
        while (true){ std::string k = parseString(c); match(c,':'); Value v = parseValue(c); obj.emplace(k, v); skipWs(c); if (match(c,'}')) break; match(c,','); }
        return Value(obj);
    }
    Value parseValue(Cursor& c){ skipWs(c); if (c.i>=c.s->size()) return NONE_VALUE; char ch=(*c.s)[c.i];
        if (ch=='"') return Value(parseString(c));
        if (ch=='[') return parseArray(c);
        if (ch=='{') return parseObject(c);
        if (!c.s->compare(c.i,4,"true")) { c.i+=4; return Value(true);} 
        if (!c.s->compare(c.i,5,"false")) { c.i+=5; return Value(false);} 
        if (!c.s->compare(c.i,4,"null")) { c.i+=4; return NONE_VALUE;} 
        return Value(parseNumber(c));
    }

    std::string escapeString(const std::string& s){
        std::string out; out.reserve(s.size()+2); out.push_back('"');
        for(char ch: s){
            switch(ch){ case '"': out+="\\\""; break; case '\\': out+="\\\\"; break; case '\n': out+="\\n"; break; case '\r': out+="\\r"; break; case '\t': out+="\\t"; break; default: out+=ch; }
        }
        out.push_back('"'); return out;
    }
    std::string stringifyValue(const Value& v){
        switch(v.type){
            case VAL_NONE: return "null";
            case VAL_BOOLEAN: return v.asBoolean()?"true":"false";
            case VAL_NUMBER: return v.toString();
            case VAL_STRING: return escapeString(v.asString());
            case VAL_ARRAY: {
                const auto& a=v.asArray(); std::string out="["; for(size_t i=0;i<a.size();++i){ if(i) out+=","; out+=stringifyValue(a[i]); } out+="]"; return out;
            }
            case VAL_DICT: {
                const auto& d=v.asDict(); std::string out="{"; bool first=true; for(const auto& kv:d){ if(!first) out+=","; first=false; out+=escapeString(kv.first); out+=":"; out+=stringifyValue(kv.second);} out+="}"; return out;
            }
            default: return "null";
        }
    }
}

void registerJsonModule(Interpreter& interpreter) {
    interpreter.registerModule("json", [](Interpreter::ModuleBuilder& m) {
        m.fn("parse", [](std::vector<Value> a, int, int) -> Value {
            if (a.size() != 1 || !a[0].isString()) return NONE_VALUE;
            Cursor c{&a[0].asString(), 0};
            return parseValue(c);
        });
        m.fn("stringify", [](std::vector<Value> a, int, int) -> Value {
            if (a.size() != 1) return Value(std::string("null"));
            return Value(stringifyValue(a[0]));
        });
    });
}


