#include "gui_xml.hpp"

#include "elements/Panel.hpp"
#include "elements/Image.hpp"
#include "elements/Menu.hpp"
#include "elements/Button.hpp"
#include "elements/CheckBox.hpp"
#include "elements/TextBox.hpp"
#include "elements/TrackBar.hpp"
#include "elements/InputBindBox.hpp"
#include "elements/InventoryView.hpp"

#include "../../frontend/menu.hpp"
#include "../../frontend/locale.hpp"
#include "../../items/Inventory.hpp"
#include "../../logic/scripting/scripting.hpp"
#include "../../maths/voxmaths.hpp"
#include "../../util/stringutil.hpp"
#include "../../window/Events.hpp"

#include <stdexcept>
#include <utility>

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

static runnable create_runnable(
    const UiXmlReader& reader, 
    const xml::xmlelement& element,
    const std::string& name
) {
    if (element->has(name)) {
        std::string text = element->attr(name).getText();
        if (!text.empty()) {
            return scripting::create_runnable(
                reader.getEnvironment(), text, reader.getFilename()
            );
        }
    }
    return nullptr;
}

static onaction create_action(UiXmlReader& reader, const xml::xmlelement& element, const std::string& name) {
    auto callback = create_runnable(reader, element, name);
    if (callback == nullptr) {
        return nullptr;
    }
    return [callback](GUI*) {callback();};
}

/* Read basic UINode properties */
static void _readUINode(UiXmlReader& reader, const xml::xmlelement& element, UINode& node) {
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
        glm::vec4 pressedColor = color;
        if (element->has("hover-color")) {
            hoverColor = node.getHoverColor();
        }
        if (element->has("pressed-color")) {
            pressedColor = node.getPressedColor();
        }
        node.setColor(color);
        node.setHoverColor(hoverColor);
        node.setPressedColor(pressedColor);
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
    if (element->has("enabled")) {
        node.setEnabled(element->attr("enabled").asBool());
    }
    if (element->has("position-func")) {
        node.setPositionFunc(scripting::create_vec2_supplier(
            reader.getEnvironment(),
            element->attr("position-func").getText(),
            reader.getFilename()
        ));
    }
    if (element->has("size-func")) {
        node.setSizeFunc(scripting::create_vec2_supplier(
            reader.getEnvironment(),
            element->attr("size-func").getText(),
            reader.getFilename()
        ));
    }
    if (element->has("hover-color")) {
        node.setHoverColor(element->attr("hover-color").asColor());
    }
    if (element->has("pressed-color")) {
        node.setPressedColor(element->attr("pressed-color").asColor());
    }
    std::string alignName = element->attr("align", "").getText();
    node.setAlign(align_from_string(alignName, node.getAlign()));

    if (element->has("gravity")) {
        node.setGravity(gravity_from_string(
            element->attr("gravity").getText()
        ));
    }

    if (element->has("tooltip")) {
        node.setTooltip(util::str2wstr_utf8(element->attr("tooltip").getText()));
    }
    if (element->has("tooltip-delay")) {
        node.setTooltipDelay(element->attr("tooltip-delay").asFloat());
    }

    if (auto onclick = create_action(reader, element, "onclick")) {
        node.listenAction(onclick);
    }

    if (auto ondoubleclick = create_action(reader, element, "ondoubleclick")) {
        node.listenDoubleClick(ondoubleclick);
    }
}

