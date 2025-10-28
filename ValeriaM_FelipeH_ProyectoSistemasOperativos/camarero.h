#pragma once
#include <mutex>
#include <condition_variable>
#include <deque>

// El camarero controla cuántos filósofos pueden intentar comer simultáneamente.
// Con capacidad N-1 se evita el deadlock clásico de los cinco filósofos.
// Implementación con cola FIFO para reducir/evitar inanición (fairness observable).
class camarero {
public:
 explicit camarero(int capacidad)
 : capacidad_max_(capacidad), en_uso_(0) {}

 // Bloquea hasta que haya permiso disponible, sirviendo en orden de llegada (FIFO).
 void solicitar_permiso() {
 std::unique_lock<std::mutex> lk(mtx_);
 long mi_ticket = next_ticket_++;
 cola_.push_back(mi_ticket);
 cv_.wait(lk, [this, mi_ticket] {
 return (!cola_.empty() && cola_.front() == mi_ticket) && en_uso_ < capacidad_max_;
 });
 cola_.pop_front();
 ++en_uso_;
 }

 // Libera un permiso y notifica al siguiente en la cola.
 void liberar_permiso() {
 std::lock_guard<std::mutex> lk(mtx_);
 if (en_uso_ >0) {
 --en_uso_;
 }
 // Notificar a todos para que el siguiente en la cola reevalúe la condición y entre si hay capacidad.
 cv_.notify_all();
 }

private:
 int capacidad_max_;
 int en_uso_;
 std::mutex mtx_;
 std::condition_variable cv_;
 // Fairness
 long next_ticket_ =0;
 std::deque<long> cola_;
};
