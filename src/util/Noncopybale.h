//
// Created by chelovek on 11/24/23.
//

#ifndef NONCOPYBALE_H
#define NONCOPYBALE_H



class Noncopybale {
public:
    Noncopybale() = default;
    Noncopybale(const Noncopybale& other) = delete;

    Noncopybale(Noncopybale&& other) noexcept = delete;

    Noncopybale& operator=(const Noncopybale& other) = delete;

    Noncopybale& operator=(Noncopybale&& other) noexcept = delete;
};



#endif //NONCOPYBALE_H
