#pragma once
#include "tenedor.h"
#include <string>
#include <thread>

class filosofo {
public:
    filosofo(int id, tenedor& left, tenedor& right);
    void cenar();

private:
    int id_;
    tenedor& left_tenedor_;
    tenedor& right_tenedor_;
    std::thread thread_;
};
