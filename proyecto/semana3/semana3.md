# Semana 3 — Creación y administración de procesos

**Curso:** Sistemas Operativos (16 semanas)  
**Proyecto:** MiniServ — servidor TCP/HTTP multiproceso en C sobre Linux  
**Duración:** 1 clase × 120 minutos  

**Pregunta central de la semana:**

> MiniServ V1 atiende un cliente a la vez porque todo el echo ocurre en el **mismo** proceso. ¿Cómo puede el kernel crear **otra instancia** de ejecución para atender un cliente mientras el padre vuelve a `accept()`?

---

# A. Visión general de la semana

## Objetivos de aprendizaje

Al terminar la semana, el estudiante podrá:

1. **Explicar** qué es un proceso en Linux (PID, PPID, imagen en memoria, tabla de recursos) más allá de la intuición de la Semana 1.
2. **Describir** el comportamiento de `fork()`: duplicación de proceso, retorno distinto en padre e hijo, continuación en la misma instrucción lógica.
3. **Implementar** un patrón **fork-per-client** en MiniServ: padre acepta conexiones, hijo atiende el echo.
4. **Utilizar** `wait()` / `waitpid()` para recolectar hijos terminados y evitar procesos zombie en el milestone.
5. **Interpretar** salida de `ps`, `pstree` y `/proc` para reconstruir el árbol de procesos de MiniServ.
6. **Comparar** MiniServ V1 (secuencial) y V3 (multiproceso): concurrencia de atención vs. un solo hilo de control.
7. **Explicar** a alto nivel qué hace `exec()` y por qué MiniServ esta semana no lo necesita en el camino crítico del servidor.

## Motivación

**Problema concreto:** con MiniServ V1, mientras un cliente está conectado el padre permanece en `read`/`write` sobre `client_fd`. Un segundo cliente queda en la cola del kernel hasta que el primero cierre. En laboratorio esto se ve con dos terminales `nc`: la segunda no entra hasta terminar la primera.

**Por qué importa en Sistemas Operativos:** el cuello de botella no es TCP ni el puerto: es que hay **un solo proceso** ejecutando la lógica de atención. La concurrencia que buscamos es a nivel **proceso**, unidad clásica de aislamiento y scheduling en Unix.

**Manifestación en MiniServ:** necesitamos que, tras `accept()`, alguien atienda al cliente **sin** impedir que el listener vuelva a aceptar otro.

**Mecanismo del SO:** `fork()` — el kernel crea un proceso hijo casi idéntico; padre e hijo son entidades distintas con PIDs distintos y pueden ejecutar ramas de código diferentes.

## Conexión con semanas anteriores

```text
Semana 1: proceso = programa en ejecución; syscalls; strace
              │
              ▼
Semana 2: fds, sockets, accept/read/write bloqueantes; MiniServ V1 secuencial
              │
              ▼
problema: un cliente activo bloquea accept/read del siguiente
              │
              ▼
Semana 3: fork(), wait/waitpid, árbol de procesos → MiniServ V3
              │
              ▼
Semana 4 (puente): herencia de fds, zombies mal gestionados, /proc en detalle
```

---

# 7. Clase — 120 minutos

| Tiempo | Hilo narrativo |
| ------ | -------------- |
| 0–12 min | Experimento: dos clientes contra V1 |
| 12–28 min | Proceso, PID, PPID, tabla de procesos |
| 28–45 min | `fork()`: mecanismo y semántica de retorno |
| 45–58 min | Padre/hijo, `_exit` vs `exit`, introducción `exec` |
| 58–72 min | `wait` / `waitpid`, zombies (motivación) |
| 72–95 min | Laboratorio: MiniServ V3 + `pstree` / `ps` |
| 95–110 min | Ejercicios, predicción, preguntas |
| 110–120 min | Milestone, cierre, puente Semana 4 |

---

## Bloque 1 — Problema motivador (0–12 min)

**Demostración:** `./miniserv` desde `miniserv/v1`. Terminal A: `nc 127.0.0.1 8080` (quedar conectado). Terminal B: segundo `nc` — espera.

**Preguntas:**

1. ¿El kernel rechazó la conexión de B? (No: está en cola; el padre aún no llamó `accept` o no llegó a atender.)
2. ¿Cuántos procesos user space atienden clientes? (Uno.)
3. ¿Qué tendría que duplicarse para atender A y B “a la vez”? (La **ejecución** — otro proceso.)

**Diagrama:**

