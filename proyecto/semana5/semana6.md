# Semana 6 — Scheduling y cambios de contexto

**Curso:** Sistemas Operativos (16 semanas)  
**Proyecto:** MiniServ — servidor TCP/HTTP multiproceso en C sobre Linux  
**Duración:** 1 clase × 120 minutos  

**Pregunta central de la semana:**

> MiniServ V4 puede tener **padre + muchos hijos** bajo carga. ¿Quién decide qué proceso usa la CPU, por cuánto tiempo, y qué costo tiene crear un proceso por cliente?

---

# A. Visión general de la semana

## Objetivos de aprendizaje

Al terminar la semana, el estudiante podrá:

1. **Explicar** el rol del **scheduler** del kernel: elegir qué proceso ejecuta en cada núcleo y durante cuánto tiempo.
2. **Identificar** estados clásicos de un proceso (**running**, **ready**, **blocked/waiting**) y relacionarlos con la columna `STAT` de `ps`.
3. **Diferenciar** procesos **CPU-bound** e **I/O-bound** y predecir cuál favorece mejor el throughput de un servidor de red.
4. **Describir** qué es un **context switch** y qué datos mínimos debe guardar/restaurar el kernel (contexto de CPU, no “copiar todo el programa”).
5. **Comparar** MiniServ **V1** (secuencial) y **V4** (fork-per-client) con clientes concurrentes: tiempo total, número de procesos, uso de CPU/memoria.
6. **Interpretar** métricas en `/proc/<pid>/status` (`voluntary_ctxt_switches`, `nonvoluntary_ctxt_switches`) y salidas de `time`, `ps`, `top`.
7. **Analizar** el trade-off fork-per-client: concurrencia real vs. costo de creación de procesos y presión sobre el scheduler.

## Motivación

**Problema concreto:** en laboratorio, veinte terminales con `nc` contra MiniServ V4 generan veinte hijos casi a la vez. El servidor “responde en paralelo”, pero la máquina puede sentirse lenta: sube el load average, aparecen muchos procesos en `ps`, y el padre compite por CPU con los workers.

**Por qué importa en Sistemas Operativos:** ya no basta con saber *cómo* creamos procesos (`fork`) y *cómo* los limpiamos (`wait`, señales). Hay **pocos núcleos** y **muchos procesos listos o bloqueados**. El kernel debe **multiplexar** la CPU: eso es scheduling y conlleva **cambios de contexto**, visibles en herramientas y en el comportamiento bajo carga.

**Manifestación en MiniServ:** el padre pasa mucho tiempo **bloqueado** en `accept` (I/O-bound). Cada hijo alterna **read/write** (más I/O) con algo de CPU al copiar bytes. Bajo ráfagas de clientes, el patrón fork-per-client multiplica procesos y context switches.

**Mecanismo del SO:** colas de procesos listos, políticas de planificación (CFS en Linux moderno, vista de alto nivel), preemption por quantum/temporizador, y conmutación de contexto en el kernel.

## Conexión con semanas anteriores

```text
Semana 3: fork-per-client → muchos procesos
              │
              ▼
Semana 4: fds heredados, zombies, /proc
              │
              ▼
Semana 5: señales, apagado ordenado → MiniServ V4 estable bajo carga
              │
              ▼
problema: ¿qué hace el kernel con N procesos y 1–K CPUs?
              │
              ▼
Semana 6: scheduler, estados, context switch, CPU vs I/O bound
              │
              ▼
Semana 7: los workers querrán hablar entre sí → pipes (logger)
```

---

# 7. Clase — 120 minutos

| Tiempo | Hilo narrativo |
| ------ | -------------- |
| 0–12 min | Demo: N clientes vs V1 y V4 (`demo_bench_concurrency.sh`) |
| 12–30 min | Proceso y estados; ready queue; running vs blocked |
| 30–48 min | Scheduler, quantum, preemption (modelo pedagógico) |
| 48–62 min | Context switch: qué se guarda, costo, dónde se observa |
| 62–78 min | CPU-bound vs I/O-bound; MiniServ como I/O-bound |
| 78–95 min | Laboratorio: `/proc`, `ps`, demos C, stress con `nc` |
| 95–110 min | Ejercicios, predicción, discusión del experimento |
| 110–120 min | Milestone (informe), cierre, puente a pipes (Semana 7) |

