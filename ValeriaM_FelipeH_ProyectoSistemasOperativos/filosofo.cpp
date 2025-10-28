#include "filosofo.h"
#include <iostream>
#include <chrono>
#include <mutex>
#include <thread>

filosofo::filosofo(int id, tenedor& left, tenedor& right, camarero& waiter)
    : id_(id), left_tenedor_(left), right_tenedor_(right), waiter_(waiter) {
}

void filosofo::cenar() {
    using namespace std::chrono_literals;

    // Ciclos de vida del filósofo: PENSANDO -> HAMBRIENTO -> COMIENDO -> DORMIR
    for (int ciclo =0; ciclo <3; ++ciclo) {
        // PENSANDO
        std::cout << "Filosofo " << id_ << ": PENSANDO" << std::endl;
        std::this_thread::sleep_for(700ms);

        // HAMBRIENTO (intenta adquirir recursos)
        std::cout << "Filosofo " << id_ << ": HAMBRIENTO" << std::endl;

        // Solicitar permiso al camarero antes de intentar tomar los tenedores
        waiter_.solicitar_permiso();

        // Asimetría de orden de adquisición: pares D->I, impares I->D
        bool par = (id_ %2)==0;
#if __cplusplus >=201703L
        if (par) {
            // par: derecho luego izquierdo
            std::unique_lock<std::mutex> r(right_tenedor_.getMutex(), std::defer_lock);
            std::unique_lock<std::mutex> l(left_tenedor_.getMutex(), std::defer_lock);
            std::lock(r, l);
            // COMIENDO (ambos tenedores adquiridos)
            std::cout << "Filosofo " << id_ << ": COMIENDO" << std::endl;
            std::this_thread::sleep_for(700ms);
        } else {
            // impar: izquierdo luego derecho
            std::unique_lock<std::mutex> l(left_tenedor_.getMutex(), std::defer_lock);
            std::unique_lock<std::mutex> r(right_tenedor_.getMutex(), std::defer_lock);
            std::lock(l, r);
            // COMIENDO (ambos tenedores adquiridos)
            std::cout << "Filosofo " << id_ << ": COMIENDO" << std::endl;
            std::this_thread::sleep_for(700ms);
        }
#else
        if (par) {
            std::unique_lock<std::mutex> r(right_tenedor_.getMutex(), std::defer_lock);
            std::unique_lock<std::mutex> l(left_tenedor_.getMutex(), std::defer_lock);
            std::lock(r, l);
            // COMIENDO (ambos tenedores adquiridos)
            std::cout << "Filosofo " << id_ << ": COMIENDO" << std::endl;
            std::this_thread::sleep_for(700ms);
        } else {
            std::unique_lock<std::mutex> l(left_tenedor_.getMutex(), std::defer_lock);
            std::unique_lock<std::mutex> r(right_tenedor_.getMutex(), std::defer_lock);
            std::lock(l, r);
            // COMIENDO (ambos tenedores adquiridos)
            std::cout << "Filosofo " << id_ << ": COMIENDO" << std::endl;
            std::this_thread::sleep_for(700ms);
        }
#endif

        // Liberar permiso cuando termina de comer
        waiter_.liberar_permiso();

        // DORMIR (tras comer, antes de volver a pensar)
        std::cout << "Filosofo " << id_ << ": DORMIR" << std::endl;
        std::this_thread::sleep_for(600ms);
    }

    // Fin de ciclos de demostración
    std::cout << "Filosofo " << id_ << " ha terminado sus ciclos." << std::endl;
}
