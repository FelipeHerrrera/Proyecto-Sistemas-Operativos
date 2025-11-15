# Proyecto: Problema de los Filósofos

## Descripción
Simulación concurrente del problema de los filósofos comensales en C++ (hilos + sincronización). La solución evita interbloqueo usando la estrategia Mesero/Árbitro (Waiter):
- Un camarero limita a N-1 filósofos intentando comer a la vez.
- Monitor con mutex + condition_variable y cola FIFO (tickets) para fairness observable.
- Intentos de toma de tenedores con orden alternado (pares D→I, impares I→D) y reintentos con pequeño backoff aleatorio para reducir colisiones. La garantía anti-deadlock proviene del camarero.

Genera logging de estados con timestamp relativo y métricas por filósofo. Exporta un CSV con resultados por filósofo.

## Compilación y ejecución
- Windows (Visual Studio 2022): abrir la solución/proyecto y compilar. Estándar C++14 configurado; compatible con C++17.
- Linux/macOS (ejemplo):
  g++ -std=c++17 -pthread -O2 Main.cpp filosofo.cpp tenedor.cpp -o filosofos

## CLI (parámetros)
- --n <int>               número de filósofos (por defecto 5)
- --t <duración>          30s | 2500ms (duración total)
- --strategy waiter       estrategia anti-deadlock (única implementada)
- --think A-Bms           rango pensar en ms (p. ej., 50-150ms)
- --eat C-Dms             rango comer en ms (p. ej., 80-120ms)
- --sleep X-Yms           rango dormir en ms (p. ej., 60-120ms)
- --seed <uint>           semilla para reproducibilidad
- --extra-state DORMIR    estado extra (fijo)
- --out <archivo.csv>     nombre del archivo de salida CSV (por defecto metricas.csv)

Notas:
- --strategy: si no es "waiter", se muestra aviso y se usa waiter igualmente.
- --extra-state: si no es DORMIR/DUERME, se muestra aviso y se usa DORMIR.

## Arquitectura
- tenedor
  - Encapsula un std::mutex (exclusión mutua por tenedor).
- camarero
  - Monitor con FIFO por tickets; concede permisos hasta N-1 concurrentes.
  - solicitar_permiso() / liberar_permiso().
- filosofo
  - Lógica de ciclo de vida, logging y métricas.
  - Usa camarero + dos tenedores.
- Logger
  - Módulo thread-safe; imprime una cabecera única y estados con timestamps relativos.

## Estrategia anti-deadlock (elegida)
- Waiter (Mesero, N-1 permisos): rompe la espera circular y garantiza ausencia de deadlock.
- Fairness: cola FIFO por tickets en el camarero reduce inanición observable.
- Complementos: asimetría de intento (pares D→I, impares I→D) y backoff con jitter para reducir colisiones; no son requeridos para la corrección del anti-deadlock.

## Estados y logging
Estados registrados: PENSANDO, HAMBRIENTO, COMIENDO, DORMIR (extra), BACKOFF (transitorio técnico), FIN.

## Tabla de transiciones (estado extra DORMIR)
| Estado Actual | Evento / Condición                        | Nuevo Estado | Precondición                                      | Observaciones                                     |
|---------------|-------------------------------------------|--------------|----------------------------------------------------|---------------------------------------------------|
| PENSANDO      | Termina intervalo de pensar               | HAMBRIENTO   | Tiempo pensando ≥ límite aleatorio                 | Entra a intentar adquirir recursos.               |
| COMIENDO      | Termina intervalo de comer                | DORMIR       | Haber adquirido y usado ambos tenedores            | Descanso breve tras comer.                        |
| DORMIR        | Termina intervalo de dormir               | PENSANDO     | Tiempo de dormir ≥ límite aleatorio                | Vuelve al ciclo normal de pensamiento.            |
| DORMIR        | Intento de comer                          | — (Bloqueado)| N/A                                                | No puede adquirir tenedores mientras duerme.      |
| DORMIR        | —                                         | COMIENDO     | Nunca                                              | Transición no permitida (debe pasar por PENSANDO).|

## Métricas y CSV
- Para cada filósofo:
  - comidas: número de veces que comió.
  - espera_promedio_ms: media de espera desde HAMBRIENTO hasta COMIENDO.
  - espera_maxima_ms: máximo de espera observado.
  - desviacion_espera_ms: sqrt(E[x^2] − (E[x])^2) con x = espera_ms.
- CSV generado: columnas: filosofo, comidas, espera_promedio_ms, espera_maxima_ms, desviacion_espera_ms.
- Verificación: en escenario estándar todos comen ≥ 1 vez (impreso en consola).

## Escenarios para demo (comandos listos)
1) Estándar: N=5, 30s
- ./filosofos --n 5 --t 30s --strategy waiter --think 50-150ms --eat 80-120ms --sleep 60-120ms --seed 123 --out estandar.csv

2) Stress: pensar 1–10 ms / comer 5–15 ms
- ./filosofos --n 5 --t 30s --think 1-10ms --eat 5-15ms --sleep 5-10ms --seed 123 --out stress.csv

3) Justicia: N=7, 25s
- ./filosofos --n 7 --t 25s --think 50-150ms --eat 80-120ms --sleep 60-120ms --seed 123 --out justicia.csv
- Revisar promedios y máximos de espera, y que todos coman ≥ 1.

## Autores
- Valeria Marin Calderon
- Felipe Herrera Granados