---

## Bloque 1 — Problema motivador (0–12 min)

**Demostración:**

```bash
cd proyecto/semana5/programas_ejemplo
make
N=15 ./demo_bench_concurrency.sh
```

**Qué debe notar la clase:**

- V1: los clientes en paralelo **esperan turno** en el servidor; el tiempo wall crece ~lineal con N (orden de magnitud pedagógico).
- V4: las conexiones se atienden **en procesos distintos**; el tiempo wall suele bajar frente a V1 para el mismo N (en una sola máquina modesta).

**Preguntas:**

1. ¿Cuántos procesos `miniserv` hay en V4 durante la prueba? (`pgrep -c miniserv` o `ps --forest`)
2. ¿El kernel atendió las conexiones TCP “en paralelo” aunque haya un solo núcleo? (Sí en I/O; la CPU alterna.)
3. ¿Fork-per-client escala sin límite? (No: memoria, tabla de procesos, costo de fork y context switches.)

**Diagrama:**

```text
V1 — un proceso
[ MiniServ ]──accept──► cliente 1 (read/write)
                         clientes 2..N (cola TCP / esperando accept)

V4 — N+1 procesos bajo ráfaga
[ Padre accept ]     [H1][H2]...[HN]
       │                │  paralelo lógico (time-sharing en 1 CPU)
       └─ fork × N ────┘
```

---

## Bloque 2 — Estados del proceso (12–30 min)

**Qué explicar:**

- Cada proceso tiene un **estado** desde la vista del scheduler.
- Modelo clásico de tres estados (+ transiciones):

```text
                    I/O wait (read, sleep, accept...)
              ┌──────────────────────────────┐
              ▼                              │
         ┌─────────┐   dispatch          ┌─────────┐
         │ BLOCKED │◄────────────────────│ RUNNING │
         │ (wait)  │                     │  on CPU │
         └────┬────┘                     └────┬────┘
              │                               │
              │ evento (dato listo)           │ quantum / preemption
              ▼                               ▼
         ┌─────────┐                     ┌─────────┐
         │  READY  │◄─── admit ──────────│ (sale)  │
         │  queue  │                     └─────────┘
         └─────────┘
```

- En Linux, `ps` muestra letras en `STAT`: `R` running/runnable, `S` sleeping (interruptible), `D` disk sleep (raro en MiniServ), `Z` zombie (Semana 4), etc.

**Pizarrón — MiniServ V4 bajo carga:**

```text
Padre:     S  (accept bloqueante)
Hijo i:    S  en read  |  R  copiando en user space  |  S  en write
```

**Preguntas:**

- ¿Un proceso en `sleep(2)` está usando CPU? (No; blocked; **voluntary** context switch al bloquearse.)
- ¿Puede haber más procesos `R` que núcleos? (Sí; “R” incluye listos para correr; el scheduler elige uno por núcleo.)

---

## Bloque 3 — Scheduler y preemption (30–48 min)

**Qué explicar (nivel licenciatura, sin implementar CFS):**

1. **Scheduler** = parte del kernel que asigna CPU a procesos (y en sistemas modernos, también a threads del kernel).
2. **Objetivos:** justicia, baja latencia interactiva, throughput, aprovechar varios núcleos.
3. **Ready queue:** procesos elegibles para CPU; no están bloqueados en I/O.
4. **Quantum / time slice:** en sistemas preemptivos (Linux lo es), un proceso puede ser **desalojado** aunque no haga I/O — temporizador de hardware → **involuntary** context switch.
5. **Política vs. mecanismo:** política (quién sigue); mecanismo (`context_switch` en kernel, cambio de page tables, TLB shootdown en detalle opcional).

