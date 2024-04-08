#ifndef CODERS_TOML_H_
#define CODERS_TOML_H_

#include <string>
#include <vector>
#include <unordered_map>

#include "commons.h"

namespace toml {
    enum class fieldtype {
        ftbool,
        ftint,
        ftuint,
        ftfloat,
        ftdouble,
        ftstring,
    };

    struct Field {
        fieldtype type;
        void* ptr;
    };

    class Section {
        std::unordered_map<std::string, Field> fields;
        std::vector<std::string> keyOrder;
        std::string name;
        void add(std::string name, Field field);
    public:
        Section(std::string name);
        void add(std::string name, bool* ptr);
        void add(std::string name, int* ptr);
        void add(std::string name, uint* ptr);
        void add(std::string name, float* ptr);
        void add(std::string name, double* ptr);
        void add(std::string name, std::string* ptr);

        const Field* field(const std::string& name) const;

        void set(const std::string& name, double value);
        void set(const std::string& name, bool value);
        void set(const std::string& name, std::string value);
    
        const std::string& getName() const;
        const std::vector<std::string>& keys() const;
    };

    class Wrapper {
        std::unordered_map<std::string, Section*> sections;
        std::vector<std::string> keyOrder;
    public:
        ~Wrapper();
        Section& add(std::string section);
        Section* section(std::string name);

        std::string write() const;
    };

    class Reader : public BasicParser {
        Wrapper* wrapper;
        void skipWhitespace() override;
        void readSection(Section* section);
    public:
        Reader(Wrapper* wrapper, std::string file, std::string source);
        void read();
    };
}

#endif // CODERS_TOML_H_
