#include "filosofo.h"
#include <iostream>
#include <chrono>

filosofo::filosofo(int id, tenedor& left, tenedor& right)
    : id_(id), left_tenedor_(left), right_tenedor_(right) {
}

void filosofo::cenar() {
    std::cout << "Fil�sofo " << id_ << " est� pensando.\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    std::unique_lock<std::mutex> left_lock(left_tenedor_.getMutex());
    std::cout << "Fil�sofo " << id_ << " tom� el tenedor izquierdo.\n";

    std::unique_lock<std::mutex> right_lock(right_tenedor_.getMutex());
    std::cout << "Fil�sofo " << id_ << " tom� el tenedor derecho.\n";

    std::cout << "Fil�sofo " << id_ << " est� comiendo.\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}