**Diagrama — un CPU, tres procesos listos:**

```text
Ready:  P2 → P5 → P8
              │
              ▼
            CPU ← actualmente P2
```

Tras quantum o bloqueo de P2, otro proceso ocupa CPU.

**Conexión MiniServ:** fork-per-client **no crea paralelismo real de CPU** en un solo núcleo; crea **concurrencia** (muchos flujos de control) que el scheduler entrelaza.

**Error frecuente:** “tengo 20 hijos → uso 20 CPUs”. (Solo si hay ≥20 núcleos libres y los procesos compiten por CPU; además muchos estarán bloqueados en I/O.)

---

## Bloque 4 — Context switch (48–62 min)

**Qué es:** el kernel deja de ejecutar el proceso A en user space, guarda su **contexto de CPU** (registros, PC, stack pointer, etc.), restaura el de B, y reanuda B.

**Qué NO es (misconception):** copiar todo el espacio de direcciones — eso sería fork en cada switch. La memoria del proceso sigue en su lugar; cambia **quién** usa la CPU ahora.

**Costo pedagógico:**

- Directo: ejecutar código del kernel, guardar/restaurar registros, actualizar estructuras (`task_struct`).
- Indirecto: invalidar/calentar cachés y TLB (mencionar sin profundizar).

**Dónde observarlo:**

```bash
./demo_ctxt_switches
grep ctxt /proc/self/status   # mientras corre un demo
```

Campos:

- `voluntary_ctxt_switches`: el proceso se bloqueó (`read`, `sleep`, `accept`).
- `nonvoluntary_ctxt_switches`: el scheduler lo sacó de CPU (quantum, llegó otro más prioritario).

**strace (opcional avanzado):** no muestra cada context switch; use `/proc` o `perf stat` (mención).

---

## Bloque 5 — CPU-bound vs I/O-bound (62–78 min)

**Definiciones:**

- **CPU-bound:** pasa la mayor parte del tiempo computando en user space; `%CPU` alto en `top`.
- **I/O-bound:** pasa mucho tiempo **esperando** I/O (red, disco, sleep); `STAT` suele ser `S`; pocos ciclos útiles de CPU por unidad de tiempo.

**Demos en vivo:**

```bash
./demo_cpu_bound 8 &
./demo_io_bound 2 4 &
watch -n0.5 'ps -o pid,stat,pcpu,comm -p $(pgrep -n demo_cpu_bound),$(pgrep -n demo_io_bound)'
```

**Servidor típico:** **I/O-bound** en el camino crítico (`accept`, `read`, `write`). Por eso fork-per-client ayuda: mientras un hijo espera red, otro puede usar CPU o también bloquearse en I/O.

**Cuando fork-per-client duele:** clientes con transferencias enormes + N muy grande → muchos procesos, mucha memoria (copia de page tables, COW — adelanto Semana 10), muchos switches.

**Referencia README V4:** transferencia grande con `nc` y archivo de 10 000 MiB — observar load y memoria.

---

## Bloque 6 — Laboratorio (78–95 min)

### 6.1 Árbol de procesos bajo stress

Terminal 1:

```bash
cd miniserv/v4 && ./miniserv
```

Terminal 2:

```bash
for i in $(seq 1 30); do ( printf "hola %s\n" "$i" | nc -q 3 127.0.0.1 8080 ) & done
```

Terminal 3:

```bash
pstree -p $(pgrep -n -f 'v4/miniserv')
ps -o pid,ppid,stat,pcpu,rss,cmd --ppid $(pgrep -n -f 'v4/miniserv') | head -20
```

### 6.2 Padre vs hijos — ¿quién consume CPU?

Durante carga ligera, el padre debería estar casi siempre en `S` en `accept`. Los hijos picos breves en `R` al hacer echo.

### 6.3 Informe rápido (plantilla milestone)

