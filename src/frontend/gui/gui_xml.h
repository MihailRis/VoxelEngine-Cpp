#ifndef FRONTEND_GUI_GUI_XML_H_
#define FRONTEND_GUI_GUI_XML_H_

#include <memory>
#include <unordered_map>

#include "GUI.h"
#include "../../coders/xml.h"

namespace gui {
    class UiXmlReader;

    using uinode_reader = std::function<std::shared_ptr<UINode>(UiXmlReader&, xml::xmlelement)>;

    class UiXmlReader {
        std::unordered_map<std::string, uinode_reader> readers;
    public:
        UiXmlReader();

        void add(const std::string& tag, uinode_reader reader);
        
        std::shared_ptr<UINode> readUINode(xml::xmlelement element);

        std::shared_ptr<UINode> readXML(
            const std::string& filename,
            const std::string& source
        );
    };
}

#endif // FRONTEND_GUI_GUI_XML_H_
