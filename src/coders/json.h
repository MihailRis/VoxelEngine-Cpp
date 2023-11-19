#ifndef CODERS_JSON_H_
#define CODERS_JSON_H_

#include <vector>
#include <string>
#include <stdint.h>
#include <stdexcept>
#include <unordered_map>

#include "commons.h"

typedef unsigned int uint;

namespace json {
    class JObject;
    class JArray;
    class Value;

    extern std::string stringify(JObject* obj, bool nice, std::string indent);

    enum class valtype {
        object, array, number, string, boolean
    };

    union valvalue {
        JObject* obj;
        JArray* arr;
        std::string* str;
        double num;
        bool boolean;
    };

    class Value {
    public:
        valtype type;
        valvalue value;
        Value(valtype type, valvalue value);
        ~Value();
    };

    class JArray {
    public:
        std::vector<Value*> values;
        ~JArray();

        std::string str(size_t index) const;
        double num(size_t index) const;
        JObject* obj(size_t index) const;
        JArray* arr(size_t index) const;
        bool flag(size_t index) const;

        inline size_t size() const {
            return values.size();
        }

        JArray& put(uint value);
        JArray& put(int value);
        JArray& put(float value);
        JArray& put(double value);
        JArray& put(std::string value);
        JArray& put(JObject* value);
        JArray& put(JArray* value);
        JArray& put(bool value);
    };

    class JObject {
    public:
        std::unordered_map<std::string, Value*> map;
        ~JObject();

        void str(std::string key, std::string& dst) const;
        void num(std::string key, int& dst) const;
        void num(std::string key, float& dst) const;
        void num(std::string key, uint& dst) const;
        void num(std::string key, double& dst) const;
        JObject* obj(std::string key) const;
        JArray* arr(std::string key) const;
        void flag(std::string key, bool& dst) const;

        JObject& put(std::string key, uint value);
        JObject& put(std::string key, int value);
        JObject& put(std::string key, float value);
        JObject& put(std::string key, double value);
        JObject& put(std::string key, const char* value);
        JObject& put(std::string key, std::string value);
        JObject& put(std::string key, JObject* value);
        JObject& put(std::string key, JArray* value);
        JObject& put(std::string key, bool value);
    };

    class Parser : public BasicParser {
        JArray* parseArray();
        JObject* parseObject();
        Value* parseValue();
    public:
        Parser(std::string filename, std::string source);
        
        JObject* parse();
    };

    extern JObject* parse(std::string filename, std::string source);
    extern JObject* parse(std::string source);
}

#endif // CODERS_JSON_H_