Capturar: salida de `demo_bench_concurrency.sh`, una captura de `ps`/`pstree`, y números de context switches de un hijo ( `/proc/<pid>/status` ) antes y después de servir 10 clientes.

### 6.4 Demo `demo_two_children_sched`

Mientras corre, discutir: ¿el hijo CPU monopoliza la percepción de “lentitud” del hijo I/O en una sola CPU? (El I/O-bound termina en tiempo wall ~6 s; el CPU-bound también ~6 s; el scheduler reparte.)

---

## Bloque 7 — Discusión y cierre en clase (95–120 min)

Repaso del experimento, entrega del informe, puente: “cada hijo escribe logs → caos en `server.log` → necesitamos un consumidor dedicado (pipe, Semana 7)”.

---

# 9. Explicación teórica (profundidad)

## 9.1 ¿Qué es el scheduler?

1. **Qué es:** componente del kernel que decide **qué proceso (o hilo) ejecutar** en cada CPU y cuándo cambiar.
2. **Por qué existe:** hay más procesos listos que CPUs; hay que multiplexar tiempo de CPU.
3. **Problema que resuelve:** justicia y eficiencia sin que cada proceso necesite busy-wait.
4. **Cómo funciona (modelo):** procesos blocked no compiten; los ready entran en colas; eventos (I/O completo, timer) mueven procesos entre estados.
5. **Rol del kernel:** mantiene `task_struct`, ejecuta context switch, programa interrupciones de timer.
6. **MiniServ:** padre + hijos compiten por CPU; la mayoría del tiempo útil del servidor es **esperar I/O**.
7. **Error:** confundir scheduler con “el planificador de mi calendario” — aquí es **planificación de CPU**, no cola de sockets (esa es otra cola en el kernel de red).

## 9.2 Ready queue y estados

- **Running:** en CPU (como máximo uno por núcleo en el modelo simple).
- **Ready:** puede correr pero no tiene CPU aún.
- **Blocked/Waiting:** esperando evento (datos en socket, disco, timer).

Transición accept bloqueante: proceso pasa a blocked → **voluntary** ctxt switch → otro proceso usa CPU.

## 9.3 Preemption y quantum

Linux es **preemptive**: un proceso CPU-bound no puede quedarse para siempre (salvo casos especiales kernel). El **quantum** no es un número fijo que memoricen; la idea pedagógica es **turno limitado** → `nonvoluntary_ctxt_switches` sube en loops calientes.

## 9.4 Context switch

Participan: scheduler, código de arquitectura (guardar registros), posible cambio de **address space** (CR3 en x86) al cambiar de proceso — costo mayor que cambiar entre hilos del mismo proceso (misma address space).

## 9.5 CPU-bound vs I/O-bound y servidores

Servidores de red clásicos: diseño orientado a **no bloquear el único thread de atención** (V1) y a **no desperdiciar CPU** cuando hay I/O. Fork-per-client es un mecanismo simple de **concurrencia de procesos**, no la arquitectura final del curso (pool prefork, Semana 11).

## 9.6 Throughput vs latencia vs overhead

- **Throughput:** clientes atendidos por unidad de tiempo.
- **Latencia:** tiempo desde conexión hasta respuesta completa.
- **Overhead:** `fork`, destrucción de proceso, context switches, memoria por proceso.

V4 mejora latencia bajo clientes concurrentes vs V1; con N enorme, el overhead domina.

---

# 10. Diagramas

### Frontera user/kernel en un `read` bloqueante (hijo MiniServ)

```text
Hijo worker                         Kernel
    │                                  │
 read(client_fd) ─── syscall ────────►│ socket sin datos
    │                                  │ proceso → BLOCKED
    │◄────────── (context switch) ─────│ otro proceso corre
    │                                  │ ... datos llegan ...
    │◄────────── despertar ────────────│ proceso → READY → RUNNING
    │                                  │
 retorna bytes                         │
```

**Explicación:** el hijo no “gasta CPU esperando”; cede la CPU en bloqueo.

