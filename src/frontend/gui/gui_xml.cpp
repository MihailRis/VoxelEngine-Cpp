#include "gui_xml.h"

#include <charconv>
#include <stdexcept>

#include "containers.h"
#include "controls.h"

#include "../../assets/AssetsLoader.h"
#include "../locale/langs.h"
#include "../../logic/scripting/scripting.h"
#include "../../util/stringutil.h"

using namespace gui;

static Align align_from_string(const std::string& str, Align def) {
    if (str == "left") return Align::left;
    if (str == "center") return Align::center;
    if (str == "right") return Align::right;
    if (str == "top") return Align::top;
    if (str == "bottom") return Align::bottom;
    return def;
}

static Gravity gravity_from_string(const std::string& str) {
    static const std::unordered_map<std::string, Gravity> gravity_names {
        {"top-left", Gravity::top_left},
        {"top-center", Gravity::top_center},
        {"top-right", Gravity::top_right},
        {"center-left", Gravity::center_left},
        {"center-center", Gravity::center_center},
        {"center-right", Gravity::center_right},
        {"bottom-left", Gravity::bottom_left},
        {"bottom-center", Gravity::bottom_center},
        {"bottom-right", Gravity::bottom_right},
    };
    auto found = gravity_names.find(str);
    if (found != gravity_names.end()) {
        return found->second;
    }
    return Gravity::none;
}

/* Read basic UINode properties */
static void _readUINode(UiXmlReader& reader, xml::xmlelement element, UINode& node) {
    if (element->has("id")) {
        node.setId(element->attr("id").getText());
    }
    if (element->has("pos")) {
        node.setPos(element->attr("pos").asVec2());
    }
    if (element->has("size")) {
        node.setSize(element->attr("size").asVec2());
    }
    if (element->has("color")) {
        glm::vec4 color = element->attr("color").asColor();
        glm::vec4 hoverColor = color;
        if (element->has("hover-color")) {
            hoverColor = node.getHoverColor();
        }
        node.setColor(color);
        node.setHoverColor(hoverColor);
    }
    if (element->has("margin")) {
        node.setMargin(element->attr("margin").asVec4());
    }
    if (element->has("z-index")) {
        node.setZIndex(element->attr("z-index").asInt());
    }
    if (element->has("interactive")) {
        node.setInteractive(element->attr("interactive").asBool());
    }
    if (element->has("visible")) {
        node.setVisible(element->attr("visible").asBool());
    }
    if (element->has("position-func")) {
        auto supplier = scripting::create_vec2_supplier(
            reader.getEnvironment().getId(),
            element->attr("position-func").getText(),
            reader.getFilename()+".lua"
        );
        node.setPositionFunc(supplier);
    }
    if (element->has("hover-color")) {
        node.setHoverColor(element->attr("hover-color").asColor());
    }
    std::string alignName = element->attr("align", "").getText();
    node.setAlign(align_from_string(alignName, node.getAlign()));

    if (element->has("gravity")) {
        node.setGravity(gravity_from_string(
            element->attr("gravity").getText()
        ));
    }
}


static void _readContainer(UiXmlReader& reader, xml::xmlelement element, Container& container) {
    _readUINode(reader, element, container);

    if (element->has("scrollable")) {
        container.setScrollable(element->attr("scrollable").asBool());
    }
    for (auto& sub : element->getElements()) {
        if (sub->isText())
            continue;
        auto subnode = reader.readUINode(sub);
        if (subnode) {
            container.add(subnode);
        }
    }
}

void UiXmlReader::readUINode(UiXmlReader& reader, xml::xmlelement element, Container& container) {
    _readContainer(reader, element, container);
}

void UiXmlReader::readUINode(UiXmlReader& reader, xml::xmlelement element, UINode& node) {
    _readUINode(reader, element, node);
}

