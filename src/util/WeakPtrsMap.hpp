#include <mutex>
#include <memory>
#include <unordered_map>

namespace util {
    template <typename K, typename V>
    class WeakPtrsMap {
        std::unordered_map<K, std::weak_ptr<V>> map;
        std::mutex mutex;
    public:
        std::weak_ptr<V>& operator[](const K& k) {
            return map[k];
        }

        std::shared_ptr<V> fetch(const K& k) {
            auto found = map.find(k);
            if (found == map.end()) {
                return nullptr;
            }
            auto ptr = found->second.lock();
            if (ptr == nullptr) {
                map.erase(found);
            }
            return ptr;
        }

        void erase(const K& k) {
            map.erase(k);
        }

        size_t size() const {
            return map.size();
        }

        void lock() {
            mutex.lock();
        }

        void unlock() {
            mutex.unlock();
        }
    };
}