### V4 bajo ráfaga de clientes

```text
Tiempo ─────────────────────────────────────►

Padre:  accept──fork──accept──fork──accept──...
Hijo1:       [====read/write====]
Hijo2:            [====read/write====]
Hijo3:                 [====read/write====]
CPU:    |P|P|H1|H2|P|H1|H3|...|  (alternancia simplificada)
```

---

# 11. Demostraciones en C

Fuentes en `proyecto/semana5/programas_ejemplo/`. Compilar con `make`.

---

## Demo 1 — `demo_cpu_bound.c`

### Objetivo

Mostrar proceso **CPU-bound** (`STAT` often `R`, `%CPU` alto).

### Compilación

```bash
gcc -Wall -Wextra -O0 demo_cpu_bound.c -o demo_cpu_bound
```

### Ejecución

```bash
./demo_cpu_bound 5
```

### Salida esperada

Líneas en stderr con pid; proceso termina tras ~5 s.

### No determinismo

Valor de `acc`, `%CPU` exacto, número de preemptions.

### Explicación paso a paso

1. Bucle caliente en user space → mucho tiempo RUNNING.
2. Timer del kernel → posibles **nonvoluntary** context switches.
3. Al terminar, `_exit` implícito en return de main.

---

## Demo 2 — `demo_io_bound.c`

### Objetivo

Proceso **I/O-bound** vía `sleep` → blocked, **voluntary** context switches.

### Compilación / ejecución

```bash
gcc -Wall -Wextra demo_io_bound.c -o demo_io_bound
./demo_io_bound 2 3
```

### Salida esperada

Tres iteraciones con pausa ~2 s.

### No determinismo

Timing exacto de mensajes.

### Explicación paso a paso

1. `sleep` → syscall → blocked.
2. Al expirar timer, proceso ready → scheduler lo ejecuta de nuevo.

---

## Demo 3 — `demo_ctxt_switches.c`

### Objetivo

Leer contadores de context switch en `/proc/self/status`.

### Compilación / ejecución

```bash
gcc -Wall -Wextra -O0 demo_ctxt_switches.c -o demo_ctxt_switches
./demo_ctxt_switches
```

### Salida esperada (representativa)

```text
[inicio] voluntary=0 nonvoluntary=0
[tras CPU ~2s] voluntary=0 nonvoluntary=≥1
[tras sleep 2s] voluntary=≥1 nonvoluntary=...
```

### No determinismo

Contadores exactos dependen de carga del sistema.

### Explicación paso a paso

1. CPU caliente incrementa sobre todo **nonvoluntary**.
2. `sleep` incrementa **voluntary**.

---

## Demo 4 — `demo_two_children_sched.c`

### Objetivo

Un padre, un hijo CPU-bound y un hijo I/O-bound concurrentes — observar con `ps`.

### Compilación / ejecución

```bash
gcc -Wall -Wextra -O0 demo_two_children_sched.c -o demo_two_children_sched
./demo_two_children_sched
```

### Salida esperada

PIDs de hijos; padre espera con `waitpid`.

### No determinismo

Orden de terminación si duraciones iguales; `%CPU` instantáneo.

---

## Demo 5 — `demo_bench_concurrency.sh`

### Objetivo

Comparar wall time V1 vs V4 con N clientes `nc` en paralelo.

### Ejecución

```bash
chmod +x demo_bench_concurrency.sh
N=20 ./demo_bench_concurrency.sh
```

### Salida esperada

Dos bloques con “Tiempo wall aprox: … s”; V4 típicamente menor que V1 para el mismo N.

### No determinismo

Tiempos absolutos, carga de fondo del host.

### Explicación paso a paso

1. Arranca servidor en background.
2. Lanza N procesos `nc` (clientes independientes).
3. Mide intervalo wall; envía SIGTERM al padre (V4 apagado ordenado si implementado).

---

# 12. Herramientas Linux

### `ps -o pid,ppid,stat,pcpu,rss,cmd`

