# Proyecto: Problema de los Filósofos

## Descripción
Simulación del problema clásico de los filósofos comensales usando C++14 y `std::thread`. La solución evita interbloqueo con:
- Un camarero (waiter) que limita a N-1 filósofos intentando comer de forma concurrente (FIFO por tickets para fairness).
- Adquisición no bloqueante de tenedores con `std::mutex::try_lock()`; si falla, se liberan recursos y se reintenta con backoff exponencial y jitter aleatorio.
- Asimetría por paridad (pares D→I, impares I→D).

Incluye logging básico de estados y métricas preliminares. Al finalizar, exporta `metricas.csv` con resultados por filósofo.

## Arquitectura
- `tenedor`
 - Encapsula un `std::mutex`.
 - No copiable ni movible.
- `camarero`
 - Monitor con `std::mutex` + `std::condition_variable` y cola FIFO por tickets.
 - Métodos: `solicitar_permiso()` / `liberar_permiso()`; capacidad `N-1`.
- `filosofo`
 - Atributos: `id`, dos `tenedor` (izq/der) y `camarero`.
 - `cenar()`: PENSANDO → HAMBRIENTO → (intentos con `try_lock` + backoff) → COMIENDO → DORMIR; repite hasta fin del tiempo global.
- `Main.cpp`
 - Configura duración (10 s por defecto), crea recursos, lanza hilos, imprime métricas y genera `metricas.csv`.

## Estrategia anti-deadlock y contención
- Waiter (capacidad `N-1`) rompe la espera circular y reduce inanición (FIFO).
- Asimetría por paridad al intentar tomar tenedores.
- Reintentos no bloqueantes con `try_lock` y backoff exponencial con jitter para evitar colisiones/livelock.

## Estados (logging)
- `PENSANDO`, `HAMBRIENTO`, `COMIENDO`, `DORMIR`, `BACKOFF`, `FIN` (timestamps relativos al inicio, salida thread-safe con encabezado único).

## Métricas
- Por filósofo: `comidas`, `espera_promedio_ms`, `espera_maxima_ms`, `desviacion_espera_ms` (HAMBRIENTO→COMIENDO).
- Consola: imprime métricas y verificación de que todos comieron ≥1.
- CSV: archivo `metricas.csv` con columnas: `filosofo,comidas,espera_promedio_ms,espera_maxima_ms,desviacion_espera_ms`.


## Consideraciones
- Sin busy-wait: el camarero bloquea por `condition_variable` y los reintentos usan `sleep` con backoff.
- Posibles mejoras: CLI para N, duración y rangos de backoff; semillas para reproducibilidad; tiempos aleatorios de pensar/comer/dormir.

## Autores
- Valeria Marin Calderon.
- Felipe Herrera Granados.