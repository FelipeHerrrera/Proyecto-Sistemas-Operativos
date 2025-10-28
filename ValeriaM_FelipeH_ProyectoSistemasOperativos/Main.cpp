#include <iostream>
#include <vector>
#include <thread>
#include "filosofo.h"
#include "tenedor.h"
#include "camarero.h"

const int num_filosofos =5;

int main() {
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

    return 0;
}