**Observar:** `STAT`, `%CPU`, memoria residente por hijo.  
**Relación:** estados y costo memoria de fork-per-client.  
**Alumno identifica:** padre en `S`; hijos mayormente `S` en I/O.

### `pstree -p <pid_padre>`

**Observar:** árbol padre → hijos efímeros.  
**Relación:** concurrencia = muchos procesos, no un solo hilo.

### `top` / `htop`

**Observar:** load average, `%CPU`, número de tareas.  
**Relación:** muchos procesos listos/blocked aumentan load aunque CPU no esté al 100%.

### `time`

```bash
time ( for i in $(seq 1 50); do nc -q 1 127.0.0.1 8080 </dev/null; done )
```

**Observar:** real vs user (en secuencial, real >> user por serialización).  
**Relación:** medición del experimento de la semana.

### `/proc/<pid>/status`

**Observar:** `voluntary_ctxt_switches`, `nonvoluntary_ctxt_switches`, `VmRSS`.  
**Relación:** evidencia de scheduling.

### `/proc/<pid>/stat`

**Observar:** campo de estado (tercer token), utime/stime (ticks en CPU).  
**Relación:** puente hacia métricas finales del curso (Semana 16).

### `grep -E 'ctxt|VmRSS' /proc/<pid>/status`

Atajo durante laboratorio.

---

# 13. Integración con MiniServ

### Estado inicial

MiniServ **V4** (`miniserv/v4/`): fork-per-client, señales, apagado ordenado. **No hay nueva versión obligatoria esta semana.**

### Problema

Bajo concurrencia, el diseño V4 **escala en procesos**, no magicamente en CPU. Hay que **entender** el comportamiento del kernel: estados, scheduler, costos.

### Mecanismo del Sistema Operativo

Planificación de CPU, colas ready/blocked, context switch.

### Cambio arquitectónico

Esta semana **no cambia el código del milestone anterior**; cambia el **modelo mental** y la **observabilidad**:

```text
MiniServ V4 (funcional)
      │
      ▼
¿Qué pasa con 50 clientes a la vez?
      │
      ▼
Scheduler + context switch + CPU vs I/O
      │
      ▼
Informe de medición + decisiones informadas (futuro: pool, logger, IPC)
```

---

# 14. Milestone semanal — Laboratorio de scheduling

No se entrega `miniserv/v5`. Se entrega **evidencia experimental** sobre V1 y V4.

## Requisitos funcionales

- Ejecutar `demo_bench_concurrency.sh` con al menos **N=10** y **N=30** (dos corridas).
- Capturar **pstree** o `ps` del padre V4 bajo ráfaga de al menos 15 clientes concurrentes.
- Registrar **context switches** de un proceso elegido (padre o hijo) leyendo `/proc/<pid>/status` antes y después de la ráfaga.
- Redactar conclusiones: secuencial vs fork-per-client, CPU-bound vs I/O-bound en el padre/hijos.

## Requisitos técnicos

- Usar herramientas del curso: `ps`, `pstree`, `time` o script provisto, `/proc`.
- Ejecutar al menos **dos** demos C de `programas_ejemplo/` y describir qué observaron en `STAT` o ctxt switches.

## Restricciones

- **No** implementar threads como “solución” al benchmark.
- **No** prefork pool (Semana 11).
- **No** es obligatorio `perf` (opcional para bonus).

## Criterios de aceptación

```text
[ ] reporte-semana6.md con tablas o capturas legibles
[ ] Dos valores de N documentados para V1 vs V4 (tiempo wall)
[ ] Captura ps/pstree bajo carga V4
[ ] Lectura /proc status (ctxt switches) explicada
[ ] Respuesta escrita a la pregunta puente del cierre
[ ] Demos C compiladas con -Wall -Wextra sin warnings
```

## Pruebas

