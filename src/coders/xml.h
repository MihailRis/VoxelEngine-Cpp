#ifndef CODERS_XML_H_
#define CODERS_XML_H_

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

#include "commons.h"

namespace xml {
    class Node;
    class Attribute;
    class Document;

    typedef Attribute xmlattribute;
    typedef std::shared_ptr<Node> xmlelement;
    typedef std::shared_ptr<Document> xmldocument;
    typedef std::unordered_map<std::string, xmlattribute> xmlelements_map;

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
    };

    /* XML element class. Text element has tag 'text' and attribute 'text' */
    class Node {
        std::string tag;
        std::unordered_map<std::string, xmlattribute> attrs;
        std::vector<xmlelement> elements;
    public:
        Node(std::string tag);

        /* Add sub-element */
        void add(xmlelement element);
        
        /* Set attribute value. Creates attribute if does not exists */
        void set(std::string name, std::string text);

        /* Get element tag */
        const std::string& getTag() const;

        inline bool isText() const {
            return getTag() == "#";
        }

        inline const std::string& text() const {
            return attr("#").getText();
        }

        /* Get attribute by name
           @param name attribute name
           @throws std::runtime_error if element has no attribute 
           @return xmlattribute - {name, value} */
        const xmlattribute attr(const std::string& name) const;
        /* Get attribute by name
           @param name name
           @param def default value will be returned wrapped in xmlattribute 
           if element has no attribute 
           @return xmlattribute - {name, value} or {name, def} if not found*/
        const xmlattribute attr(const std::string& name, const std::string& def) const;

        /* Check if element has attribute
           @param name attribute name */
        bool has(const std::string& name) const;

        /* Get sub-element by index
           @throws std::out_of_range if an invalid index given */
        xmlelement sub(size_t index);

        /* Get number of sub-elements */
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

        void setRoot(xmlelement element);
        xmlelement getRoot() const;

        const std::string& getVersion() const;
        const std::string& getEncoding() const;
    };

    class Parser : public BasicParser {
        xmldocument document;

        xmlelement parseOpenTag();
        xmlelement parseElement();
        void parseDeclaration();
        void parseComment();
        std::string parseText();
        std::string parseXMLName();
    public:
        Parser(std::string filename, std::string source);

        xmldocument parse();
    };

    /* Serialize XML Document to string 
       @param document serializing document
       @param nice use human readable format 
       (with indents and line-separators)
       @param indentStr indentation characters sequence
       (default - 4 spaces)*/
    extern std::string stringify(
        const xmldocument document,
        bool nice=true,
        const std::string& indentStr="    "
    );

    /* Read XML Document from string
       @param filename file name will be shown in error messages
       @param source xml source code string */
    extern xmldocument parse(std::string filename, std::string source);
}

#endif // CODERS_XML_H_
