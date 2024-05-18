#ifndef GRAPHICS_UI_ELEMENTS_MENU_HPP_
#define GRAPHICS_UI_ELEMENTS_MENU_HPP_

#include "Container.hpp"

#include <stack>

namespace gui {
    struct Page {
        std::string name;
        std::shared_ptr<UINode> panel = nullptr;
    };

    using page_loader_func = std::function<std::shared_ptr<UINode>(const std::string& name)>;

    class Menu : public Container {
    protected:
        std::unordered_map<std::string, Page> pages;
        std::stack<Page> pageStack;
        Page current;
        std::unordered_map<std::string, supplier<std::shared_ptr<UINode>>> pageSuppliers;
        page_loader_func pagesLoader = nullptr;
    public:
        Menu();

        /// @brief Check menu have page or page supplier
        /// @param name page name
        bool has(const std::string& name);

        /// @brief Set current page to specified one.
        /// @param name page or page supplier name
        /// @param history previous page will not be saved in history if false
        void setPage(std::string name, bool history=true);
        void setPage(Page page, bool history=true);
        void addPage(std::string name, std::shared_ptr<UINode> panel);
        std::shared_ptr<UINode> fetchPage(const std::string& name);

        /// @brief Add page supplier used if page is not found
        /// @param name page name
        /// @param pageSupplier page supplier function
        void addSupplier(std::string name, supplier<std::shared_ptr<UINode>> pageSupplier);

        /// @brief Page loader is called if accessed page is not found 
        void setPageLoader(page_loader_func loader);

        /// @brief Set page to previous saved in history
        void back();

        /// @brief Clear pages history
        void clearHistory();

        /// @brief Clear history and remove and set page to null
        void reset();
    
        /// @brief Get current page
        Page& getCurrent();
    };
}

#endif // GRAPHICS_UI_ELEMENTS_MENU_HPP_