static void _readPanel(UiXmlReader& reader, xml::xmlelement element, Panel& panel) {
    _readUINode(reader, element, panel);

    if (element->has("padding")) {
        glm::vec4 padding = element->attr("padding").asVec4();
        panel.setPadding(padding);
        glm::vec2 size = panel.getSize();
        panel.setSize(glm::vec2(
            size.x + padding.x + padding.z,
            size.y + padding.y + padding.w
        ));
    }
    if (element->has("size")) {
        panel.setResizing(false);
    }
    if (element->has("max-length")) {
        panel.setMaxLength(element->attr("max-length").asInt());
    }
    for (auto& sub : element->getElements()) {
        if (sub->isText())
            continue;
        auto subnode = reader.readUINode(sub);
        if (subnode) {
            panel.add(subnode);
        }
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
    _readUINode(reader, element, *label);
    if (element->has("valign")) {
        label->setVerticalAlign(
            align_from_string(element->attr("valign").getText(), label->getVerticalAlign())
        );
    }
    return label;
}

static std::shared_ptr<UINode> readContainer(UiXmlReader& reader, xml::xmlelement element) {
    auto container = std::make_shared<Container>(glm::vec2());
    _readContainer(reader, element, *container);
    return container;
}

static std::shared_ptr<UINode> readPanel(UiXmlReader& reader, xml::xmlelement element) {
    float interval = element->attr("interval", "2").asFloat();
    auto panel = std::make_shared<Panel>(glm::vec2(), glm::vec4(), interval);
    _readPanel(reader, element, *panel);
    return panel;
}

static std::shared_ptr<UINode> readButton(UiXmlReader& reader, xml::xmlelement element) {
    std::wstring text = readAndProcessInnerText(element);
    auto button = std::make_shared<Button>(text, glm::vec4(0.0f), nullptr);
    _readPanel(reader, element, *button);

    if (element->has("onclick")) {
        auto callback = scripting::create_runnable(
            reader.getEnvironment().getId(),
            element->attr("onclick").getText(),
            reader.getFilename()
        );
        button->listenAction([callback](GUI*) {
            callback();
        });
    }
    if (element->has("text-align")) {
        button->setTextAlign(align_from_string(element->attr("text-align").getText(), button->getTextAlign()));
    }
    if (element->has("pressed-color")) {
        button->setPressedColor(element->attr("pressed-color").asColor());
    }
    return button;
}

static std::shared_ptr<UINode> readCheckBox(UiXmlReader& reader, xml::xmlelement element) {
    auto text = readAndProcessInnerText(element);
    bool checked = element->attr("checked", "false").asBool();
    auto checkbox = std::make_shared<FullCheckBox>(text, glm::vec2(), checked);
    _readPanel(reader, element, *checkbox);

    if (element->has("consumer")) {
        auto consumer = scripting::create_bool_consumer(
            reader.getEnvironment().getId(),
            element->attr("consumer").getText(),
            reader.getFilename()
        );
        checkbox->setConsumer(consumer);
    }

    if (element->has("supplier")) {
        auto supplier = scripting::create_bool_supplier(
            reader.getEnvironment().getId(),
            element->attr("supplier").getText(),
            reader.getFilename()
        );
        checkbox->setSupplier(supplier);
    }
    return checkbox;
}

static std::shared_ptr<UINode> readTextBox(UiXmlReader& reader, xml::xmlelement element) {
    auto placeholder = util::str2wstr_utf8(element->attr("placeholder", "").getText());
    auto text = readAndProcessInnerText(element);
    auto textbox = std::make_shared<TextBox>(placeholder, glm::vec4(0.0f));
    _readPanel(reader, element, *textbox);
    textbox->setText(text);

    if (element->has("multiline")) {
        textbox->setMultiline(element->attr("multiline").asBool());
    }

    if (element->has("editable")) {
        textbox->setEditable(element->attr("editable").asBool());
    }
    
    if (element->has("consumer")) {
        auto consumer = scripting::create_wstring_consumer(
            reader.getEnvironment().getId(),
            element->attr("consumer").getText(),
            reader.getFilename()
        );
        textbox->setTextConsumer(consumer);
    }

    if (element->has("supplier")) {
        auto supplier = scripting::create_wstring_supplier(
            reader.getEnvironment().getId(),
            element->attr("supplier").getText(),
            reader.getFilename()
        );
        textbox->setTextSupplier(supplier);
    }
    if (element->has("focused-color")) {
        textbox->setFocusedColor(element->attr("focused-color").asColor());
    }
    if (element->has("error-color")) {
        textbox->setErrorColor(element->attr("error-color").asColor());
    }
    if (element->has("validator")) {
        auto validator  = scripting::create_wstring_validator(
            reader.getEnvironment().getId(),
            element->attr("validator").getText(),
            reader.getFilename()
        );
        textbox->setTextValidator(validator);
    }
    return textbox;
}

static std::shared_ptr<UINode> readImage(UiXmlReader& reader, xml::xmlelement element) {
    std::string src = element->attr("src", "").getText();
    auto image = std::make_shared<Image>(src);
    _readUINode(reader, element, *image);
    reader.getAssetsLoader().add(AssetType::texture, "textures/"+src, src, nullptr);
    return image;
}

static std::shared_ptr<UINode> readTrackBar(UiXmlReader& reader, xml::xmlelement element) {
    float min = element->attr("min", "0.0").asFloat();
    float max = element->attr("max", "1.0").asFloat();
    float def = element->attr("value", "0.0").asFloat();
    float step = element->attr("step", "1.0").asFloat();
    int trackWidth = element->attr("track-width", "1.0").asInt();
    auto bar = std::make_shared<TrackBar>(min, max, def, step, trackWidth);
    _readUINode(reader, element, *bar);
    if (element->has("consumer")) {
        auto consumer = scripting::create_number_consumer(
            reader.getEnvironment().getId(),
            element->attr("consumer").getText(),
            reader.getFilename()
        );
        bar->setConsumer(consumer);
    }
    if (element->has("supplier")) {
        auto supplier = scripting::create_number_supplier(
            reader.getEnvironment().getId(),
            element->attr("supplier").getText(),
            reader.getFilename()
        );
        bar->setSupplier(supplier);
    }
    if (element->has("track-color")) {
        bar->setTrackColor(element->attr("track-color").asColor());
    }
    return bar;
}

UiXmlReader::UiXmlReader(const scripting::Environment& env, AssetsLoader& assetsLoader) 
: env(env), assetsLoader(assetsLoader)
{
    add("image", readImage);
    add("label", readLabel);
    add("panel", readPanel);
    add("button", readButton);
    add("textbox", readTextBox);
    add("chackbox", readCheckBox);
    add("trackbar", readTrackBar);
    add("container", readContainer);
}

void UiXmlReader::add(const std::string& tag, uinode_reader reader) {
    readers[tag] = reader;
}

bool UiXmlReader::hasReader(const std::string& tag) const {
    return readers.find(tag) != readers.end();
}

void UiXmlReader::addIgnore(const std::string& tag) {
    ignored.insert(tag);
}

std::shared_ptr<UINode> UiXmlReader::readUINode(xml::xmlelement element) {
    const std::string& tag = element->getTag();

    auto found = readers.find(tag);
    if (found == readers.end()) {
        if (ignored.find(tag) != ignored.end()) {
            return nullptr;
        }
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

std::shared_ptr<UINode> UiXmlReader::readXML(
    const std::string& filename,
    xml::xmlelement root
) {
    this->filename = filename;
    return readUINode(root);
}

const std::string& UiXmlReader::getFilename() const {
    return filename;
}

const scripting::Environment& UiXmlReader::getEnvironment() const {
    return env;
}

AssetsLoader& UiXmlReader::getAssetsLoader() {
    return assetsLoader;
}
