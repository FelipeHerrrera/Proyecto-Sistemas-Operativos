#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include "filosofo.h"
#include "tenedor.h"
#include "camarero.h"

const int num_filosofos =5;

int main() {
    using clock = std::chrono::steady_clock;
    filosofo::set_start(clock::now());

    // Crear tenedores
    std::vector<tenedor> tenedores(num_filosofos);

    // Crear camarero con capacidad N-1
    camarero waiter(num_filosofos -1);

    // Crear filósofos y sus hilos
    std::vector<filosofo> filosofos;
    std::vector<std::thread> hilos;
    filosofos.reserve(num_filosofos);
    hilos.reserve(num_filosofos);

    for (int i =0; i < num_filosofos; ++i) {
        // Cada filósofo recibe su tenedor izquierdo y derecho, y el camarero
        filosofos.emplace_back(i, tenedores[i], tenedores[(i +1) % num_filosofos], waiter);
    }

    // Lanzar los hilos de los filósofos
    for (int i =0; i < num_filosofos; ++i) {
        hilos.emplace_back(&filosofo::cenar, &filosofos[i]);
    }

    // Esperar a que terminen
    for (auto& hilo : hilos) {
        hilo.join();
    }

    // Métricas al finalizar
    std::cout << "\n===== METRICAS =====" << std::endl;
    bool todos_comieron = true;
    for (const auto& f : filosofos) {
        std::cout << "Filosofo " << f.id()
                  << ": comidas=" << f.comidas()
                  << ", espera_prom_ms=" << f.promedio_espera_ms()
                  << ", espera_max_ms=" << f.max_espera_ms()
                  << ", desv_espera_ms=" << f.desviacion_espera_ms()
                  << std::endl;
        if (f.comidas() <1) todos_comieron = false;
    }
    std::cout << "Verificacion (todos comieron >=1): " << (todos_comieron ? "OK" : "FALLO") << std::endl;

    return 0;
}
