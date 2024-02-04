#include "gui_xml.h"

#include <charconv>
#include <stdexcept>

#include "panels.h"
#include "controls.h"

#include "../locale/langs.h"
#include "../../logic/scripting/scripting.h"
#include "../../util/stringutil.h"

using namespace gui;

static double readDouble(const std::string& str, size_t offset, size_t len) {
    double value;
    auto res = std::from_chars(str.data()+offset, str.data()+offset+len, value);
    if (res.ptr != str.data()+offset+len) {
        throw std::runtime_error("invalid number format "+escape_string(str));
    }
    return value;
}

/* Read 2d vector formatted `x,y`*/
static glm::vec2 readVec2(const std::string& str) {
    size_t pos = str.find(',');
    if (pos == std::string::npos) {
        throw std::runtime_error("invalid vec2 value "+escape_string(str));
    }
    return glm::vec2(
        readDouble(str, 0, pos),
        readDouble(str, pos+1, str.length()-pos-1)
    );
}

/* Read 3d vector formatted `x,y,z`*/
[[maybe_unused]]
static glm::vec3 readVec3(const std::string& str) {
    size_t pos1 = str.find(',');
    if (pos1 == std::string::npos) {
        throw std::runtime_error("invalid vec3 value "+escape_string(str));
    }
    size_t pos2 = str.find(',', pos1+1);
    if (pos2 == std::string::npos) {
        throw std::runtime_error("invalid vec3 value "+escape_string(str));
    }
    return glm::vec3(
        readDouble(str, 0, pos1),
        readDouble(str, pos1+1, pos2),
        readDouble(str, pos2+1, str.length()-pos2-1)
    );
}

/* Read 4d vector formatted `x,y,z,w`*/
static glm::vec4 readVec4(const std::string& str) {
    size_t pos1 = str.find(',');
    if (pos1 == std::string::npos) {
        throw std::runtime_error("invalid vec4 value "+escape_string(str));
    }
    size_t pos2 = str.find(',', pos1+1);
    if (pos2 == std::string::npos) {
        throw std::runtime_error("invalid vec4 value "+escape_string(str));
    }
    size_t pos3 = str.find(',', pos2+1);
    if (pos3 == std::string::npos) {
        throw std::runtime_error("invalid vec4 value "+escape_string(str));
    }
    return glm::vec4(
        readDouble(str, 0, pos1),
        readDouble(str, pos1+1, pos2-pos1-1),
        readDouble(str, pos2+1, pos3-pos2-1),
        readDouble(str, pos3+1, str.length()-pos3-1)
    );
}

/* Read RGBA color. Supported formats:
   - "#RRGGBB" or "#RRGGBBAA" hex color */
static glm::vec4 readColor(const std::string& str) {
    if (str[0] == '#') {
        if (str.length() != 7 && str.length() != 9) {
            throw std::runtime_error("#RRGGBB or #RRGGBBAA required");
        }
        int a = 255;
        int r = (hexchar2int(str[1]) << 4) | hexchar2int(str[2]);
        int g = (hexchar2int(str[3]) << 4) | hexchar2int(str[4]);
        int b = (hexchar2int(str[5]) << 4) | hexchar2int(str[6]);
        if (str.length() == 9) {
            a = (hexchar2int(str[7]) << 4) | hexchar2int(str[8]);
        }
        return glm::vec4(
            r / 255.f,
            g / 255.f,
            b / 255.f,
            a / 255.f
        );
    } else {
        throw std::runtime_error("hex colors are only supported");
    }
}

/* Read basic UINode properties */
static void readUINode(xml::xmlelement element, UINode& node) {
    if (element->has("coord")) {
        node.setCoord(readVec2(element->attr("coord").getText()));
    }
    if (element->has("size")) {
        node.setSize(readVec2(element->attr("size").getText()));
    }
    if (element->has("color")) {
        node.setColor(readColor(element->attr("color").getText()));
    }
}

static void _readContainer(UiXmlReader& reader, xml::xmlelement element, Container& container) {
    readUINode(element, container);

    for (auto& sub : element->getElements()) {
        if (sub->isText())
            continue;
        container.add(reader.readUINode(sub));
    }
}

static void _readPanel(UiXmlReader& reader, xml::xmlelement element, Panel& panel) {
    readUINode(element, panel);

    if (element->has("padding")) {
        panel.setPadding(readVec4(element->attr("padding").getText()));
    }

    if (element->has("margin")) {
        panel.setMargin(readVec4(element->attr("margin").getText()));
    }

    if (element->has("size")) {
        panel.setResizing(false);
    }

    for (auto& sub : element->getElements()) {
        if (sub->isText())
            continue;
        panel.add(reader.readUINode(sub));
    }
}


static std::wstring readAndProcessInnerText(xml::xmlelement element) {
    std::wstring text = L"";
    if (element->size() == 1) {
        text = util::str2wstr_utf8(element->sub(0)->attr("#").getText()); 
        if (text[0] == '@') {
            text = langs::get(text.substr(1));
        }
    }
    return text;
}

static std::shared_ptr<UINode> readLabel(UiXmlReader& reader, xml::xmlelement element) {
    std::wstring text = readAndProcessInnerText(element);
    auto label = std::make_shared<Label>(text);
    readUINode(element, *label);
    return label;
}

static std::shared_ptr<UINode> readContainer(UiXmlReader& reader, xml::xmlelement element) {
    auto container = std::make_shared<Container>(glm::vec2(), glm::vec2());
    _readContainer(reader, element, *container);
    return container;
}

static std::shared_ptr<UINode> readButton(UiXmlReader& reader, xml::xmlelement element) {
    std::wstring text = readAndProcessInnerText(element);
    auto button = std::make_shared<Button>(text, glm::vec4(0.0f), nullptr);
    _readPanel(reader, element, *button);

    if (element->has("onclick")) {
        runnable callback = scripting::create_runnable("<onclick>", element->attr("onclick").getText());
        button->listenAction([callback](GUI*) {
            callback();
        });
    }
    return button;
}

static std::shared_ptr<UINode> readTextBox(UiXmlReader& reader, xml::xmlelement element) {
    auto placeholder = util::str2wstr_utf8(element->attr("placeholder", "").getText());
    auto text = readAndProcessInnerText(element);
    auto textbox = std::make_shared<TextBox>(placeholder, glm::vec4(0.0f));
    _readPanel(reader, element, *textbox);
    textbox->setText(text);
    return textbox;
}

UiXmlReader::UiXmlReader() {
    add("label", readLabel);
    add("button", readButton);
    add("textbox", readTextBox);
    add("container", readContainer);
}

void UiXmlReader::add(const std::string& tag, uinode_reader reader) {
    readers[tag] = reader;
}

std::shared_ptr<UINode> UiXmlReader::readUINode(xml::xmlelement element) {
    const std::string& tag = element->getTag();

    auto found = readers.find(tag);
    if (found == readers.end()) {
        throw std::runtime_error("unsupported element '"+tag+"'");
    }
    return found->second(*this, element);
}

std::shared_ptr<UINode> UiXmlReader::readXML(
    const std::string& filename,
    const std::string& source
) {
    auto document = xml::parse(filename, source);
    auto root = document->getRoot();
    return readUINode(root);
}
