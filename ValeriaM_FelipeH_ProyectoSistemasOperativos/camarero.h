#pragma once
#include <mutex>
#include <condition_variable>

// El camarero controla cuántos filósofos pueden intentar comer simultáneamente.
// Con capacidad N-1 se evita el deadlock clásico de los cinco filósofos.
class camarero {
public:
 explicit camarero(int capacidad)
 : capacidad_max_(capacidad), en_uso_(0) {}

 // Bloquea hasta que haya permiso disponible.
 void solicitar_permiso() {
 std::unique_lock<std::mutex> lk(mtx_);
 cv_.wait(lk, [this]{ return en_uso_ < capacidad_max_; });
 ++en_uso_;
 }

 // Libera un permiso y notifica a un filósofo esperando.
 void liberar_permiso() {
 std::lock_guard<std::mutex> lk(mtx_);
 if (en_uso_ >0) {
 --en_uso_;
 }
 cv_.notify_one();
 }

private:
 int capacidad_max_;
 int en_uso_;
 std::mutex mtx_;
 std::condition_variable cv_;
};
