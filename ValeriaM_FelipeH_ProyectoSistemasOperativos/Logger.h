#pragma once
#include <chrono>
#include <mutex>
#include <iostream>
#include <iomanip>
#include <string>
#include <atomic>
#include <array>

// Paso 1: enum de estados y módulo de logging thread-safe.

enum class Estado {
    PENSANDO,
    HAMBRIENTO,
    COMIENDO,
    DORMIR,
    BACKOFF,
    FIN
};

inline const char* to_string(Estado e) {
    switch (e) {
    case Estado::PENSANDO:  return "PENSANDO";
    case Estado::HAMBRIENTO: return "HAMBRIENTO";
    case Estado::COMIENDO:  return "COMIENDO";
    case Estado::DORMIR:    return "DORMIR";
    case Estado::BACKOFF:   return "BACKOFF";
    case Estado::FIN:       return "FIN";
    }
    return "?";
}

class Logger {
public:
    static void log(int idFilosofo, Estado estado, std::chrono::steady_clock::time_point inicio) {
        using clock = std::chrono::steady_clock;
        static std::mutex m;            // mutex estático local (Meyers) para sincronizar
        static bool header_impreso = false; // bandera estática local para una sola impresión del header

        auto now = clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - inicio).count();

        std::lock_guard<std::mutex> lk(m);
        if (!header_impreso) {
            std::cout << "\n===== LOG DE ESTADOS =====\n";
            std::cout << "+---------+-----------+-----------+\n";
            std::cout << "| Tiempo  | Filosofo  | Estado    |\n";
            std::cout << "+---------+-----------+-----------+\n";
            header_impreso = true;
        }
        std::cout << "| " << std::setw(7) << ms << "ms | Filosofo " << std::setw(2) << idFilosofo
                  << " | " << std::left << std::setw(10) << to_string(estado) << "|" << std::endl;
    }
};
