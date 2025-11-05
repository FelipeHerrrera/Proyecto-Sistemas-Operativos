#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <fstream>
#include <iomanip>
#include "filosofo.h"
#include "tenedor.h"
#include "camarero.h"

const int num_filosofos =5;

int main() {
    using clock = std::chrono::steady_clock;
    auto start = clock::now();
    filosofo::set_start(start);

    // Duración por defecto (10s) hasta agregar CLI
    auto duracion = std::chrono::seconds(10);
    filosofo::set_end(start + duracion);

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
                  << ", espera_promedio_ms=" << f.promedio_espera_ms()
                  << ", espera_maxima_ms=" << f.max_espera_ms()
                  << ", desviacion_espera_ms=" << f.desviacion_espera_ms()
                  << std::endl;
        if (f.comidas() <1) todos_comieron = false;
    }
    std::cout << "Verificacion (todos comieron >=1): " << (todos_comieron ? "OK" : "FALLO") << std::endl;

    // Exportar métricas a CSV
    try {
        std::ofstream csv("metricas.csv", std::ios::out | std::ios::trunc);
        if (!csv) {
            std::cerr << "No se pudo crear el archivo metricas.csv" << std::endl;
            return 1;
        }
        csv << "filosofo,comidas,espera_promedio_ms,espera_maxima_ms,desviacion_espera_ms" << '\n';
        csv.setf(std::ios::fixed);
        csv << std::setprecision(3);
        for (const auto& f : filosofos) {
            csv << f.id() << ','
                << f.comidas() << ','
                << f.promedio_espera_ms() << ','
                << f.max_espera_ms() << ','
                << f.desviacion_espera_ms() << '\n';
        }
        csv.close();
        std::cout << "Archivo CSV generado: metricas.csv" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error al escribir metricas.csv: " << e.what() << std::endl;
    }

    return 0;
}
