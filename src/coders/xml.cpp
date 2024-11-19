#include "xml.hpp"

#include <charconv>
#include <sstream>
#include <stdexcept>
#include <utility>

#include "util/stringutil.hpp"

using namespace xml;

Attribute::Attribute(std::string name, std::string text)
    : name(std::move(name)), text(std::move(text)) {
}

const std::string& Attribute::getName() const {
    return name;
}

const std::string& Attribute::getText() const {
    return text;
}

int64_t Attribute::asInt() const {
    return std::stoll(text);
}

double Attribute::asFloat() const {
    return util::parse_double(text);
}

bool Attribute::asBool() const {
    return text == "true" || text == "1";
}

/* Read 2d vector formatted `x,y`*/
glm::vec2 Attribute::asVec2() const {
    size_t pos = text.find(',');
    if (pos == std::string::npos) {
        return glm::vec2(util::parse_double(text, 0, text.length()));
    }
    return glm::vec2(
        util::parse_double(text, 0, pos),
        util::parse_double(text, pos + 1, text.length() - pos - 1)
    );
}

/* Read 3d vector formatted `x,y,z`*/
glm::vec3 Attribute::asVec3() const {
    size_t pos1 = text.find(',');
    if (pos1 == std::string::npos) {
        return glm::vec3(util::parse_double(text, 0, text.length()));
    }
    size_t pos2 = text.find(',', pos1 + 1);
    if (pos2 == std::string::npos) {
        throw std::runtime_error("invalid vec3 value " + util::quote(text));
    }
    return glm::vec3(
        util::parse_double(text, 0, pos1),
        util::parse_double(text, pos1 + 1, pos2),
        util::parse_double(text, pos2 + 1, text.length() - pos2 - 1)
    );
}

/* Read 4d vector formatted `x,y,z,w`*/
glm::vec4 Attribute::asVec4() const {
    size_t pos1 = text.find(',');
    if (pos1 == std::string::npos) {
        return glm::vec4(util::parse_double(text, 0, text.length()));
    }
    size_t pos2 = text.find(',', pos1 + 1);
    if (pos2 == std::string::npos) {
        throw std::runtime_error("invalid vec4 value " + util::quote(text));
    }
    size_t pos3 = text.find(',', pos2 + 1);
    if (pos3 == std::string::npos) {
        throw std::runtime_error("invalid vec4 value " + util::quote(text));
    }
    return glm::vec4(
        util::parse_double(text, 0, pos1),
        util::parse_double(text, pos1 + 1, pos2 - pos1 - 1),
        util::parse_double(text, pos2 + 1, pos3 - pos2 - 1),
        util::parse_double(text, pos3 + 1, text.length() - pos3 - 1)
    );
}

/* Read RGBA color. Supported formats:
   - "#RRGGBB" or "#RRGGBBAA" hex color */
glm::vec4 Attribute::asColor() const {
    if (text[0] == '#') {
        if (text.length() != 7 && text.length() != 9) {
            throw std::runtime_error("#RRGGBB or #RRGGBBAA required");
        }
        int a = 255;
        int r = (hexchar2int(text[1]) << 4) | hexchar2int(text[2]);
        int g = (hexchar2int(text[3]) << 4) | hexchar2int(text[4]);
        int b = (hexchar2int(text[5]) << 4) | hexchar2int(text[6]);
        if (text.length() == 9) {
            a = (hexchar2int(text[7]) << 4) | hexchar2int(text[8]);
        }
        return glm::vec4(r / 255.f, g / 255.f, b / 255.f, a / 255.f);
    } else {
        return asVec4() / 255.f;
    }
}

Node::Node(std::string tag) : tag(std::move(tag)) {
}

void Node::add(const xmlelement& element) {
    elements.push_back(element);
}

void Node::set(const std::string& name, const std::string& text) {
    attrs[name] = Attribute(name, text);
}

const std::string& Node::getTag() const {
    return tag;
}

const xmlattribute& Node::attr(const std::string& name) const {
    auto found = attrs.find(name);
    if (found == attrs.end()) {
        throw std::runtime_error(
            "element <" + tag + " ...> missing attribute " + name
        );
    }
    return found->second;
}

xmlattribute Node::attr(const std::string& name, const std::string& def) const {
    auto found = attrs.find(name);
    if (found == attrs.end()) {
        return Attribute(name, def);
    }
    return found->second;
}

bool Node::has(const std::string& name) const {
    auto found = attrs.find(name);
    return found != attrs.end();
}

xmlelement Node::sub(size_t index) {
    return elements.at(index);
}

size_t Node::size() const {
    return elements.size();
}

const std::vector<xmlelement>& Node::getElements() const {
    return elements;
}

const xmlelements_map& Node::getAttributes() const {
    return attrs;
}

Document::Document(std::string version, std::string encoding)
    : version(std::move(version)), encoding(std::move(encoding)) {
}

void Document::setRoot(const xmlelement& element) {
    this->root = element;
}

xmlelement Document::getRoot() const {
    return root;
}

const std::string& Document::getVersion() const {
    return version;
}

const std::string& Document::getEncoding() const {
    return encoding;
}

Parser::Parser(std::string_view filename, std::string_view source)
    : BasicParser(filename, source) {
}

xmlelement Parser::parseOpenTag() {
    std::string tag = parseXMLName();
    auto node = std::make_shared<Node>(tag);

    char c;
    while (true) {
        skipWhitespace();
        c = peek();
        if (c == '/' || c == '>' || c == '?') break;
        std::string attrname = parseXMLName();
        std::string attrtext = "";
        skipWhitespace();
        if (peek() == '=') {
            nextChar();
            skipWhitespace();

            char quote = peek();
            if (quote != '\'' && quote != '"') {
                throw error("string literal expected");
            }
            skip(1);
            attrtext = parseString(quote);
        }
        node->set(attrname, attrtext);
    }
    return node;
}

