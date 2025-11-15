#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <string>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <ctime>
#include "filosofo.h"
#include "tenedor.h"
#include "camarero.h"
#include "Logger.h" // Paso1: nuevo módulo de logging/estado

struct Config {
 int n =5;
 std::chrono::milliseconds dur{30000};
 std::pair<int,int> think{50,150};
 std::pair<int,int> eat{80,120};
 std::pair<int,int> sleep{60,120};
 unsigned int seed =123;
 std::string strategy = "waiter"; // solo documentación por ahora
 std::string extra = "DORMIR";
 std::string out = "metricas.csv";
 // Soporte de escenarios
 std::string scenario; // estandar | stress | justicia (insensible a mayúsculas)
 bool user_set_n = false;
 bool user_set_t = false;
 bool user_set_think = false;
 bool user_set_eat = false;
 bool user_set_sleep = false;
 bool user_set_seed = false;
 bool user_set_out = false;
 bool user_set_strategy = false;
 bool user_set_extra = false;
 bool user_set_scenario = false;
};

static bool parse_range_ms(const std::string& s, std::pair<int,int>& out) {
 auto dash = s.find('-');
 if (dash == std::string::npos) return false;
 std::string a = s.substr(0, dash);
 std::string b = s.substr(dash +1);
 // admitir sufijo ms opcional
 if (a.size() >2 && a.substr(a.size()-2) == "ms") a.resize(a.size()-2);
 if (b.size() >2 && b.substr(b.size()-2) == "ms") b.resize(b.size()-2);
 int ai=0, bi=0;
 try {
 ai = std::stoi(a);
 bi = std::stoi(b);
 } catch (...) { return false; }
 if (ai > bi || ai <0) return false;
 out = {ai, bi};
 return true;
}

static bool parse_duration(const std::string& s, std::chrono::milliseconds& out) {
 // soporta "30s" o "2500ms"
 if (s.size() >=3 && s.substr(s.size()-2) == "ms") {
 std::string num = s.substr(0, s.size()-2);
 try { out = std::chrono::milliseconds(std::stoi(num)); } catch (...) { return false; }
 return true;
 }
 if (!s.empty() && s.back() == 's') {
 std::string num = s.substr(0, s.size()-1);
 try { out = std::chrono::milliseconds(std::stoi(num) *1000); } catch (...) { return false; }
 return true;
 }
 return false;
}

static std::string to_upper(std::string s) {
 std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return static_cast<char>(std::toupper(c)); });
 return s;
}

static Config parse_cli(int argc, char** argv) {
 Config cfg;
 for (int i =1; i < argc; ++i) {
 std::string arg = argv[i];
 auto next = [&]() -> std::string { return (i +1 < argc) ? std::string(argv[++i]) : std::string(); };
 if (arg == "--n") {
 std::string v = next();
 try { cfg.n = std::stoi(v); cfg.user_set_n = true; } catch (...) {}
 if (cfg.n <2) cfg.n =2;
 } else if (arg == "--t") {
 std::string v = next();
 std::chrono::milliseconds d;
 if (parse_duration(v, d)) { cfg.dur = d; cfg.user_set_t = true; }
 } else if (arg == "--think") {
 std::string v = next();
 if (parse_range_ms(v, cfg.think)) cfg.user_set_think = true;
 } else if (arg == "--eat") {
 std::string v = next();
 if (parse_range_ms(v, cfg.eat)) cfg.user_set_eat = true;
 } else if (arg == "--sleep") {
 std::string v = next();
 if (parse_range_ms(v, cfg.sleep)) cfg.user_set_sleep = true;
 } else if (arg == "--seed") {
 std::string v = next();
 try { cfg.seed = static_cast<unsigned int>(std::stoul(v)); cfg.user_set_seed = true; } catch (...) {}
 } else if (arg == "--strategy") {
 cfg.strategy = next();
 cfg.user_set_strategy = true;
 } else if (arg == "--extra-state") {
 cfg.extra = next();
 cfg.user_set_extra = true;
 } else if (arg == "--out") {
 cfg.out = next();
 cfg.user_set_out = true;
 } else if (arg == "--scenario" || arg == "--escenario") {
 cfg.scenario = next();
 cfg.user_set_scenario = true;
 } else if (arg == "--estandar" || arg == "--estándar" || arg == "--standard") {
 cfg.scenario = "estandar"; cfg.user_set_scenario = true;
 } else if (arg == "--stress") {
 cfg.scenario = "stress"; cfg.user_set_scenario = true;
 } else if (arg == "--justicia") {
 cfg.scenario = "justicia"; cfg.user_set_scenario = true;
 }
 }
 return cfg;
}

