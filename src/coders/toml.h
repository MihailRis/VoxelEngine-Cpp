#ifndef CODERS_TOML_H_
#define CODERS_TOML_H_

#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <unordered_map>

#include "commons.h"

namespace toml {
    enum class fieldtype {
        ftbool,
        ftint,
        ftuint,
        ftfloat,
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
        Section() = default;
        Section(std::string name);
        void add(std::string name, bool* ptr);
        void add(std::string name, int* ptr);
        void add(std::string name, uint* ptr);
        void add(std::string name, float* ptr);
        void add(std::string name, std::string* ptr);

        std::optional<std::reference_wrapper<const Field>> field(std::string name) const;

        void set(std::string name, double value);
        void set(std::string name, bool value);
        void set(std::string name, std::string value);

        std::string getName() const;
        const std::vector<std::string>& keys() const;
    };

    class Wrapper {
        std::unordered_map<std::string, Section> sections;
        std::vector<std::string> keyOrder;
    public:
        ~Wrapper() = default;
        Section& add(std::string section);
        std::optional<std::reference_wrapper<Section>> section(std::string name);

        std::string write() const;
    };

    class Reader : public BasicParser {
        Wrapper& wrapper;
        void skipWhitespace() override;
        void readSection(std::optional<std::reference_wrapper<Section>> section);
    public:
        Reader(Wrapper& wrapper, std::string file, std::string source);
        void read();
    };
}

#endif // CODERS_TOML_H_
