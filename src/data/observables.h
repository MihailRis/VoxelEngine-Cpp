#ifndef DATA_OBSERVABLES_H_
#define DATA_OBSERVABLES_H_

#include "../delegates.h"

#include <vector>

template<class T>
class Observer;

template<class T> 
using observer_func = std::function<void(Observer<T>*, T)>;

template<class T>
class Observer {
    bool attached = true;
    observer_func<T> callback;
public:
    Observer(observer_func<T> callback) : callback(callback) {}

    void update(T value) {
        callback(this, value);
    }

    bool isAttached() const {
        return attached;
    }

    void detach() {
        attached = false;
    }
};

template<class T>
class observable {
protected:
    T value;
    std::vector<Observer<T>> observers;
public:
    observable() : value() {}
    observable(T value) : value(value) {}
    virtual ~observable() {};

    virtual observable<T>& operator=(T newvalue) {
        value = newvalue;
        auto it = observers.begin();
        while (it != observers.end()) {
            if (it->isAttached()) {
                it->update(newvalue);
                it++;
            } else {
                it = observers.erase(it);
            }
        }
        return *this;
    }

    virtual bool operator==(const T& other) const {
        return value == other;
    }

    virtual T& get() {
        return value;
    }

    virtual T& operator*() {
        return value;
    }

    virtual operator T() const {
        return value;
    }

    virtual void observe(observer_func<T> callback) {
        Observer observer(callback);
        observer.update(value);
        if (observer.isAttached()) {
            observers.push_back(observer);
        }
    }
};

#endif // DATA_OBSERVABLES_H_