```text
V1 — un proceso
[ MiniServ ]──accept──► cliente A (en read/write)
                         cliente B (esperando)

V3 — objetivo
[ Padre MiniServ ]──accept──► fork ──► [ Hijo ] atiende A
                  └──accept──► fork ──► [ Hijo ] atiende B
```

---

## Bloque 2 — Proceso, PID y PPID (12–28 min)

**Qué explicar:** proceso = instancia viva con identidad **PID** (único mientras dura), **PPID** (PID del padre), estado (`R/S/D/Z/...`), recursos (fds, memoria, contexto de CPU).

**Herramientas:**

```bash
cat /proc/self/status | grep -E '^(Pid|PPid|Name):'
ps -o pid,ppid,stat,cmd
```

**Conexión MiniServ:** al arrancar V1 hay un PID; tras V3 habrá un árbol: padre + hijos efímeros por cliente.

**Error frecuente:** confundir PID con fd — números distintos, namespaces distintos en el kernel.

---

## Bloque 3 — `fork()` (28–45 min)

**Qué explicar:**

- `fork()` es syscall: el kernel duplica el proceso llamador.
- Tras `fork()`, **ambos** continúan después de la llamada (salvo scheduling).
- Retorno: **`-1`** error; **`0`** en hijo; **`>0`** (PID hijo) en padre.

**Pizarrón:**

```text
        fork()
          │
    ┌─────┴─────┐
    ▼           ▼
  Padre       Hijo
 pid=P       pid=C
 ret=C       ret=0
```

**Demostración:** `demo_fork_basic.c`, `strace -e fork,getpid ./demo_fork_basic`.

**Pregunta de predicción:** ver Bloque 7 / §15 — `demo_fork_order.c`.

**Conexión MiniServ:** tras `accept`, `fork()`; hijo ejecuta echo; padre vuelve al loop.

**Error frecuente:** “`fork` crea un thread” — falsos; threads comparten espacio de direcciones; procesos hijos son entidades separadas (profundizaremos memoria en Semana 10).

---

## Bloque 4 — Roles padre/hijo e introducción a `exec` (45–58 min)

**Padre en V3:** conserva `listen_fd`, cierra `client_fd` tras fork (no lo usa), acepta más conexiones, recolecta hijos.

**Hijo en V3:** cierra `listen_fd` (no debe competir por `accept`), atiende `client_fd`, termina con `_exit(0)`.

**`_exit` vs `exit`:** en hijo tras fork, `exit()` puede ejecutar handlers/flush de stdio duplicado; en servidores se prefiere `_exit` en hijo (detalle que Semana 4 ampliará).

**Introducción `exec`:** `fork` crea copia; `exec*` **reemplaza** el programa manteniendo PID. Patrón shell: `fork` + `exec` + `wait`. MiniServ **no** usa exec esta semana — el hijo sigue siendo el mismo binario con rama `if (pid == 0)`.

**Demostración:** `demo_exec_intro.c`.

---

## Bloque 5 — `wait` / `waitpid` (58–72 min)

**Qué explicar:**

- Al terminar, el hijo queda **zombie** (`Z`) hasta que el padre haga `wait*` — tabla de procesos conserva exit status.
- `wait(&status)` espera **cualquier** hijo.
- `waitpid(pid, &status, options)` más control; `WNOHANG` no bloquea (útil en loop del padre).

**MiniServ V3:** tras cada `fork`, padre llama `reap_children()` con `waitpid(-1, NULL, WNOHANG)` para limpiar hijos ya terminados sin bloquear el accept (simplificación pedagógica; Semana 5 usará señales).

**Demostración:** `demo_fork_wait.c` y `demo_zombie.c` — comparar wait inmediato vs padre que demora 20 s en `wait` (mostrar `Z` en `ps`).

**Diagrama zombie:**

```text
Hijo termina ──► estado Z (cuerpo liberado, slot en tabla)
                      │
                 wait() en padre
                      │
                      ▼
                 entrada eliminada
```

---

## Bloque 6 — Laboratorio MiniServ V3 (72–95 min)

**Referencia:** [`miniserv/v3/src/server.c`](../miniserv/v3/src/server.c).

**Arquitectura:**

```text
MiniServ V1
      │
      ▼
un cliente bloquea todo el echo
      │
      ▼
fork() — hijo atiende, padre accept
      │
      ▼
MiniServ V3 (fork-per-client)
```

**Actividades:**