static void _readContainer(UiXmlReader& reader, const xml::xmlelement& element, Container& container) {
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

void UiXmlReader::readUINode(UiXmlReader& reader, const xml::xmlelement& element, Container& container) {
    _readContainer(reader, element, container);
}

void UiXmlReader::readUINode(UiXmlReader& reader, const xml::xmlelement& element, UINode& node) {
    _readUINode(reader, element, node);
}

static void _readPanel(UiXmlReader& reader, const xml::xmlelement& element, Panel& panel, bool subnodes=true) {
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
    if (element->has("orientation")) {
        auto &oname = element->attr("orientation").getText();
        if (oname == "horizontal") {
            panel.setOrientation(Orientation::horizontal);
        }
    }
    if (subnodes) {
        for (auto& sub : element->getElements()) {
            if (sub->isText())
                continue;
            auto subnode = reader.readUINode(sub);
            if (subnode) {
                panel.add(subnode);
            }
        }
    }
}

static std::wstring readAndProcessInnerText(const xml::xmlelement& element, const std::string& context) {
    std::wstring text = L"";
    if (element->size() == 1) {
        std::string source = element->sub(0)->attr("#").getText();
        util::trim(source);
        text = util::str2wstr_utf8(source); 
        if (text[0] == '@') {
            if (context.empty()) {
                text = langs::get(text.substr(1));
            } else {
                text = langs::get(text.substr(1), util::str2wstr_utf8(context));
            }
        }
    }
    return text;
}

static std::shared_ptr<UINode> readLabel(UiXmlReader& reader, const xml::xmlelement& element) {
    std::wstring text = readAndProcessInnerText(element, reader.getContext());
    auto label = std::make_shared<Label>(text);
    _readUINode(reader, element, *label);
    if (element->has("valign")) {
        label->setVerticalAlign(
            align_from_string(element->attr("valign").getText(), label->getVerticalAlign())
        );
    }
    if (element->has("supplier")) {
        label->textSupplier(scripting::create_wstring_supplier(
            reader.getEnvironment(),
            element->attr("supplier").getText(),
            reader.getFilename()
        ));
    }
    if (element->has("autoresize")) {
        label->setAutoResize(element->attr("autoresize").asBool());
    }
    if (element->has("multiline")) {
        label->setMultiline(element->attr("multiline").asBool());
        if (!element->has("valign")) {
            label->setVerticalAlign(Align::top);
        }
    }
    if (element->has("text-wrap")) {
        label->setTextWrapping(element->attr("text-wrap").asBool());
    }
    return label;
}

static std::shared_ptr<UINode> readContainer(UiXmlReader& reader, const xml::xmlelement& element) {
    auto container = std::make_shared<Container>(glm::vec2());
    _readContainer(reader, element, *container);
    return container;
}

static std::shared_ptr<UINode> readPanel(UiXmlReader& reader, const xml::xmlelement& element) {
    float interval = element->attr("interval", "2").asFloat();
    auto panel = std::make_shared<Panel>(glm::vec2(), glm::vec4(), interval);
    _readPanel(reader, element, *panel);
    return panel;
}

static std::shared_ptr<UINode> readButton(UiXmlReader& reader, const xml::xmlelement& element) {
    glm::vec4 padding = element->attr("padding", "10").asVec4();

    std::shared_ptr<Button> button;
    auto& elements = element->getElements();
    if (!elements.empty() && elements[0]->getTag() != "#") {
        auto inner = reader.readUINode(element->getElements().at(0));
        if (inner != nullptr) {
            button = std::make_shared<Button>(inner, padding);
        } else {
            button = std::make_shared<Button>(L"", padding, nullptr);
        }
        _readPanel(reader, element, *button, false);
    } else {
        std::wstring text = readAndProcessInnerText(element, reader.getContext());
        button = std::make_shared<Button>(text, padding, nullptr);
        _readPanel(reader, element, *button, true);
    }
    if (element->has("text-align")) {
        button->setTextAlign(align_from_string(element->attr("text-align").getText(), button->getTextAlign()));
    }
    return button;
}

static std::shared_ptr<UINode> readCheckBox(UiXmlReader& reader, const xml::xmlelement& element) {
    auto text = readAndProcessInnerText(element, reader.getContext());
    bool checked = element->attr("checked", "false").asBool();
    auto checkbox = std::make_shared<FullCheckBox>(text, glm::vec2(32), checked);
    _readPanel(reader, element, *checkbox);

    if (element->has("consumer")) {
        checkbox->setConsumer(scripting::create_bool_consumer(
            reader.getEnvironment(),
            element->attr("consumer").getText(),
            reader.getFilename()
        ));
    }

    if (element->has("supplier")) {
        checkbox->setSupplier(scripting::create_bool_supplier(
            reader.getEnvironment(),
            element->attr("supplier").getText(),
            reader.getFilename()
        ));
    }
    return checkbox;
}

static std::shared_ptr<UINode> readTextBox(UiXmlReader& reader, const xml::xmlelement& element) {
    auto placeholder = util::str2wstr_utf8(element->attr("placeholder", "").getText());
    auto text = readAndProcessInnerText(element, reader.getContext());
    auto textbox = std::make_shared<TextBox>(placeholder, glm::vec4(0.0f));
    _readPanel(reader, element, *textbox);
    textbox->setText(text);

    if (element->has("multiline")) {
        textbox->setMultiline(element->attr("multiline").asBool());
    }
    if (element->has("text-wrap")) {
        textbox->setTextWrapping(element->attr("text-wrap").asBool());
    }
    if (element->has("editable")) {
        textbox->setEditable(element->attr("editable").asBool());
    }
    if (element->has("autoresize")) {
        textbox->setAutoResize(element->attr("autoresize").asBool());
    }
    if (element->has("consumer")) {
        textbox->setTextConsumer(scripting::create_wstring_consumer(
            reader.getEnvironment(),
            element->attr("consumer").getText(),
            reader.getFilename()
        ));
    }
    if (element->has("supplier")) {
        textbox->setTextSupplier(scripting::create_wstring_supplier(
            reader.getEnvironment(),
            element->attr("supplier").getText(),
            reader.getFilename()
        ));
    }
    if (element->has("focused-color")) {
        textbox->setFocusedColor(element->attr("focused-color").asColor());
    }
    if (element->has("error-color")) {
        textbox->setErrorColor(element->attr("error-color").asColor());
    }
    if (element->has("validator")) {
        textbox->setTextValidator(scripting::create_wstring_validator(
            reader.getEnvironment(),
            element->attr("validator").getText(),
            reader.getFilename()
        ));
    }
    if (auto onUpPressed = create_runnable(reader, element, "onup")) {
        textbox->setOnUpPressed(onUpPressed);
    }
    if (auto onDownPressed = create_runnable(reader, element, "ondown")) {
        textbox->setOnDownPressed(onDownPressed);
    }
    return textbox;
}

static std::shared_ptr<UINode> readImage(UiXmlReader& reader, const xml::xmlelement& element) {
    std::string src = element->attr("src", "").getText();
    auto image = std::make_shared<Image>(src);
    _readUINode(reader, element, *image);
    return image;
}

static std::shared_ptr<UINode> readTrackBar(UiXmlReader& reader, const xml::xmlelement& element) {
    const auto& env = reader.getEnvironment();
    const auto& file = reader.getFilename();
    float minv = element->attr("min", "0.0").asFloat();
    float maxv = element->attr("max", "1.0").asFloat();
    float def = element->attr("value", "0.0").asFloat();
    float step = element->attr("step", "1.0").asFloat();
    int trackWidth = element->attr("track-width", "12").asInt();
    auto bar = std::make_shared<TrackBar>(minv, maxv, def, step, trackWidth);
    _readUINode(reader, element, *bar);
    if (element->has("consumer")) {
        bar->setConsumer(scripting::create_number_consumer(
            env, element->attr("consumer").getText(), file));
    }
    if (element->has("sub-consumer")) {
        bar->setSubConsumer(scripting::create_number_consumer(
            env, element->attr("sub-consumer").getText(), file));
    }
    if (element->has("supplier")) {
        bar->setSupplier(scripting::create_number_supplier(
            env, element->attr("supplier").getText(), file));
    }
    if (element->has("track-color")) {
        bar->setTrackColor(element->attr("track-color").asColor());
    }
    if (element->has("change-on-release")) {
        bar->setChangeOnRelease(element->attr("change-on-release").asBool());
    }
    return bar;
}

static std::shared_ptr<UINode> readInputBindBox(UiXmlReader& reader, const xml::xmlelement& element) {
    auto bindname = element->attr("binding").getText();
    auto found = Events::bindings.find(bindname);
    if (found == Events::bindings.end()) {
        throw std::runtime_error("binding does not exists "+util::quote(bindname));
    }
    glm::vec4 padding = element->attr("padding", "6").asVec4();
    auto bindbox = std::make_shared<InputBindBox>(found->second, padding);
    _readPanel(reader, element, *bindbox);

    return bindbox;
}

static slotcallback readSlotFunc(InventoryView* view, UiXmlReader& reader, xml::xmlelement& element, const std::string& attr) {
    auto consumer = scripting::create_int_array_consumer(
        reader.getEnvironment(), 
        element->attr(attr).getText()
    );
    return [=](uint slot, ItemStack&) {
        int args[] {int(view->getInventory()->getId()), int(slot)};
        consumer(args, 2);
    };
}

static void readSlot(InventoryView* view, UiXmlReader& reader, xml::xmlelement element) {
    int index = element->attr("index", "0").asInt();
    bool itemSource = element->attr("item-source", "false").asBool();
    SlotLayout layout(index, glm::vec2(), true, itemSource, nullptr, nullptr, nullptr);
    if (element->has("pos")) {
        layout.position = element->attr("pos").asVec2();
    }
    if (element->has("updatefunc")) {
        layout.updateFunc = readSlotFunc(view, reader, element, "updatefunc");
    }
    if (element->has("sharefunc")) {
        layout.shareFunc = readSlotFunc(view, reader, element, "sharefunc");
    }
    if (element->has("onrightclick")) {
        layout.rightClick = readSlotFunc(view, reader, element, "onrightclick");
    }
    auto slot = view->addSlot(layout);
    reader.readUINode(reader, element, *slot);
    view->add(slot);
}

static void readSlotsGrid(InventoryView* view, UiXmlReader& reader, xml::xmlelement element) {
    int startIndex = element->attr("start-index", "0").asInt();
    int rows = element->attr("rows", "0").asInt();
    int cols = element->attr("cols", "0").asInt();
    int count = element->attr("count", "0").asInt();
    const int slotSize = InventoryView::SLOT_SIZE;
    int interval = element->attr("interval", "-1").asInt();
    if (interval < 0) {
        interval = InventoryView::SLOT_INTERVAL;
    }
    int padding = element->attr("padding", "-1").asInt();
    if (padding < 0) {
        padding = interval;
    }
    if (rows == 0) {
        rows = ceildiv(count, cols);
    } else if (cols == 0) {
        cols = ceildiv(count, rows);
    } else if (count == 0) {
        count = rows * cols;
    }
    bool itemSource = element->attr("item-source", "false").asBool();
    SlotLayout layout(-1, glm::vec2(), true, itemSource, nullptr, nullptr, nullptr);
    if (element->has("pos")) {
        layout.position = element->attr("pos").asVec2();
    }
    if (element->has("updatefunc")) {
        layout.updateFunc = readSlotFunc(view, reader, element, "updatefunc");
    }
    if (element->has("sharefunc")) {
        layout.shareFunc = readSlotFunc(view, reader, element, "sharefunc");
    }
    if (element->has("onrightclick")) {
        layout.rightClick = readSlotFunc(view, reader, element, "onrightclick");
    }
    layout.padding = padding;

    int idx = 0;
    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++, idx++) {
            if (idx >= count) {
                return;
            }
            SlotLayout slotLayout = layout;
            slotLayout.index = startIndex + idx;
            slotLayout.position += glm::vec2(
                padding + col * (slotSize + interval),
                padding + (rows-row-1) * (slotSize + interval)
            );
            auto slot = view->addSlot(slotLayout);
            view->add(slot, slotLayout.position);
        }
    }
}

