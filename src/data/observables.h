#ifndef DATA_OBSERVABLES_H_
#define DATA_OBSERVABLES_H_

#include "../delegates.h"

#include <vector>

template<class T>
class observable {
protected:
    T value;
    std::vector<consumer<T>> observers;
public:
    observable() : value() {}
    observable(T value) : value(value) {}
    virtual ~observable() {};

    virtual observable<T>& operator=(T newvalue) {
        value = newvalue;
        for (auto& observer : observers) {
            observer(newvalue);
        }
        return *this;
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

    virtual void observe(consumer<T> observer) {
        observers.push_back(observer);
    }
};

#endif // DATA_OBSERVABLES_H_