// Aplica parámetros por escenario si el usuario lo solicitó. Solo setea
// parámetros que el usuario NO estableció explícitamente por CLI.
static void apply_scenario_defaults(Config& cfg) {
 if (!cfg.user_set_scenario) return;
 const auto scen = to_upper(cfg.scenario);
 auto set_if = [](bool user_set, auto& target, const auto& value) {
 if (!user_set) target = value;
 };
 if (scen == "ESTANDAR" || scen == "STANDARD" || scen == "ESTÁNDAR") {
 set_if(cfg.user_set_n, cfg.n,5);
 set_if(cfg.user_set_t, cfg.dur, std::chrono::milliseconds(30000));
 set_if(cfg.user_set_think, cfg.think, std::pair<int,int>{50,150});
 set_if(cfg.user_set_eat, cfg.eat, std::pair<int,int>{80,120});
 set_if(cfg.user_set_sleep, cfg.sleep, std::pair<int,int>{60,120});
 if (!cfg.user_set_out) cfg.out = "estandar.csv";
 } else if (scen == "STRESS") {
 set_if(cfg.user_set_n, cfg.n,5);
 set_if(cfg.user_set_t, cfg.dur, std::chrono::milliseconds(30000));
 set_if(cfg.user_set_think, cfg.think, std::pair<int,int>{1,10});
 set_if(cfg.user_set_eat, cfg.eat, std::pair<int,int>{5,15});
 set_if(cfg.user_set_sleep, cfg.sleep, std::pair<int,int>{5,10});
 if (!cfg.user_set_out) cfg.out = "stress.csv";
 } else if (scen == "JUSTICIA") {
 set_if(cfg.user_set_n, cfg.n,7);
 set_if(cfg.user_set_t, cfg.dur, std::chrono::milliseconds(25000));
 set_if(cfg.user_set_think, cfg.think, std::pair<int,int>{50,150});
 set_if(cfg.user_set_eat, cfg.eat, std::pair<int,int>{80,120});
 set_if(cfg.user_set_sleep, cfg.sleep, std::pair<int,int>{60,120});
 if (!cfg.user_set_out) cfg.out = "justicia.csv";
 } else {
 // escenario desconocido: no cambia nada
 }
}

static bool eq_pair(const std::pair<int,int>& a, const std::pair<int,int>& b) {
 return a.first == b.first && a.second == b.second;
}

static std::string detectar_escenario_por_param(const Config& cfg) {
 const auto ms = [](int x){ return std::chrono::milliseconds(x); };
 if (cfg.n ==5 && cfg.dur == ms(30000)
 && eq_pair(cfg.think, {50,150})
 && eq_pair(cfg.eat, {80,120})
 && eq_pair(cfg.sleep, {60,120})) return "Estandar";
 if (cfg.n ==5 && cfg.dur == ms(30000)
 && eq_pair(cfg.think, {1,10})
 && eq_pair(cfg.eat, {5,15})
 && eq_pair(cfg.sleep, {5,10})) return "Stress";
 if (cfg.n ==7 && cfg.dur == ms(25000)
 && eq_pair(cfg.think, {50,150})
 && eq_pair(cfg.eat, {80,120})
 && eq_pair(cfg.sleep, {60,120})) return "Justicia";
 return "Personalizado";
}

