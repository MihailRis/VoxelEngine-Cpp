#ifndef CODERS_JSON_H_
#define CODERS_JSON_H_

#include <vector>
#include <string>
#include <stdint.h>
#include <stdexcept>
#include <unordered_map>

typedef unsigned int uint;

namespace json {
    typedef double number_t;
    class JObject;
    class JArray;
    class Value;

    extern std::string escape(std::string s);
    extern std::string stringify(JObject* obj, bool nice, std::string indent);
    class parsing_error : public std::runtime_error {
    public:
        std::string filename;
        std::string source;
        uint pos;
        uint line;
        uint linestart;

        parsing_error(std::string message, 
                      std::string filename, 
                      std::string source, 
                      uint pos, 
                      uint line, 
                      uint linestart);

        std::string errorLog() const;
    };

    enum class valtype {
        object, array, number, string, boolean
    };

    union valvalue {
        JObject* obj;
        JArray* arr;
        std::string* str;
        number_t num;
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
        number_t num(size_t index) const;
        JObject* obj(size_t index) const;
        JArray* arr(size_t index) const;
        bool flag(size_t index) const;

        inline size_t size() const {
            return values.size();
        }

        JArray& put(uint value);
        JArray& put(int value);
        JArray& put(float value);
        JArray& put(number_t value);
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
        void num(std::string key, number_t& dst) const;
        JObject* obj(std::string key) const;
        JArray* arr(std::string key) const;
        void flag(std::string key, bool& dst) const;

        JObject& put(std::string key, uint value);
        JObject& put(std::string key, int value);
        JObject& put(std::string key, float value);
        JObject& put(std::string key, number_t value);
        JObject& put(std::string key, std::string value);
        JObject& put(std::string key, JObject* value);
        JObject& put(std::string key, JArray* value);
        JObject& put(std::string key, bool value);
    };

    class Parser {
        std::string filename;
        std::string source;
        uint pos = 0;
        uint line = 0;
        uint linestart = 0;

        void skipWhitespace();
        void expect(char expected);
        char peek();
        char nextChar();
        bool hasNext();

        std::string parseName();
        int64_t parseSimpleInt(int base);
        number_t parseNumber(int sign);
        std::string parseString(char chr);

        JArray* parseArray();
        JObject* parseObject();
        Value* parseValue();

        parsing_error error(std::string message);

    public:
        Parser(std::string filename, std::string source);
        
        JObject* parse();
    };

    extern JObject* parse(std::string filename, std::string source);
    extern JObject* parse(std::string source);
}

#endif // CODERS_JSON_H_