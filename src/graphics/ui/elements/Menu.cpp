#include "Menu.hpp"

#include <stdexcept>

using namespace gui;

Menu::Menu() : Container(glm::vec2(1)){
}

bool Menu::has(const std::string& name) {
    return pages.find(name) != pages.end() ||
           pageSuppliers.find(name) != pageSuppliers.end();
}

void Menu::addPage(std::string name, std::shared_ptr<UINode> panel) {
    pages[name] = Page{name, panel};
}

void Menu::addSupplier(std::string name, supplier<std::shared_ptr<UINode>> pageSupplier) {
    pageSuppliers[name] = pageSupplier;
}

std::shared_ptr<UINode> Menu::fetchPage(const std::string& name) {
    auto found = pages.find(name);
    if (found == pages.end()) {
        auto supplier = pageSuppliers.find(name);
        if (supplier == pageSuppliers.end()) {
            if (pagesLoader) {
                return pagesLoader(name);
            }
            return nullptr;
        } else {
            return supplier->second();
            // supplied pages caching is not implemented
        }
    } else {
        return found->second.panel;
    }
}

void Menu::setPage(std::string name, bool history) {
    Page page {name, fetchPage(name)};
    if (page.panel == nullptr) {
        throw std::runtime_error("no page found");
    }
    setPage(page, history);
}

void Menu::setPage(Page page, bool history) {
    if (current.panel) {
        Container::remove(current.panel);
    }
    if (history) {
        pageStack.push(current);
    }
    current = page;
    Container::add(current.panel);
    setSize(current.panel->getSize());
}

void Menu::back() {
    if (pageStack.empty())
        return;
    Page page = pageStack.top();
    pageStack.pop();
    
    auto updated = fetchPage(page.name);
    if (updated) {
        page.panel = updated;
    }

    setPage(page, false);
}

void Menu::setPageLoader(page_loader_func loader) {
    pagesLoader = loader;
}

Page& Menu::getCurrent() {
    return current;
}

void Menu::clearHistory() {
    pageStack = std::stack<Page>();
}

void Menu::reset() {
    clearHistory();
    if (current.panel) {
        Container::remove(current.panel);
        current = Page{"", nullptr};
    }
}
