#include "filosofo.h"
#include <iostream>
#include <chrono>
#include <mutex>
#include <thread>

filosofo::filosofo(int id, tenedor& left, tenedor& right, camarero& waiter)
    : id_(id), left_tenedor_(left), right_tenedor_(right), waiter_(waiter) {
}

void filosofo::cenar() {
    std::cout << "Filosofo " << id_ << " esta pensando.\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    // Solicitar permiso al camarero antes de intentar tomar los tenedores
    waiter_.solicitar_permiso();

    // Bloquear ambos tenedores de forma atómica. En C++17 usamos std::scoped_lock.
#if __cplusplus >= 201703L
    {
        std::scoped_lock lock(left_tenedor_.getMutex(), right_tenedor_.getMutex());
        std::cout << "Filosofo " << id_ << " tomo ambos tenedores.\n";
        std::cout << "Filosofo " << id_ << " esta comiendo.\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    } // se liberan ambos mutex al salir del scope
#else
    {
        std::unique_lock<std::mutex> l1(left_tenedor_.getMutex(), std::defer_lock);
        std::unique_lock<std::mutex> l2(right_tenedor_.getMutex(), std::defer_lock);
        std::lock(l1, l2);
        std::cout << "Filosofo " << id_ << " tomo ambos tenedores.\n";
        std::cout << "Filosofo " << id_ << " esta comiendo.\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
#endif

    // Liberar permiso cuando termina de comer
    waiter_.liberar_permiso();
}