```bash
cd proyecto/semana5/programas_ejemplo
make
N=10 ./demo_bench_concurrency.sh | tee bench-N10.txt
N=30 ./demo_bench_concurrency.sh | tee bench-N30.txt

cd ../../miniserv/v4 && ./miniserv &
SRV=$!
sleep 0.2
for i in $(seq 1 15); do nc -q 2 127.0.0.1 8080 </dev/null & done
wait
pstree -p $SRV
kill -TERM $SRV
```

---

# 15. Actividades de clase

### Pregunta 1

**¿Un proceso bloqueado en `accept` consume CPU?**

**Respuesta:** no de forma útil; está en estado esperando I/O. El kernel no lo ejecuta en user space hasta que haya conexión o interrupción (`EINTR`).

**Razonamiento:** blocked ≠ running; voluntary context switch al entrar en la syscall bloqueante.

### Pregunta 2

**¿Por qué fork-per-client mejora la atención concurrente aun con una sola CPU?**

**Respuesta:** mientras un proceso espera I/O de un cliente, el scheduler puede ejecutar otro proceso (otro hijo o el padre en `accept`).

**Razonamiento:** multiplexación de CPU + I/O overlap; no implica 20 CPUs.

### Pregunta 3

**¿Qué sube más en un bucle infinito sin I/O: voluntary o nonvoluntary context switches?**

**Respuesta:** principalmente **nonvoluntary** (preemption por quantum).

**Razonamiento:** no hay syscalls bloqueantes que cedan CPU voluntariamente.

### Ejercicio 1 (15 min)

Modificar `demo_io_bound.c` para que, entre sleeps, haga un bucle corto de CPU e identificar en `/proc/self/status` un incremento en ambos contadores.

### Ejercicio 2 (20 min)

Con MiniServ V4 en marcha, script bash que cada segundo imprima `$(pgrep -c miniserv)` mientras se lanzan 20 clientes; graficar a mano el pico de procesos.

### Predicción

**Situación:** V1 y V4; N=1 cliente con mensaje de 1 KB.  
**¿Tiempos muy distintos?** Probablemente **no** (casi igual).  
**Situación:** N=40 clientes lentos conectados a la vez.  
**¿V1 más lento en wall time?** **Sí** (serialización).  
**Determinista:** N y tamaño de mensaje; no el orden exacto de terminación en V4.

---

# 16. Errores y misconceptions

### "`fork()` crea un thread"

Incorrecto. `fork()` crea un **proceso** con su propio PID y tabla de recursos; threads comparten address space (Semana posterior si se menciona pthreads).

### "Más procesos siempre = más rápido"

Falso más allá de un punto: overhead de creación, memoria, context switches, presión en el scheduler.

### "Context switch copia toda la memoria del proceso"

Incorrecto. Se cambia contexto de CPU y punteros a page tables; la memoria del proceso permanece en su layout (COW en fork es otro tema).

### "Load average 4.0 significa 400% CPU usada"

Load incluye procesos **esperando CPU** y en ciertos estados de I/O; interpretar según número de núcleos.

### "El padre MiniServ debería estar en R todo el tiempo"

En un servidor sano, el padre pasa **mucho** tiempo blocked en `accept`.

---

# 17. Conexión profesional

- **Nginx / Apache prefork / worker models:** separación master/workers nace también de **no bloquear** el único proceso y de **controlar** cuántos procesos compiten (pool vs fork-per-request).
- **systemd:** servicios Type=simple con muchos procesos hijos — el kernel sigue aplicando la misma planificación.
- **Contenedores:** límites `cpu.cfs_quota_us` (cgroups) — capa sobre el mismo scheduler CFS.
- **PostgreSQL:** un backend por conexión (históricamente) vs pool — trade-off análogo a V1 vs V4 vs pool.

Arquitectura recurrente:

```text
Muchas conexiones
      │
      ▼
¿Un proceso secuencial?  → cola y latencia
¿Proceso por conexión?   → concurrencia + overhead
¿Pool acotado?           → equilibrio (Semana 11)
```

---

# 18. Historia y fun facts

