#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <string>
#include <sstream>
#include <algorithm>
#include "filosofo.h"
#include "tenedor.h"
#include "camarero.h"
#include "Logger.h" // Paso 1: nuevo módulo de logging/estado

struct Config {
    int n = 5;
    std::chrono::milliseconds dur{30000};
    std::pair<int,int> think{50,150};
    std::pair<int,int> eat{80,120};
    std::pair<int,int> sleep{60,120};
    unsigned int seed = 123;
    std::string strategy = "waiter"; // solo documentación por ahora
    std::string extra = "DORMIR";
    std::string out = "metricas.csv";
};

static bool parse_range_ms(const std::string& s, std::pair<int,int>& out) {
    auto dash = s.find('-');
    if (dash == std::string::npos) return false;
    std::string a = s.substr(0, dash);
    std::string b = s.substr(dash + 1);
    // admitir sufijo ms opcional
    if (a.size() > 2 && a.substr(a.size()-2) == "ms") a.resize(a.size()-2);
    if (b.size() > 2 && b.substr(b.size()-2) == "ms") b.resize(b.size()-2);
    int ai=0, bi=0;
    try {
        ai = std::stoi(a);
        bi = std::stoi(b);
    } catch (...) { return false; }
    if (ai > bi || ai < 0) return false;
    out = {ai, bi};
    return true;
}

static bool parse_duration(const std::string& s, std::chrono::milliseconds& out) {
    // soporta "30s" o "2500ms"
    if (s.size() >= 2 && s.substr(s.size()-1) == "s") {
        std::string num = s.substr(0, s.size()-1);
        try { out = std::chrono::milliseconds(std::stoi(num) * 1000); } catch (...) { return false; }
        return true;
    }
    if (s.size() >= 3 && s.substr(s.size()-2) == "ms") {
        std::string num = s.substr(0, s.size()-2);
        try { out = std::chrono::milliseconds(std::stoi(num)); } catch (...) { return false; }
        return true;
    }
    return false;
}

static Config parse_cli(int argc, char** argv) {
    Config cfg;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        auto next = [&]() -> std::string { return (i + 1 < argc) ? std::string(argv[++i]) : std::string(); };
        if (arg == "--n") {
            std::string v = next();
            try { cfg.n = std::stoi(v); } catch (...) {}
            if (cfg.n < 2) cfg.n = 2;
        } else if (arg == "--t") {
            std::string v = next();
            std::chrono::milliseconds d;
            if (parse_duration(v, d)) cfg.dur = d;
        } else if (arg == "--think") {
            std::string v = next();
            parse_range_ms(v, cfg.think);
        } else if (arg == "--eat") {
            std::string v = next();
            parse_range_ms(v, cfg.eat);
        } else if (arg == "--sleep") {
            std::string v = next();
            parse_range_ms(v, cfg.sleep);
        } else if (arg == "--seed") {
            std::string v = next();
            try { cfg.seed = static_cast<unsigned int>(std::stoul(v)); } catch (...) {}
        } else if (arg == "--strategy") {
            cfg.strategy = next();
        } else if (arg == "--extra-state") {
            cfg.extra = next();
        } else if (arg == "--out") {
            cfg.out = next();
        }
    }
    return cfg;
}

static std::string to_upper(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return static_cast<char>(std::toupper(c)); });
    return s;
}

int main(int argc, char** argv) {
    Config cfg = parse_cli(argc, argv);

    // Validaciones y advertencias de CLI
    if (to_upper(cfg.strategy) != "WAITER") {
        std::cerr << "[Aviso] --strategy='" << cfg.strategy << "' no está implementada; se usará 'waiter'." << std::endl;
        cfg.strategy = "waiter";
    }
    std::string extra_upper = to_upper(cfg.extra);
    if (extra_upper != "DORMIR" && extra_upper != "DUERME" && extra_upper != "DORMIDO" && extra_upper != "DUERMER") {
        std::cerr << "[Aviso] --extra-state='" << cfg.extra << "' no soportado; se usará 'DORMIR'." << std::endl;
        cfg.extra = "DORMIR";
    }

    using clock = std::chrono::steady_clock;
    auto start = clock::now();
    filosofo::set_start(start);
    filosofo::set_end(start + cfg.dur);
    filosofo::set_ranges(cfg.think, cfg.eat, cfg.sleep);
    filosofo::set_seed(cfg.seed);

    const int num_filosofos = cfg.n;
    std::vector<tenedor> tenedores(num_filosofos);
    camarero waiter(num_filosofos - 1);

    std::vector<filosofo> filosofos;
    std::vector<std::thread> hilos;
    filosofos.reserve(num_filosofos);
    hilos.reserve(num_filosofos);

    for (int i = 0; i < num_filosofos; ++i) {
        filosofos.emplace_back(i, tenedores[i], tenedores[(i + 1) % num_filosofos], waiter);
    }

    for (int i = 0; i < num_filosofos; ++i) {
        hilos.emplace_back(&filosofo::cenar, &filosofos[i]);
    }

    for (auto& hilo : hilos) {
        hilo.join();
    }

    std::cout << "\n===== METRICAS =====" << std::endl;
    bool todos_comieron = true;
    for (const auto& f : filosofos) {
        std::cout << "Filosofo " << f.id()
                  << ": comidas=" << f.comidas()
                  << ", espera_promedio_ms=" << f.promedio_espera_ms()
                  << ", espera_maxima_ms=" << f.max_espera_ms()
                  << ", desviacion_espera_ms=" << f.desviacion_espera_ms()
                  << std::endl;
        if (f.comidas() < 1) todos_comieron = false;
    }
    std::cout << "Verificacion (todos comieron >=1): " << (todos_comieron ? "OK" : "FALLO") << std::endl;

    try {
        std::ofstream csv(cfg.out.c_str(), std::ios::out | std::ios::trunc);
        if (!csv) {
            std::cerr << "No se pudo crear el archivo '" << cfg.out << "'" << std::endl;
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
        std::cout << "Archivo CSV generado: " << cfg.out << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error al escribir '" << cfg.out << "': " << e.what() << std::endl;
    }

    return 0;
}
