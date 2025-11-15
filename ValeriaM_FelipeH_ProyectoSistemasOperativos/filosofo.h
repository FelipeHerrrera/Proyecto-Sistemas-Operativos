#pragma once
#include "tenedor.h"
#include "camarero.h"
#include "Logger.h"
#include <string>
#include <chrono>
#include <cmath>
#include <utility>

class filosofo {
public:
    filosofo(int id, tenedor& left, tenedor& right, camarero& waiter);
    void cenar();

    // Config global para timestamps y fin
    static void set_start(std::chrono::steady_clock::time_point tp) { start_ = tp; }
    static void set_end(std::chrono::steady_clock::time_point tp) { end_ = tp; }

    // Configuración de tiempos (ms)
    static void set_ranges(std::pair<int,int> think_ms,
                           std::pair<int,int> eat_ms,
                           std::pair<int,int> sleep_ms) {
        think_min_ms_ = think_ms.first; think_max_ms_ = think_ms.second;
        eat_min_ms_ = eat_ms.first;     eat_max_ms_ = eat_ms.second;
        sleep_min_ms_ = sleep_ms.first; sleep_max_ms_ = sleep_ms.second;
    }
    static void set_seed(unsigned int s) { seed_ = s; }

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

    // Rango de tiempos (ms)
    static int think_min_ms_;
    static int think_max_ms_;
    static int eat_min_ms_;
    static int eat_max_ms_;
    static int sleep_min_ms_;
    static int sleep_max_ms_;
    static unsigned int seed_;

    // Estado actual (informativo)
    Estado estado_actual_ = Estado::PENSANDO;

    // helper de logging con timestamp relativo
    void log_estado(Estado e) const { Logger::log(id_, e, start_); }
};
