#ifndef CODERS_XML_HPP_
#define CODERS_XML_HPP_

#include "commons.hpp"

#include <string>
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <unordered_map>

namespace xml {
    class Node;
    class Attribute;
    class Document;

    using xmlattribute = Attribute;
    using xmlelement = std::shared_ptr<Node>;
    using xmldocument = std::shared_ptr<Document>;
    using xmlelements_map = std::unordered_map<std::string, xmlattribute>;

    class Attribute {
        std::string name;
        std::string text;
    public:
        Attribute() {};
        Attribute(std::string name, std::string text);

        const std::string& getName() const;
        const std::string& getText() const;
        int64_t asInt() const;
        double asFloat() const;
        bool asBool() const;
        glm::vec2 asVec2() const;
        glm::vec3 asVec3() const;
        glm::vec4 asVec4() const;
        glm::vec4 asColor() const;
    };

    /// @brief XML element class. Text element has tag 'text' and attribute 'text'
    class Node {
        std::string tag;
        std::unordered_map<std::string, xmlattribute> attrs;
        std::vector<xmlelement> elements;
    public:
        Node(std::string tag);

        /// @brief Add sub-element
        void add(const xmlelement& element);

        /// @brief Set attribute value. Creates attribute if does not exists
        /// @param name attribute name
        /// @param text attribute value
        void set(const std::string& name, const std::string &text);
        
        /// @brief Get element tag
        const std::string& getTag() const;

        inline bool isText() const {
            return getTag() == "#";
        }

        inline const std::string& text() const {
            return attr("#").getText();
        }

        /// @brief Get attribute by name
        /// @param name attribute name
        /// @throws std::runtime_error if element has no attribute 
        /// @return xmlattribute - {name, value}
        const xmlattribute attr(const std::string& name) const;
        
        /// @brief Get attribute by name
        /// @param name attribute name
        /// @param def default value will be returned wrapped in xmlattribute
        /// if element has no attribute 
        /// @return xmlattribute - {name, value} or {name, def} if not found*/
        const xmlattribute attr(const std::string& name, const std::string& def) const;

        /// @brief Check if element has attribute
        /// @param name attribute name
        bool has(const std::string& name) const;

        /// @brief Get sub-element by index
        /// @param index sub-element index
        /// @throws std::out_of_range if an invalid index given
        /// @return sub-element
        xmlelement sub(size_t index);

        /// @brief Get number of sub-elements
        size_t size() const;

        const std::vector<xmlelement>& getElements() const;
        const xmlelements_map& getAttributes() const;
    };

    class Document {
        xmlelement root = nullptr;
        std::string version;
        std::string encoding;
    public:
        Document(std::string version, std::string encoding);

        void setRoot(const xmlelement &element);
        xmlelement getRoot() const;

        const std::string& getVersion() const;
        const std::string& getEncoding() const;
    };

    class Parser : BasicParser {
        xmldocument document;

        xmlelement parseOpenTag();
        xmlelement parseElement();
        void parseDeclaration();
        void parseComment();
        std::string parseText();
        std::string parseXMLName();
    public:
        Parser(std::string_view filename, std::string_view source);

        xmldocument parse();
    };

    /// @brief Serialize XML Document to string 
    /// @param document serializing document
    /// @param nice use human readable format (with indents and line-separators)
    /// @param indentStr indentation characters sequence (default - 4 spaces)
    /// @return XML string
    extern std::string stringify(
        const xmldocument& document,
        bool nice=true,
        const std::string& indentStr="    "
    );
    
    /// @brief Read XML Document from string
    /// @param filename file name will be shown in error messages
    /// @param source xml source code string
    /// @return xml document
    extern xmldocument parse(const std::string& filename, const std::string& source);
}

#endif // CODERS_XML_HPP_
