# Proyecto: Problema de los Filósofos (C++14)

## Descripción
Simulación del problema clásico de los filósofos comensales usando C++14 y `std::thread`. Se implementa una estrategia anti-deadlock basada en un rol de camarero (waiter) que limita concurrentemente a N-1 filósofos intentando comer y el uso de bloqueo atómico de tenedores.

## Arquitectura
- `tenedor`
 - Encapsula un `std::mutex` para sincronizar acceso.
 - No copiable ni asignable.
- `camarero`
 - Controla la concurrencia de comensales mediante una capacidad `N-1`.
 - Métodos: `solicitar_permiso()` y `liberar_permiso()` usando `std::mutex` + `std::condition_variable`.
- `filosofo`
 - Mantiene `id`, referencias a dos `tenedor` y referencia a `camarero`.
 - Método `cenar()`: pensar → solicitar permiso → tomar ambos tenedores → comer → liberar permiso.
 - Toma ambos tenedores con `std::unique_lock` (defer) + `std::lock(l1, l2)`.
- `Main.cpp`
 - Crea5 tenedores, un `camarero` con capacidad `N-1`,5 filósofos y5 hilos.
 - Lanza los hilos y hace `join` al final.

## Hilos creados
-5 hilos (`std::thread`), uno por cada filósofo, creados y gestionados desde `Main.cpp` (se hace `join` a todos).

## Estrategia anti-deadlock
- Patrón camarero (capacidad `N-1`): previene que todos los filósofos intenten tomar tenedores a la vez, rompiendo la espera circular.
- Adquisición atómica de ambos tenedores con `std::lock` y `std::unique_lock` con `std::defer_lock` para evitar interbloqueo por orden de bloqueo.

## Estructura de archivos
- `Main.cpp`
- `filosofo.h` / `filosofo.cpp`
- `tenedor.h` / `tenedor.cpp`
- `camarero.h`

- 
## Consideraciones
- Esta solución evita deadlock. No impone una política de equidad estricta; la condición puede despertar en orden no-fifo.
- Mejoras posibles: bucle de pensar/comer con tiempos aleatorios, registro de eventos y métricas, política de fairness.

## Autores
- Valeria Marin Calderon.
- Felipe Herrera Granados.
