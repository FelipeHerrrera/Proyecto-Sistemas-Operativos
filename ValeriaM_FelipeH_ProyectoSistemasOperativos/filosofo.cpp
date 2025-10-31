#include "filosofo.h"
#include <iostream>
#include <chrono>
#include <mutex>
#include <thread>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <string>

namespace {
    std::mutex g_log_mtx;
    std::once_flag g_log_header_once;

    void print_log_header_unlocked() {
        std::cout << "\n===== LOG DE ESTADOS =====\n";
        std::cout << "+---------+-----------+-----------+\n";
        std::cout << "| Tiempo  | Filosofo  | Estado    |\n";
        std::cout << "+---------+-----------+-----------+\n";
    }
    void print_log_header() {
        std::lock_guard<std::mutex> lk(g_log_mtx);
        print_log_header_unlocked();
    }
}

std::chrono::steady_clock::time_point filosofo::start_{};
std::chrono::steady_clock::time_point filosofo::end_{};

filosofo::filosofo(int id, tenedor& left, tenedor& right, camarero& waiter)
    : id_(id), left_tenedor_(left), right_tenedor_(right), waiter_(waiter) {
}

void filosofo::log_estado(const char* estado) const {
    using clock = std::chrono::steady_clock;
    auto now = clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_).count();

    std::lock_guard<std::mutex> lk(g_log_mtx);
    std::call_once(g_log_header_once, print_log_header_unlocked);

    std::ostringstream oss;
    oss << "| " << std::setw(7) << ms << "ms | "
        << "Filosofo " << std::setw(2) << id_ << " | "
        << std::left << std::setw(10) << estado << "|";

    std::cout << oss.str() << std::endl;
}

void filosofo::cenar() {
    using namespace std::chrono_literals;
    using clock = std::chrono::steady_clock;

    while (clock::now() < end_) {
        // PENSANDO
        log_estado("PENSANDO");
        std::this_thread::sleep_for(500ms);

        // HAMBRIENTO (intenta adquirir recursos)
        log_estado("HAMBRIENTO");
        auto t_hambre = clock::now();

        // Solicitar permiso al camarero antes de intentar tomar los tenedores
        waiter_.solicitar_permiso();

        // Preparar locks de ambos tenedores fuera del if para que vivan durante COMIENDO
        std::unique_lock<std::mutex> l(left_tenedor_.getMutex(), std::defer_lock);
        std::unique_lock<std::mutex> r(right_tenedor_.getMutex(), std::defer_lock);

        // Asimetría de orden de adquisición: pares D->I, impares I->D
        bool par = (id_ %2)==0;
        if (par) {
            // par: derecho luego izquierdo, adquiriendo ambos sin interbloqueo
            std::lock(r, l);
        } else {
            // impar: izquierdo luego derecho
            std::lock(l, r);
        }

        // COMIENDO (ambos tenedores adquiridos; locks activos en este scope)
        auto t_comer = clock::now();
        double wait_ms = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(t_comer - t_hambre).count());
        wait_sum_ms_ += wait_ms;
        wait_sq_sum_ms_ += wait_ms * wait_ms;
        if (wait_ms > wait_max_ms_) wait_max_ms_ = wait_ms;
        ++eat_count_;
        log_estado("COMIENDO");
        std::this_thread::sleep_for(600ms);

        // Al salir del scope, se liberan los tenedores automáticamente (RAII)
        // Liberar permiso cuando termina de comer
        waiter_.liberar_permiso();

        // DORMIR (tras comer, antes de volver a pensar)
        log_estado("DORMIR");
        std::this_thread::sleep_for(400ms);
    }

    // Fin
    log_estado("FIN");
}