1. **Multiprogramación** (años 60–70): se inventó para que la CPU no se ociosara mientras un job hacía I/O en cinta.
2. **Unix** trató el proceso como unidad de aislamiento; fork fue central en la historia de servidores.
3. Linux **CFS** (Completely Fair Scheduler, ~2007) reemplazó el enfoque de colas fijas O(1) para desktop/server modernos — en clase usamos modelo de tres estados + preemption.
4. El costo de **context switch** motivó arquitecturas **event-driven** (un proceso, muchos fds) — puente a Semana 13 (`select`/`epoll`), sin implementarlo aún.

---

# 19. Tarea semanal

## Objetivo

Entregar informe de scheduling sobre MiniServ V1 vs V4.

## Entregables

```text
reporte-semana6.md
(proyecto/semana5/programas_ejemplo compilado localmente, no obligatorio entregar binarios)
```

## Preguntas del reporte

1. Tabla: N=10 y N=30, tiempo wall V1 vs V4 (misma máquina, anotar núcleos: `nproc`).
2. Captura `pstree` bajo carga en V4 con al menos 15 clientes.
3. Antes/después de ráfaga: `voluntary_ctxt_switches` y `nonvoluntary_ctxt_switches` de un pid — ¿cuál subió más y por qué?
4. ¿El padre estuvo en `S` o `R` durante la carga? ¿Un hijo?
5. Una frase: ¿por qué MiniServ es principalmente I/O-bound?

## Empaquetado

```bash
tar czf informe-semana6.tar.gz reporte-semana6.md bench-*.txt
```

---

# 20. Cierre de la semana

## Conceptos fundamentales

1. Scheduler = asignación de CPU a procesos listos.
2. Estados: running, ready, blocked (modelo clásico).
3. Preemption + quantum → desalojo sin I/O.
4. Context switch = cambiar proceso en CPU, no copiar toda la memoria.
5. CPU-bound vs I/O-bound — servidores de red suelen ser I/O-bound.
6. Fork-per-client = concurrencia por procesos + costo de scheduling.
7. `/proc` y `ps` permiten **observar** scheduling desde user space.

## Mapa conceptual

```text
muchos clientes MiniServ V4
        │
        ▼
muchos procesos (padre + hijos)
        │
        ▼
scheduler + context switch
        │
        ▼
medición (time, ps, /proc)
```

## Qué cambia en la siguiente semana

**Semana 7 — Pipes:** cada hijo escribiendo directamente en el mismo log crea **contención** y mezcla de bytes. Necesitamos **IPC**: un proceso **logger** y `pipe()` como canal kernel entre workers y consumidor.

**Pregunta puente:**

> Si veinte hijos escriben a la vez en el mismo archivo, ¿el scheduler arregla la corrupción de datos?

---

## Apéndice para el profesor

### Orden sugerido de demos

1. `demo_bench_concurrency.sh` (motivación)
2. `demo_cpu_bound` + `demo_io_bound` en paralelo con `watch ps`
3. `demo_ctxt_switches`
4. `demo_two_children_sched`
5. Laboratorio MiniServ V4 + `/proc`

### Comandos rápidos proyector

```bash
nproc
grep -E 'model name|cpu MHz' /proc/cpuinfo | head -2
ps -L -o pid,tid,stat,pcpu,comm -p $(pgrep -n -f v4/miniserv)  # opcional: ver threads del mismo binario si los hubiera
```

### Nota sobre puertos

- V1 escucha **8082** (`miniserv/v1`).
- V3/V4 escuchan **8080** (`miniserv/v3`, `miniserv/v4`).

El script `demo_bench_concurrency.sh` ya usa esos puertos.

### Transferencia grande (opcional)

Desde README V4:

```bash
dd if=/dev/urandom of=/tmp/big.bin bs=1M count=100 status=progress
nc 127.0.0.1 8080 < /tmp/big.bin > /dev/null
```

Observar `VmRSS` del hijo y load durante la transferencia.
