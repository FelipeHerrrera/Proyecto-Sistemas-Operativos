#include <iostream>
#include <vector>
#include <thread>
#include "filosofo.h"
#include "tenedor.h"

const int num_filosofos = 5;

int main() {
    // Crear tenedores
    std::vector<tenedor> tenedores(num_filosofos);

    // Crear filósofos y sus hilos
    std::vector<filosofo> filosofos;
    std::vector<std::thread> hilos;

    for (int i = 0; i < num_filosofos; ++i) {
        // Cada filósofo recibe su tenedor izquierdo y derecho
        filosofos.emplace_back(i, tenedores[i], tenedores[(i + 1) % num_filosofos]);
    }

    // Lanzar los hilos de los filósofos
    for (int i = 0; i < num_filosofos; ++i) {
        hilos.emplace_back(&filosofo::cenar, &filosofos[i]);
    }

    // Esperar a que terminen
    for (auto& hilo : hilos) {
        hilo.join();
    }

    return 0;
}
