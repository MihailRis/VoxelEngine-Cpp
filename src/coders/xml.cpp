#include "xml.h"

#include <stdexcept>
#include <sstream>
#include "../util/stringutil.h"

using namespace xml;

Attribute::Attribute(std::string name, std::string text)
    : name(name), 
      text(text) {
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
    return std::stod(text);
}

bool Attribute::asBool() const {
    return text == "true" || text == "1";
}

Node::Node(std::string tag) : tag(tag) {
}

void Node::add(xmlelement element) {
    elements.push_back(element);
}

void Node::set(std::string name, std::string text) {
    attrs.insert_or_assign(name, Attribute(name, text));
}

const std::string& Node::getTag() const {
    return tag;
}

const xmlattribute Node::attr(const std::string& name) const {
    auto found = attrs.find(name);
    if (found == attrs.end()) {
        throw std::runtime_error("element <"+tag+" ...> missing attribute "+name);
    }
    return found->second;
}

const xmlattribute Node::attr(const std::string& name, const std::string& def) const {
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
    : version(version),
      encoding(encoding) {
}

void Document::setRoot(xmlelement element) {
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

Parser::Parser(std::string filename, std::string source) 
    : BasicParser(filename, source) {
}

xmlelement Parser::parseOpenTag() {
    std::string tag = parseName();
    auto node = std::make_shared<Node>(tag);

    char c;
    while (true) {
        skipWhitespace();
        c = peek();
        if (c == '/' || c == '>' || c == '?')
            break;
        std::string attrname = parseName();
        std::string attrtext = "";
        skipWhitespace();
        if (peek() == '=') {
            nextChar();
            skipWhitespace();
            expect('"');
            attrtext = parseString('"');
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
    return source.substr(start, pos-start);
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
    document->setRoot(parseElement());
    return document;
}

xmldocument xml::parse(std::string filename, std::string source) {
    Parser parser(filename, source);
    return parser.parse();
}

inline void newline(
    std::stringstream& ss,
    bool nice,
    const std::string& indentStr,
    int indent
) {
    if (!nice)
        return;
    ss << '\n';
    for (int i = 0; i < indent; i++) {
        ss << indentStr;
    }
}

static void stringifyElement(
    std::stringstream& ss,
    const xmlelement element,
    bool nice,
    const std::string& indentStr,
    int indent
) {
    if (element->isText()) {
        std::string text = element->attr("#").getText();
        util::replaceAll(text, "&", "&amp;");
        util::replaceAll(text, "\"","&quot;");
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
                ss << "=" << escape_string(attr.getText());
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
        stringifyElement(ss, elements[0], nice, indentStr, indent+1);
        ss << "</" << tag << ">";
        return;
    }
    if (!elements.empty()) {
        ss << '>';
        for (auto& sub : elements) {
            newline(ss, nice, indentStr, indent+1);
            stringifyElement(ss, sub, nice, indentStr, indent+1);
        }
        newline(ss, nice, indentStr, indent);
        ss << "</" << tag << ">";
        
    } else {
        ss << "/>";
    }
    
}

std::string xml::stringify(
    const xmldocument document,
    bool nice,
    const std::string& indentStr
) {
    std::stringstream ss;
    
    // XML declaration
    ss << "<?xml version=\"" << document->getVersion();
    ss << "\" encoding=\"UTF-8\" ?>";
    newline(ss, nice, indentStr, 0);

    stringifyElement(ss, document->getRoot(), nice, indentStr, 0);

    return ss.str();
}
