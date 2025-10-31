#pragma once
#include "tenedor.h"
#include "camarero.h"
#include <string>
#include <chrono>
#include <cmath>

class filosofo {
public:
    filosofo(int id, tenedor& left, tenedor& right, camarero& waiter);
    void cenar();

    // Configurar tiempo de inicio global para timestamps relativos
    static void set_start(std::chrono::steady_clock::time_point tp) { start_ = tp; }
    // Configurar tiempo de fin global para terminar la simulación por tiempo
    static void set_end(std::chrono::steady_clock::time_point tp) { end_ = tp; }

    // Métricas y getters
    int id() const { return id_; }
    int comidas() const { return eat_count_; }
    double promedio_espera_ms() const { return eat_count_ >0 ? (wait_sum_ms_ / eat_count_) :0.0; }
    double max_espera_ms() const { return wait_max_ms_; }
    double desviacion_espera_ms() const {
        if (eat_count_ <=1) return 0.0;
        double n = static_cast<double>(eat_count_);
        double mean = wait_sum_ms_ / n;
        double mean_sq = wait_sq_sum_ms_ / n;
        double var = mean_sq - mean * mean;
        return var >0.0 ? std::sqrt(var) :0.0;
    }

private:
    int id_;
    tenedor& left_tenedor_;
    tenedor& right_tenedor_;
    camarero& waiter_;

    // métricas
    int eat_count_ =0;
    double wait_sum_ms_ =0.0;
    double wait_sq_sum_ms_ =0.0;
    double wait_max_ms_ =0.0;

    // start/end time global a todos los filósofos
    static std::chrono::steady_clock::time_point start_;
    static std::chrono::steady_clock::time_point end_;

    // helper de logging con timestamp relativo
    void log_estado(const char* estado) const;
};
