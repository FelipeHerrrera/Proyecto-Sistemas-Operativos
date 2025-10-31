# Proyecto: Problema de los Filósofos (C++17)

## Descripción
Simulación del problema clásico de los filósofos comensales usando C++17 y `std::thread`. Se emplea una estrategia anti-deadlock con un camarero (waiter) que limita concurrentemente a N-1 filósofos intentando comer y la adquisición conjunta (sin interbloqueo) de ambos tenedores con `std::unique_lock` + `std::lock`.

## Arquitectura
- `tenedor`
  - Encapsula un `std::mutex` para sincronizar acceso.
  - No copiable ni asignable ni movible.
- `camarero`
  - Controla la concurrencia de comensales mediante una capacidad `N-1`.
  - Métodos: `solicitar_permiso()` y `liberar_permiso()` usando `std::mutex` + `std::condition_variable` con cola FIFO por tickets.
- `filosofo`
  - Mantiene `id`, referencias a dos `tenedor` y referencia a `camarero`.
  - Método `cenar()`: pensar → solicitar permiso → tomar ambos tenedores → comer → liberar permiso.
  - Toma ambos tenedores con `std::unique_lock(std::defer_lock)` y `std::lock(l, r)` para adquisición conjunta.
- `Main.cpp`
  - Crea 5 tenedores, un `camarero` con capacidad `N-1`, 5 filósofos y 5 hilos.
  - Lanza los hilos y hace `join` al final.

## Hilos creados
- N hilos de filósofo (`std::thread`), uno por cada filósofo (por defecto N=5), creados y gestionados desde `Main.cpp`.
- No existe un hilo dedicado de mesero: el camarero está implementado como un monitor (mutex + `condition_variable`) con cola FIFO por tickets que concede permisos (N-1) a los filósofos.

## Estrategia anti-deadlock (elegida)
- Waiter (N-1) implementado como monitor con cola FIFO por tickets: rompe la espera circular y reduce la inanición.
- Además se aplica asimetría en el orden de adquisición por paridad (pares D→I, impares I→D) como refuerzo.
- Adquisición conjunta de ambos tenedores con `std::lock` sobre dos `std::unique_lock` para evitar interbloqueo por orden de bloqueo.

## Estructura de archivos
- `Main.cpp`
- `filosofo.h` / `filosofo.cpp`
- `tenedor.h` / `tenedor.cpp`
- `camarero.h`

## Tabla de estados y transiciones
| Estado actual | Evento / Condición | Nuevo estado | Precondición | Observaciones |
| --- | --- | --- | --- | --- |
| PENSANDO | Fin de intervalo de pensamiento | HAMBRIENTO | Tiempo pensando ≥ 500 ms | Decide intentar comer. |
| HAMBRIENTO | Obtiene permiso del camarero y adquiere ambos tenedores | COMIENDO | Turno en la cola FIFO del camarero y `en_uso < capacidad_max`; bloqueo exitoso de ambos mutex con `std::lock` | Se registra el tiempo de espera HAMBRIENTO→COMIENDO para métricas. Asimetría: pares D→I, impares I→D. |
| HAMBRIENTO | No hay permiso disponible o no logra tomar ambos tenedores | — (Bloqueado) | N/A | Permanece esperando en `condition_variable` (FIFO); reduce inanición observable. |
| COMIENDO | Termina de comer | DORMIR | Tiempo comiendo ≥ 600 ms | Libera el permiso del camarero al finalizar. A DORMIR solo se entra desde COMIENDO. |
| DORMIR | Fin del descanso | PENSANDO | Tiempo durmiendo ≥ 400 ms | Reinicia el ciclo. |
| DORMIR | — | COMIENDO | Nunca | Transición no permitida; debe pasar por PENSANDO → HAMBRIENTO. |
| Cualquiera | Se completan los ciclos de demostración | FIN | Tras 3 ciclos | Termina el hilo del filósofo. |

## Requerimientos técnicos (del enunciado) y cumplimiento
- Estándar: C++17+ recomendado (el código es compatible; puede compilarse como C++17).
- Sincronización: `std::mutex`/`std::condition_variable` (cumplido).
- Prohibido busy-wait: se usan esperas bloqueantes, sin bucles activos (cumplido).

## Compilación (ejemplos CLI)
- g++ (Linux/macOS):
  - `g++ -std=c++17 -pthread -O2 -I. Main.cpp filosofo.cpp tenedor.cpp -o filosofos`
- clang++:
  - `clang++ -std=c++17 -pthread -O2 -I. Main.cpp filosofo.cpp tenedor.cpp -o filosofos`
- MSVC (Windows):
  - Proyecto de Visual Studio ya incluido; no requiere `-pthread`.

## Consideraciones
- Esta solución evita deadlock y reduce la posibilidad de inanición con una cola FIFO en el camarero.
- Mejoras posibles: tiempos aleatorios y semilla, CLI para parámetros, exportar métricas a CSV, escenarios de demo.

## Autores
- Valeria Marin Calderon.
- Felipe Herrera Granados.