static std::string ahora_formateado() {
 using std::chrono::system_clock;
 auto now = system_clock::now();
 std::time_t tt = system_clock::to_time_t(now);
#if defined(_MSC_VER)
 std::tm tm{};
 localtime_s(&tm, &tt);
#else
 std::tm tm = *std::localtime(&tt);
#endif
 std::ostringstream os;
 os << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
 return os.str();
}

static void imprimir_cfg(const Config& c) {
 std::cout << "Parametros: N=" << c.n
 << ", T=" << c.dur.count() << "ms"
 << ", think=" << c.think.first << "-" << c.think.second << "ms"
 << ", eat=" << c.eat.first << "-" << c.eat.second << "ms"
 << ", sleep=" << c.sleep.first << "-" << c.sleep.second << "ms"
 << ", seed=" << c.seed
 << ", out='" << c.out << "'"
 << std::endl;
}

int main(int argc, char** argv) {
 Config cfg = parse_cli(argc, argv);

 // Aplicar escenario si fue solicitado (respetando overrides del usuario)
 apply_scenario_defaults(cfg);

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

 // Encabezado de escenario y tiempo
 const std::string nombre_escenario = cfg.user_set_scenario && !cfg.scenario.empty()
 ? (to_upper(cfg.scenario) == "ESTANDAR" || to_upper(cfg.scenario) == "ESTÁNDAR" || to_upper(cfg.scenario) == "STANDARD" ? "Estandar"
 : (to_upper(cfg.scenario) == "STRESS" ? "Stress"
 : (to_upper(cfg.scenario) == "JUSTICIA" ? "Justicia" : "Personalizado")))
 : detectar_escenario_por_param(cfg);

 std::cout << "\n===== ESCENARIO: " << nombre_escenario << " =====" << std::endl;
 std::cout << "Inicio: " << ahora_formateado() << std::endl;
 imprimir_cfg(cfg);

 using clock = std::chrono::steady_clock;
 auto start = clock::now();
 filosofo::set_start(start);
 filosofo::set_end(start + cfg.dur);
 filosofo::set_ranges(cfg.think, cfg.eat, cfg.sleep);
 filosofo::set_seed(cfg.seed);

 const int num_filosofos = cfg.n;
 std::vector<tenedor> tenedores(num_filosofos);
 camarero waiter(num_filosofos -1);

 std::vector<filosofo> filosofos;
 std::vector<std::thread> hilos;
 filosofos.reserve(num_filosofos);
 hilos.reserve(num_filosofos);

 for (int i =0; i < num_filosofos; ++i) {
 filosofos.emplace_back(i, tenedores[i], tenedores[(i +1) % num_filosofos], waiter);
 }

 for (int i =0; i < num_filosofos; ++i) {
 hilos.emplace_back(&filosofo::cenar, &filosofos[i]);
 }

 for (auto& hilo : hilos) {
 hilo.join();
 }

 std::cout << "\n===== METRICAS =====" << std::endl;
 bool todos_comieron = true;
 int total_comidas =0;
 for (const auto& f : filosofos) {
 std::cout << "Filosofo " << f.id()
 << ": comidas=" << f.comidas()
 << ", espera_promedio_ms=" << f.promedio_espera_ms()
 << ", espera_maxima_ms=" << f.max_espera_ms()
 << ", desviacion_espera_ms=" << f.desviacion_espera_ms()
 << std::endl;
 total_comidas += f.comidas();
 if (f.comidas() <1) todos_comieron = false;
 }
 std::cout << "Verificacion (todos comieron >=1): " << (todos_comieron ? "OK" : "FALLO") << std::endl;
 std::cout << "Total de comidas (escenario): " << total_comidas << std::endl;

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

 std::cout << "Fin: " << ahora_formateado() << std::endl;
 std::cout << "===== FIN ESCENARIO: " << nombre_escenario << " =====\n" << std::endl;

 return 0;
}