1. `cd miniserv/v3 && make && ./miniserv`
2. Dos `nc` simultáneos — ambos reciben echo.
3. `pstree -p $(pgrep -n -f 'miniserv/v3/miniserv')` — padre e hijos.
4. `strace -f -e trace=process,accept,read,write,close -p <pid_padre>` — ver `fork` y que padre no hace read del cliente.

**Qué debe notar el alumno:** el padre pasa la mayor parte del tiempo en `accept`; hijos en `read`/`write`.

---

## Bloque 7 — Ejercicios y cierre parcial (95–110 min)

Ver §15 (preguntas, ejercicios, predicción con `demo_fork_order.c`).

---

# 9. Explicación teórica

## 9.1 Proceso (repaso profundo)

**Qué es:** entidad de ejecución con PID, contexto (registros, PC), espacio de direcciones, fds abiertos, credenciales.

**Kernel:** mantiene `task_struct` (Linux), estados, relación padre-hijo, contabilidad para `wait`.

**MiniServ:** de un proceso a un **árbol** efímero padre → hijos por conexión.

**Errores:** creer que terminar `main` del hijo “borra” al padre; creer que dos procesos comparten variables globales modificables sin IPC (cada uno tiene su copia tras fork — COW lo optimiza, Semana 10).

## 9.2 PID y PPID

**PID:** identificador del proceso.  
**PPID:** PID del padre que lo creó (`fork` o init adopta huérfanos, Semana 4).

**MiniServ:** padre PPID = shell; hijo PPID = PID del miniserv padre.

## 9.3 `fork()`

**Qué es:** syscall de creación de proceso por duplicación del llamador.

**Por qué existe:** Unix modela concurrencia y aislamiento con procesos; `fork` es la primitiva histórica.

**Cómo funciona (simplificado):** kernel clona estructuras, marca quién es padre/hijo, ajusta retorno de `fork`, encola ambos como runnable.

**MiniServ:** bifurcación tras cada `accept` exitoso.

**Errores:** asumir que padre e hijo ejecutan solo una rama sin `if (pid == 0)`; olvidar comprobar `-1`.

## 9.4 `wait()` y `waitpid()`

**Qué resuelven:** sincronización padre-hijo y liberación de slot zombie; opcionalmente exit status.

**Kernel:** bloquea al padre si no hay hijo terminado (sin `WNOHANG`); transfiere status.

**MiniServ V3:** `waitpid(-1, NULL, WNOHANG)` en el loop del padre.

**Errores:** no hacer wait nunca → zombies; hacer wait en el hijo incorrecto.

## 9.5 Introducción a `exec()`

Referencia de variantes (`execl`, `execvp`, `execve`, …): [`familia-exec.md`](familia-exec.md).

**Qué es:** familia de syscalls que cargan nuevo programa en el proceso **actual**.

**Patrón:** `fork` + `exec` + `wait` (shell, CGI clásico, workers que lanzan binarios).

**MiniServ:** esta semana el hijo ejecuta código del mismo `server.c`; más adelante podría exec a otro binario (no requerido).

**Errores:** llamar `exec` en el padre sin fork y destruir el listener; olvidar que `exec` no retorna si tiene éxito.

---

# 10. Diagramas

## 10.1 Fork-per-client

```text
                    ┌─────────────────┐
                    │  Padre MiniServ │
                    │  listen_fd      │
                    └────────┬────────┘
                             │ accept()
                             ▼
                        client_fd
                             │
                          fork()
                    ┌────────┴────────┐
                    ▼                 ▼
              cierra client_fd   close(listen_fd)
              reap (waitpid)     serve_client()
              accept otra...     _exit(0)
```

## 10.2 Árbol de procesos (instantánea)

```text
bash(1000)
 └── miniserv(5000)          ← padre
      ├── miniserv(5001)     ← hijo atiende cliente A
      └── miniserv(5002)     ← hijo atiende cliente B
```

(PIDs ilustrativos; hijos desaparecen al terminar.)

---

# 11. Demostraciones en C

Los fuentes están en `programas_ejemplo/`.

## 11.1 `demo_fork_basic.c`

**Objetivo:** retorno de `fork`, PIDs distintos.

**Compilación:** `gcc -Wall -Wextra demo_fork_basic.c -o demo_fork_basic`

**Salida representativa:**

```text
[padre] pid=4000 hijo=4001
[hijo]  pid=4001 ppid=4000
```

**No determinismo:** orden de líneas padre/hijo puede variar.

## 11.2 `demo_fork_order.c`

**Objetivo:** predicción A/B/C/D con `fork`.

**Salida:** siempre `A` primero; luego `B`/`C` en orden no garantizado; dos `D`.

