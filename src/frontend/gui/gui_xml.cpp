#include "gui_xml.h"

#include <charconv>
#include <stdexcept>

#include "panels.h"
#include "controls.h"

#include "../locale/langs.h"
#include "../../logic/scripting/scripting.h"
#include "../../util/stringutil.h"

using namespace gui;

static Align align_from_string(const std::string& str, Align def) {
    if (str == "left") return Align::left;
    if (str == "center") return Align::center;
    if (str == "right") return Align::right;
    return def;
}

/* Read basic UINode properties */
static void readUINode(xml::xmlelement element, UINode& node) {
    if (element->has("coord")) {
        node.setCoord(element->attr("coord").asVec2());
    }
    if (element->has("size")) {
        node.setSize(element->attr("size").asVec2());
    }
    if (element->has("color")) {
        node.setColor(element->attr("color").asColor());
    }
    if (element->has("margin")) {
        node.setMargin(element->attr("margin").asVec4());
    }

    std::string alignName = element->attr("align", "").getText();
    node.setAlign(align_from_string(alignName, node.getAlign()));
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
        panel.setPadding(element->attr("padding").asVec4());
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
        std::string source = element->sub(0)->attr("#").getText();
        util::trim(source);
        text = util::str2wstr_utf8(source); 
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
        auto callback = scripting::create_runnable("<onclick>", element->attr("onclick").getText());
        button->listenAction([callback](GUI*) {
            callback();
        });
    }
    if (element->has("text-align")) {
        button->setTextAlign(align_from_string(element->attr("text-align").getText(), button->getTextAlign()));
    }
    return button;
}

static std::shared_ptr<UINode> readTextBox(UiXmlReader& reader, xml::xmlelement element) {
    auto placeholder = util::str2wstr_utf8(element->attr("placeholder", "").getText());
    auto text = readAndProcessInnerText(element);
    auto textbox = std::make_shared<TextBox>(placeholder, glm::vec4(0.0f));
    _readPanel(reader, element, *textbox);
    textbox->setText(text);

    if (element->has("consumer")) {
        auto consumer = scripting::create_wstring_consumer(
            element->attr("consumer").getText(),
            reader.getFilename()
        );
        textbox->textConsumer(consumer);
    }
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
    this->filename = filename;
    auto document = xml::parse(filename, source);
    auto root = document->getRoot();
    return readUINode(root);
}

const std::string& UiXmlReader::getFilename() const {
    return filename;
}