static std::shared_ptr<UINode> readInventory(UiXmlReader& reader, const xml::xmlelement& element) {
    auto view = std::make_shared<InventoryView>();
    view->setColor(glm::vec4(0.122f, 0.122f, 0.122f, 0.878f)); // todo: fixme
    reader.addIgnore("slot");
    reader.addIgnore("slots-grid");
    reader.readUINode(reader, element, *view);

    for (auto& sub : element->getElements()) {
        if (sub->getTag() == "slot") {
            readSlot(view.get(), reader, sub);
        } else if (sub->getTag() == "slots-grid") {
            readSlotsGrid(view.get(), reader, sub);
        }
    }
    return view;
} 

static std::shared_ptr<UINode> readPageBox(UiXmlReader& reader, const xml::xmlelement& element) {
    auto menu = std::make_shared<Menu>();
    // fixme
    menu->setPageLoader(menus::create_page_loader(scripting::engine));
    _readContainer(reader, element, *menu);

    return menu;
}

UiXmlReader::UiXmlReader(const scriptenv& env) : env(env) {
    contextStack.emplace("");
    add("image", readImage);
    add("label", readLabel);
    add("panel", readPanel);
    add("button", readButton);
    add("textbox", readTextBox);
    add("pagebox", readPageBox);
    add("checkbox", readCheckBox);
    add("trackbar", readTrackBar);
    add("container", readContainer);
    add("bindbox", readInputBindBox);
    add("inventory", readInventory);
}

