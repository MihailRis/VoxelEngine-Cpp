#ifndef OBJECTS_OBJECTS_HPP_
#define OBJECTS_OBJECTS_HPP_

#include "Object.hpp"
#include "../typedefs.hpp"

#include <memory>
#include <unordered_map>

class Objects {
    std::unordered_map<objectid_t, std::shared_ptr<Object>> objects;
    blockid_t nextId = 0;
public:
    void update(float delta);

    /// Spawns object of class T and returns pointer to it.
    /// @param T class that derives the Object class
    /// @param args pass arguments needed for T class constructor
    template<class T, typename... Args>
    std::shared_ptr<T> spawn(Args&&... args) {
        static_assert(std::is_base_of<Object, T>::value, "T must be a derived of Object class");
        std::shared_ptr<T> tObj = std::make_shared<T>(args...);
        
        blockid_t id = nextId++;
        std::shared_ptr<Object> obj = std::dynamic_pointer_cast<Object, T>(tObj);
        obj->objectUID = id;
        objects.emplace(id, obj);
        obj->spawned();
        return tObj;
    }

    template<class T>
    std::shared_ptr<T> get(blockid_t id) {
        static_assert(std::is_base_of<Object, T>::value, "T must be a derived of Object class");
        auto found = objects.find(id);
        if (found == objects.end()) {
            return nullptr;
        }
        std::shared_ptr<T> object = std::dynamic_pointer_cast<T>(found->second);
        return object; 
    }
    
    void clear() {
        objects.clear();
    }

    inline auto begin() {
        return objects.begin();
    }

    inline auto end() {
        return objects.end();
    }
};

#endif // OBJECTS_OBJECTS_HPP_
