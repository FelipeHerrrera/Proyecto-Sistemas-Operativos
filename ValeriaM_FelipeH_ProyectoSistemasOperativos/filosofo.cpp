#include "filosofo.h"
#include <chrono>
#include <mutex>
#include <thread>
#include <random>

std::chrono::steady_clock::time_point filosofo::start_{};
std::chrono::steady_clock::time_point filosofo::end_{};
int filosofo::think_min_ms_ = 50;
int filosofo::think_max_ms_ = 150;
int filosofo::eat_min_ms_ = 80;
int filosofo::eat_max_ms_ = 120;
int filosofo::sleep_min_ms_ = 60;
int filosofo::sleep_max_ms_ = 120;
unsigned int filosofo::seed_ = 12345u;

filosofo::filosofo(int id, tenedor& left, tenedor& right, camarero& waiter)
 : id_(id), left_tenedor_(left), right_tenedor_(right), waiter_(waiter) {}

void filosofo::cenar() {
    using clock = std::chrono::steady_clock;

    const auto max_thread_deadline = start_ + std::chrono::seconds(30);
    auto deadline = (end_ < max_thread_deadline) ? end_ : max_thread_deadline;

    thread_local std::mt19937 rng(seed_ + static_cast<unsigned int>(id_));
    std::uniform_int_distribution<int> dist_think(think_min_ms_, think_max_ms_);
    std::uniform_int_distribution<int> dist_eat(eat_min_ms_, eat_max_ms_);
    std::uniform_int_distribution<int> dist_sleep(sleep_min_ms_, sleep_max_ms_);

    while (clock::now() < deadline) {
        // PENSANDO
        estado_actual_ = Estado::PENSANDO;
        log_estado(estado_actual_);
        {
            auto now = clock::now();
            if (now >= deadline) break;
            int ms = dist_think(rng);
            auto restante = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now);
            auto d = std::min(restante, std::chrono::milliseconds(ms));
            std::this_thread::sleep_for(d);
        }

        // HAMBRIENTO
        if (clock::now() >= deadline) break;
        estado_actual_ = Estado::HAMBRIENTO;
        log_estado(estado_actual_);
        auto t_hambre = clock::now();

        std::size_t intento = 0;
        bool logro_comer = false;
        while (clock::now() < deadline) {
            waiter_.solicitar_permiso();
            bool locked = false;
            {
                std::unique_lock<std::mutex> left_lock(left_tenedor_.getMutex(), std::defer_lock);
                std::unique_lock<std::mutex> right_lock(right_tenedor_.getMutex(), std::defer_lock);
                bool par = (id_ % 2) == 0;
                if (par) {
                    if (right_lock.try_lock()) {
                        if (left_lock.try_lock()) {
                            locked = true;
                        } else {
                            right_lock.unlock();
                        }
                    }
                } else {
                    if (left_lock.try_lock()) {
                        if (right_lock.try_lock()) {
                            locked = true;
                        } else {
                            left_lock.unlock();
                        }
                    }
                }
                if (locked) {
                    auto t_comer = clock::now();
                    double wait_ms = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(t_comer - t_hambre).count());
                    wait_sum_ms_ += wait_ms;
                    wait_sq_sum_ms_ += wait_ms * wait_ms;
                    if (wait_ms > wait_max_ms_) wait_max_ms_ = wait_ms;
                    ++eat_count_;
                    estado_actual_ = Estado::COMIENDO;
                    log_estado(estado_actual_);
                    auto now2 = clock::now();
                    if (now2 < deadline) {
                        int ms = dist_eat(rng);
                        auto restante = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now2);
                        auto d = std::min(restante, std::chrono::milliseconds(ms));
                        std::this_thread::sleep_for(d);
                    }
                    logro_comer = true; // RAII libera tenedores al salir
                }
            }
            waiter_.liberar_permiso();

            if (logro_comer) {
                estado_actual_ = Estado::DORMIR;
                log_estado(estado_actual_);
                auto now3 = clock::now();
                if (now3 < deadline) {
                    int ms = dist_sleep(rng);
                    auto restante = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now3);
                    auto d = std::min(restante, std::chrono::milliseconds(ms));
                    std::this_thread::sleep_for(d);
                }
                break; // volver al ciclo externo
            }

            // BACKOFF (jitter)
            const int base_ms = 10;
            const int cap_ms = 300;
            int factor = 1 << static_cast<int>(std::min<std::size_t>(intento, 8));
            int max_delay = std::min(cap_ms, base_ms * factor);
            std::uniform_int_distribution<int> dist(0, max_delay);
            int delay = dist(rng);
            estado_actual_ = Estado::BACKOFF;
            log_estado(estado_actual_);
            auto now4 = clock::now();
            if (now4 >= deadline) break;
            {
                auto restante = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now4);
                auto d = std::min(restante, std::chrono::milliseconds(delay));
                std::this_thread::sleep_for(d);
            }
            ++intento;
        }
    }
    estado_actual_ = Estado::FIN;
    log_estado(estado_actual_);
}