void UiXmlReader::add(const std::string& tag, uinode_reader reader) {
    readers[tag] = std::move(reader);
}

bool UiXmlReader::hasReader(const std::string& tag) const {
    return readers.find(tag) != readers.end();
}

void UiXmlReader::addIgnore(const std::string& tag) {
    ignored.insert(tag);
}

std::shared_ptr<UINode> UiXmlReader::readUINode(const xml::xmlelement& element) {
    if (element->has("if")) {
        const auto& cond = element->attr("if").getText();
        if (cond.empty() || cond == "false" || cond == "nil")
            return nullptr;
    }
    if (element->has("ifnot")) {
        const auto& cond = element->attr("ifnot").getText();
        if (!(cond.empty() || cond == "false" || cond == "nil"))
            return nullptr;
    }

    const std::string& tag = element->getTag();
    auto found = readers.find(tag);
    if (found == readers.end()) {
        if (ignored.find(tag) != ignored.end()) {
            return nullptr;
        }
        throw std::runtime_error("unsupported element '"+tag+"'");
    }

    bool hascontext = element->has("context");
    if (hascontext) {
        contextStack.push(element->attr("context").getText());
    }
    auto node = found->second(*this, element);
    if (hascontext) {
        contextStack.pop();
    }
    return node;
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
    const xml::xmlelement& root
) {
    this->filename = filename;
    return readUINode(root);
}

const std::string& UiXmlReader::getContext() const {
    return contextStack.top();
}

const std::string& UiXmlReader::getFilename() const {
    return filename;
}

const scriptenv& UiXmlReader::getEnvironment() const {
    return env;
}
