#ifndef DISPOSABLE_H_
#define DISPOSABLE_H_

class Disposable {
public:
    virtual ~Disposable() {}
    virtual void dispose() = 0;
};

#endif // DISPOSABLE_H_