## 11.3 `demo_fork_wait.c`

**Objetivo:** padre bloqueado en `wait` ~2 s mientras hijo duerme.

## 11.4 `demo_zombie.c`

**Objetivo:** hijo termina; padre **no** hace `wait` durante 20 s → estado **Z** en `ps`; al final `wait()` recolecta.

**Observación:** `ps -o pid,ppid,stat,cmd -p <pid_hijo>` mientras el padre duerme.

## 11.5 `demo_exec_intro.c`

**Objetivo:** hijo reemplazado por `echo`; padre hace `wait`.

**Salida:** `hola desde exec en el hijo` en stdout; mensaje del padre en stderr.

---

# 12. Herramientas Linux

## `ps`

```bash
ps -o pid,ppid,stat,cmd -p <pid_padre>
ps --ppid <pid_padre> -o pid,stat,cmd
```

**Observar:** hijos en `S` durante I/O; brevemente `Z` si quitas `waitpid` para demo.

## `pstree`

```bash
pstree -p <pid_padre>
```

**Relación:** árbol padre → hijos con PIDs.

## `strace`

```bash
strace -f -e process,accept,clone,fork,wait4 ./miniserv
```

(Linux puede mostrar `clone` en lugar de `fork` en algunos trazados.)

## `/proc`

```bash
cat /proc/<pid>/status | grep -E '^(Pid|PPid|State):'
ls -l /proc/<pid>/fd
```

**Semana 4** profundizará fds heredados en hijos.

---

# 13. Integración con MiniServ

## Estado inicial

MiniServ V1/V2 (`miniserv/v1/` TCP echo, `miniserv/v2/` HTTP secuencial): un solo proceso en el loop `accept`; un cliente activo retrasa a los demás. V3 parte de la misma limitación (implementación actual: fork sobre echo TCP en `v3/`).

## Problema

Cliente lento o interactivo impide aceptar/atender otros con prontitud.

## Mecanismo del SO

`fork()` + separación de roles + `waitpid` para lifecycle.

## Cambio arquitectónico

```text
MiniServ V1 (1 proceso)
      │
      ▼
cuello: read/write en el mismo proceso que accept
      │
      ▼
fork() — hijo = sesión cliente, padre = listener
      │
      ▼
MiniServ V3 en miniserv/v3/
```

---

# 14. Milestone semanal — MiniServ V3

## Requisitos funcionales

- Mismo comportamiento echo TCP puerto 8080 que V1.
- Varios clientes pueden estar atendidos **simultáneamente** (cada uno en un hijo).
- El servidor padre sigue vivo tras desconexiones.

## Requisitos técnicos

- `fork()` tras cada `accept` exitoso.
- Hijo: atiende cliente y termina con `_exit(0)`.
- Padre: cierra `client_fd` tras fork; no hace echo del cliente en el padre.
- Hijo: cierra `listen_fd` antes de atender.
- Recolectar hijos con `wait()` o `waitpid` (p. ej. `WNOHANG` en el loop).
- Compilar con `-Wall -Wextra`.

## Restricciones

- **No** threads.
- **No** depender de `SIGCHLD` como mecanismo principal (Semana 5).
- HTTP no requerido.

## Criterios de aceptación

```text
[ ] Código en miniserv/v3/ (o carpeta v3 acordada)
[ ] make sin warnings
[ ] Dos nc simultáneos reciben echo sin esperar al otro
[ ] pstree muestra padre e hijos bajo carga
[ ] ps no acumula hijos Z tras cerrar clientes (waitpid activo)
[ ] strace -f muestra fork en el padre e read/write en hijos
```

## Pruebas

```bash
cd miniserv/v3 && make && ./miniserv

# Terminales 2 y 3 (simultáneas)
nc 127.0.0.1 8080

PID=$(pgrep -n -f 'v3/miniserv')
pstree -p "$PID"
ps --ppid "$PID" -o pid,stat,cmd
```

---

# 15. Actividades de clase

### Pregunta 1

**¿Por qué el padre cierra `client_fd` después de `fork` si el hijo lo necesita?**

**Respuesta:** el kernel mantiene el socket abierto mientras **algún** proceso conserve un fd apuntando a él; padre e hijo tienen fds duplicados tras fork — el hijo conserva el suyo; el padre cierra el suyo para no tener fds innecesarios ni hacer I/O por error.

### Pregunta 2

**¿Qué retorna `fork()` en el hijo y por qué es útil?**

**Respuesta:** `0` — permite escribir `if (pid == 0) { ... hijo ... }` sin conocer de antemano el PID del hijo.

