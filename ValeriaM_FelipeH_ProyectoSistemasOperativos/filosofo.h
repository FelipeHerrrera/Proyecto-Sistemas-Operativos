#pragma once
#include "tenedor.h"
#include "camarero.h"
#include <string>

class filosofo {
public:
    filosofo(int id, tenedor& left, tenedor& right, camarero& waiter);
    void cenar();

private:
    int id_;
    tenedor& left_tenedor_;
    tenedor& right_tenedor_;
    camarero& waiter_;
};