void Parser::parseDeclaration() {
    std::string version = "1.0";
    std::string encoding = "UTF-8";
    expect('<');
    if (peek() == '?') {
        nextChar();
        xmlelement node = parseOpenTag();
        expect("?>");
        if (node->getTag() != "xml") {
            throw error("invalid declaration");
        }
        version = node->attr("version", version).getText();
        encoding = node->attr("encoding", encoding).getText();
        if (encoding != "utf-8" && encoding != "UTF-8") {
            throw error("UTF-8 encoding is only supported");
        }
    } else {
        goBack();
    }
    document = std::make_shared<Document>(version, encoding);
}

void Parser::parseComment() {
    expect("!--");
    if (skipTo("-->")) {
        skip(3);
    } else {
        throw error("comment close missing");
    }
}

std::string Parser::parseText() {
    size_t start = pos;
    while (hasNext()) {
        char c = peek();
        if (c == '<') {
            break;
        }
        nextChar();
    }
    return Parser("<string>", std::string(source.substr(start, pos - start)))
        .parseString('\0', false);
}

inline bool is_xml_identifier_start(char c) {
    return is_identifier_start(c) || c == ':';
}

inline bool is_xml_identifier_part(char c) {
    return is_identifier_part(c) || c == '-' || c == '.' || c == ':';
}

std::string Parser::parseXMLName() {
    char c = peek();
    if (!is_xml_identifier_start(c)) {
        throw error("identifier expected");
    }
    int start = pos;
    while (hasNext() && is_xml_identifier_part(source[pos])) {
        pos++;
    }
    return std::string(source.substr(start, pos - start));
}

xmlelement Parser::parseElement() {
    // text element
    if (peek() != '<') {
        auto element = std::make_shared<Node>("#");
        auto text = parseText();
        util::replaceAll(text, "&quot;", "\"");
        util::replaceAll(text, "&apos;", "'");
        util::replaceAll(text, "&lt;", "<");
        util::replaceAll(text, "&gt;", ">");
        util::replaceAll(text, "&amp;", "&");
        element->set("#", text);
        return element;
    }
    nextChar();

    // <!--element-->
    if (peek() == '!') {
        if (isNext("!DOCTYPE ")) {
            throw error("XML DTD is not supported yet");
        }
        parseComment();
        return nullptr;
    }

    auto element = parseOpenTag();
    char c = nextChar();

    // <element/>
    if (c == '/') {
        expect('>');
    }
    // <element>...</element>
    else if (c == '>') {
        skipWhitespace();
        while (!isNext("</")) {
            auto sub = parseElement();
            if (sub) {
                element->add(sub);
            }
            skipWhitespace();
        }
        skip(2);
        expect(element->getTag());
        expect('>');
    }
    // <element?>
    else {
        throw error("invalid syntax");
    }
    return element;
}

xmldocument Parser::parse() {
    parseDeclaration();

    xmlelement root = nullptr;
    while (root == nullptr) {
        root = parseElement();
    }
    document->setRoot(root);
    return document;
}

xmldocument xml::parse(const std::string& filename, const std::string& source) {
    Parser parser(filename, source);
    return parser.parse();
}

inline void newline(
    std::stringstream& ss, bool nice, const std::string& indentStr, int indent
) {
    if (!nice) return;
    ss << '\n';
    for (int i = 0; i < indent; i++) {
        ss << indentStr;
    }
}

static void stringifyElement(
    std::stringstream& ss,
    const xmlelement& element,
    bool nice,
    const std::string& indentStr,
    int indent
) {
    if (element->isText()) {
        std::string text = element->attr("#").getText();
        util::replaceAll(text, "&", "&amp;");
        util::replaceAll(text, "\"", "&quot;");
        util::replaceAll(text, "'", "&apos;");
        util::replaceAll(text, "<", "&lt;");
        util::replaceAll(text, ">", "&gt;");
        ss << text;
        return;
    }
    const std::string& tag = element->getTag();

    ss << '<' << tag;
    auto& attrs = element->getAttributes();
    if (!attrs.empty()) {
        ss << ' ';
        int count = 0;
        for (auto& entry : attrs) {
            auto attr = entry.second;
            ss << attr.getName();
            if (!attr.getText().empty()) {
                ss << "=" << util::escape(attr.getText());
            }
            if (count + 1 < int(attrs.size())) {
                ss << " ";
            }
            count++;
        }
    }
    auto& elements = element->getElements();
    if (elements.size() == 1 && elements[0]->isText()) {
        ss << ">";
        stringifyElement(ss, elements[0], nice, indentStr, indent + 1);
        ss << "</" << tag << ">";
        return;
    }
    if (!elements.empty()) {
        ss << '>';
        for (auto& sub : elements) {
            newline(ss, nice, indentStr, indent + 1);
            stringifyElement(ss, sub, nice, indentStr, indent + 1);
        }
        newline(ss, nice, indentStr, indent);
        ss << "</" << tag << ">";

    } else {
        ss << "/>";
    }
}

std::string xml::stringify(
    const xmldocument& document, bool nice, const std::string& indentStr
) {
    std::stringstream ss;

    // XML declaration
    ss << "<?xml version=\"" << document->getVersion();
    ss << "\" encoding=\"UTF-8\" ?>";
    newline(ss, nice, indentStr, 0);

    stringifyElement(ss, document->getRoot(), nice, indentStr, 0);

    return ss.str();
}