### Pregunta 3

**¿MiniServ V3 usa threads?**

**Respuesta:** no; usa **procesos**; concurrencia vía scheduler entre procesos, no hilos compartiendo memoria.

### Ejercicio 1 (15 min)

Añadir log en stderr del hijo: `atendiendo pid=<pid> client_fd=<n>` al entrar y al salir.

### Ejercicio 2 (20 min)

Contador global de hijos creados (variable en padre, solo padre incrementa) — imprimir tras cada fork.

### Predicción — `demo_fork_order.c`

**¿Cuántas veces aparece `D`?** Dos (padre e hijo). **¿Orden B vs C?** No determinista (scheduler).

---

# 16. Errores y misconceptions

### "`fork()` crea un thread"

Falso: crea **proceso**; memoria vista como copia (COW), fds heredados.

### "Padre e hijo comparten `client_fd` como el mismo entero mágico"

Tras fork hay **dos** entradas en tablas distintas que apuntan al mismo objeto kernel hasta cerrar.

### "No hace falta wait si el hijo ya terminó"

Sin wait, queda **zombie** hasta wait del padre.

### "El hijo puede hacer `accept` en el mismo puerto"

Mal diseño: compite con el padre; además hijo debe cerrar `listen_fd`.

### "exec y fork son lo mismo"

`fork` duplica; `exec` reemplaza imagen.

---

# 17. Conexión profesional

- **Apache prefork / Nginx workers:** variantes de “proceso por conexión” o pool — misma idea de aislamiento por proceso.
- **PostgreSQL:** un backend por conexión (proceso).
- **Shell:** `fork` + `exec` + `wait` en cada comando.
- **Docker:** procesos con PIDs distintos en namespace; el modelo fork sigue en el host.

---

# 18. Historia y fun facts

1. **`fork` (1970s Unix):** diseño simple para shells; hoy Linux implementa fork vía `clone` con flags.
2. **Zombie apocalipsis:** servidores que olvidan wait agotan PIDs — caso real de resource leak.
3. **Copy-on-write:** fork “barato” en memoria hasta escribir (Semana 10).

---

# 19. Tarea semanal

## Objetivo

Entregar MiniServ V3 y reporte con árbol de procesos.

## Entregables

```text
miniserv/v3/
reporte-semana3.md
```

## Preguntas del reporte

1. Captura de `pstree` con al menos dos hijos vivos.
2. ¿Qué syscalls diferencia `strace` entre padre e hijo?
3. ¿Qué pasó en `ps` si comentas temporalmente `waitpid`?
4. Compara tiempo percibido V1 vs V3 con dos clientes.

## Empaquetado

```bash
tar czf miniserv-semana3.tar.gz miniserv/v3 reporte-semana3.md
```

---

# 20. Cierre de la semana

## Conceptos fundamentales

1. Proceso = PID + recursos + contexto de ejecución.
2. `fork()` duplica; retorno 0 en hijo, PID hijo en padre.
3. Fork-per-client: padre escucha, hijo atiende.
4. Cerrar fds correctos en cada rol (`listen_fd` en hijo, `client_fd` en padre).
5. `wait`/`waitpid` recolectan hijos y evitan zombies.
6. `exec` reemplaza programa; patrón distinto al de V3 esta semana.
7. `ps`/`pstree`/`strace -f` observan el árbol de MiniServ.

## Mapa conceptual

```text
cliente lento bloquea V1
        │
        ▼
fork() — concurrencia por procesos
        │
        ▼
waitpid — lifecycle del hijo
        │
        ▼
MiniServ V3
```

## Qué cambia en la siguiente semana

**Semana 4:** herencia de fds en detalle, duplicación tras fork, más zombies/huérfanos, limpieza rigurosa y observación con `/proc/<pid>/fd`.

**Pregunta puente:**

> Si el hijo hereda **todos** los fds del padre, ¿qué ocurre si olvidamos cerrar `listen_fd` en el hijo o dejamos abiertos decenas de `client_fd`?

---

## Apéndice para el profesor

### Demo sin wait (solo en clase, no entregar)

Comentar `reap_children()` y cerrar un cliente; `ps -o pid,stat,cmd --ppid <padre>` → `Z`.

### Orden sugerido de demos

1. `demo_fork_basic.c`  
2. `demo_fork_order.c` (predicción)  
3. `demo_fork_wait.c`
4. `demo_zombie.c`
5. `demo_exec_intro.c`
6. MiniServ V3
