#include "filosofo.h"
#include <iostream>
#include <chrono>
#include <mutex>
#include <thread>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <string>
#include <random>

namespace {
 std::mutex g_log_mtx;
 std::once_flag g_log_header_once;

 void print_log_header_unlocked() {
 std::cout << "\n===== LOG DE ESTADOS =====\n";
 std::cout << "+---------+-----------+-----------+\n";
 std::cout << "| Tiempo | Filosofo | Estado |\n";
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

    // Llama a print_log_header fuera del lock para evitar doble bloqueo
    std::call_once(g_log_header_once, print_log_header);

    std::ostringstream oss;
    oss << "| " << std::setw(7) << ms << "ms | "
        << "Filosofo " << std::setw(2) << id_ << " | "
        << std::left << std::setw(10) << estado << "|";

    std::lock_guard<std::mutex> lk(g_log_mtx);
    std::cout << oss.str() << std::endl;
}
void filosofo::cenar() {
 using namespace std::chrono_literals;
 using clock = std::chrono::steady_clock;

 // Deadline máximo por hilo:30 segundos desde el inicio global
 const auto max_thread_deadline = start_ + std::chrono::seconds(30);
 auto deadline = (end_ < max_thread_deadline) ? end_ : max_thread_deadline;

 // RNG por hilo para backoff
 thread_local std::mt19937 rng(std::random_device{}());

 while (clock::now() < deadline) {
 // PENSANDO (capado al tiempo restante hasta el deadline)
 log_estado("PENSANDO");
 {
 auto now = clock::now();
 if (now >= deadline) break;
 auto restante = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now);
 auto d = std::min(restante,500ms);
 std::this_thread::sleep_for(d);
 }

 // HAMBRIENTO (intenta adquirir recursos)
 if (clock::now() >= deadline) break;
 log_estado("HAMBRIENTO");
 auto t_hambre = clock::now();

 // Reintentos con backoff exponencial y try_lock
 std::size_t intento =0;
 bool logro_comer = false;
 while (clock::now() < deadline) {
 // Obtener permiso del camarero para intentar tomar tenedores
 waiter_.solicitar_permiso();
 bool locked = false;
 {
 // Preparar locks de ambos tenedores con adquisición diferida
 std::unique_lock<std::mutex> left_lock(left_tenedor_.getMutex(), std::defer_lock);
 std::unique_lock<std::mutex> right_lock(right_tenedor_.getMutex(), std::defer_lock);

 // Asimetría: pares D->I, impares I->D, usando try_lock no bloqueante
 bool par = (id_ %2) ==0;
 if (par) {
 if (right_lock.try_lock()) {
 if (left_lock.try_lock()) {
 locked = true;
 }
 else {
 right_lock.unlock();
 }
 }
 } else {
 if (left_lock.try_lock()) {
 if (right_lock.try_lock()) {
 locked = true;
 }
 else {
 left_lock.unlock();
 }
 }
 }

 if (locked) {
 // COMIENDO (ambos tenedores adquiridos)
 auto t_comer = clock::now();
 double wait_ms = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(t_comer - t_hambre).count());
 wait_sum_ms_ += wait_ms;
 wait_sq_sum_ms_ += wait_ms * wait_ms;
 if (wait_ms > wait_max_ms_) wait_max_ms_ = wait_ms;
 ++eat_count_;
 log_estado("COMIENDO");
 // Capar tiempo de comer al tiempo restante
 auto now2 = clock::now();
 if (now2 < deadline) {
 auto restante = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now2);
 auto d = std::min(restante,600ms);
 std::this_thread::sleep_for(d);
 }
 // al salir del bloque se liberan automáticamente los tenedores (RAII)
 logro_comer = true;
 }
 }
 // Liberar permiso del camarero tras un intento, exitoso o no
 waiter_.liberar_permiso();

 if (logro_comer) {
 // DORMIR (tras comer, antes de volver a pensar) con tope por deadline
 log_estado("DORMIR");
 auto now3 = clock::now();
 if (now3 < deadline) {
 auto restante = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now3);
 auto d = std::min(restante,400ms);
 std::this_thread::sleep_for(d);
 }
 break; // salir del ciclo de reintentos y volver a pensar
 }

 // Backoff exponencial con jitter aleatorio antes de reintentar
 const int base_ms =10;
 const int cap_ms =300;
 int factor =1 << static_cast<int>(std::min<std::size_t>(intento,8)); // limita el crecimiento
 int max_delay = std::min(cap_ms, base_ms * factor);
 std::uniform_int_distribution<int> dist(0, max_delay);
 int delay = dist(rng);
 log_estado("BACKOFF");
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

 // Fin
 log_estado("FIN");
}

