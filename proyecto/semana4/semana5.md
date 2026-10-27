# Semana 5 — Señales

**Curso:** Sistemas Operativos (16 semanas)
**Proyecto:** MiniServ — servidor TCP/HTTP multiproceso en C sobre Linux
**Duración:** 1 clase × 120 minutos

**Pregunta central de la semana:**

> MiniServ V3 recolecta hijos haciendo *polling* con `waitpid(..., WNOHANG)` en el loop de `accept`. ¿Cómo puede el **kernel notificarnos de forma asíncrona** cuando un hijo termina o cuando el operador pide apagar el servidor — sin mezclar lógica peligrosa dentro del handler?

---

# A. Visión general de la semana

## Objetivos de aprendizaje

Al terminar la semana, el estudiante podrá:

1. **Explicar** qué es una señal en Unix/Linux: evento software asíncrono entregado a un proceso por el kernel.
2. **Diferenciar** acción por defecto, ignorar (`SIG_IGN`) e handler personalizado (`sigaction`).
3. **Implementar** handlers mínimos y seguros usando `volatile sig_atomic_t` y trabajo diferido en el loop principal.
4. **Utilizar** `SIGCHLD` para recolectar hijos con `waitpid` en el handler (patrón reaper) y eliminar dependencia exclusiva del polling.
5. **Implementar** cierre ordenado ante `SIGINT` y `SIGTERM`: dejar de aceptar clientes, cerrar `listen_fd`, recolectar hijos, salir limpio.
6. **Diagnosticar** interrupciones de syscalls bloqueantes (`EINTR`) y decidir cuándo reintentar vs. cuándo usar `SA_RESTART`.
7. **Relacionar** `SIGPIPE` con escritura a sockets cerrados (puente hacia robustez en semanas posteriores).

## Motivación

**Problema concreto:** con MiniServ V3 en producción de laboratorio ocurren dos situaciones incómodas:

1. **Apagado brusco:** el operador presiona Ctrl+C o `kill <pid>`. El proceso desaparece de inmediato; pueden quedar hijos huérfanos o sockets en estado inconsistente; no hay mensaje de “apagando…”.
2. **Recolección oportunista:** `reap_children()` solo corre cuando el padre vuelve al loop. Funciona, pero el modelo mental correcto en servidores Unix es: *“cuando un hijo muere, el kernel me avisa”* (`SIGCHLD`).

**Por qué importa en Sistemas Operativos:** las señales son el mecanismo clásico de **notificación asíncrona** entre kernel y proceso (y entre procesos). Complementan el modelo síncrono de syscalls (`wait` bloqueante) con eventos que pueden llegar **en cualquier instante**, incluso mientras el proceso está bloqueado en `accept()`.

**Manifestación en MiniServ:** necesitamos un **master** que reaccione a señales de control (`SIGINT`/`SIGTERM`) y de lifecycle (`SIGCHLD`) sin corromper estado ni violar las reglas de async-signal-safety.

**Mecanismo del SO:** entrega de señales, desvío a user space, ejecución de handler (o acción por defecto), posible interrupción de syscalls con `EINTR`.

## Conexión con semanas anteriores

```text
Semana 3: fork-per-client; waitpid(WNOHANG) en el loop
              │
              ▼
Semana 4: herencia de fds; cerrar listen_fd/client_fd; zombies en /proc
              │
              ▼
problema: apagado caótico + recolección acoplada al accept loop
              │
              ▼
Semana 5: sigaction, SIGCHLD, SIGINT/SIGTERM, EINTR → MiniServ V4
              │
              ▼
Semana 6: muchos procesos vivos → scheduling y context switch
```

---

# 7. Clase — 120 minutos


| Tiempo       | Hilo narrativo                                |
| ------------ | --------------------------------------------- |
| 0–10 min    | Demo: Ctrl+C sobre V3; hijos y sockets        |
| 10–25 min   | Qué es una señal; tabla de señales comunes |
| 25–40 min   | `sigaction`, async-signal-safety, flags       |
| 40–55 min   | SIGINT/SIGTERM y cierre ordenado              |
| 55–70 min   | SIGCHLD y reaper; contrasto con polling       |
| 70–85 min   | EINTR en`accept`/`read`                       |
| 85–105 min  | Laboratorio: MiniServ V4                      |
| 105–120 min | Predicción, preguntas, milestone, cierre     |

---

## Bloque 1 — Problema motivador (0–10 min)

**Demostración automática (recomendada en proyector):**

```bash
cd proyecto/semana4/programas_ejemplo
./demo_bloque1_apagado_brusco.sh --pause
```

El script compila MiniServ V3 si hace falta, deja un cliente bloqueado en `read`, mata **solo al padre** con SIGINT (vía `demo_bloque1_launcher`, porque un job `&` en bash ignora SIGINT) y muestra si el **hijo worker sigue vivo** como huérfano.

**Demostración manual (3 terminales):**

```bash
cd miniserv/v3 && ./miniserv
# otra terminal: nc 127.0.0.1 8080 (dejar conectado)
# Ctrl+C en el servidor
ps -ef | grep miniserv
```

**Preguntas:**

1. ¿Murieron todos los procesos relacionados al instante?
2. ¿El hijo que estaba en `read` recibió la misma señal que el padre?
3. ¿Qué debería hacer un servidor “bien educado” antes de `_exit`?

**Diagrama:**

```text
Operador                Kernel                 MiniServ padre              Hijo worker
   │                       │                          │                          │
 Ctrl+C ──► SIGINT ───────►│ entrega a foreground     │                          │
                           │                          │ (default: terminar)      │
                           │                          X                          │
                           │                          │                    ¿SIGINT?
                           │                          │                    ¿huérfano?
```

**Conexión MiniServ:** V4 instala handlers en el **padre**; hijos pueden mantener defaults o ignorar señales de control según diseño (esta semana: hijos efímeros que solo atienden un cliente).

---

## Bloque 2 — Concepto: señales (10–25 min)

**Qué explicar:**

- Señal = notificación software (número + semántica), no un byte en un fd.
- Origen: terminal (`SIGINT`), otro proceso (`kill`), kernel (`SIGCHLD` al terminar hijo, `SIGSEGV` por fault).
- Destino: un proceso (o grupo de procesos); el kernel decide cuándo entregar (puede estar pendiente).

https://www.chromium.org/chromium-os/developer-library/reference/linux-constants/signals/


**Pizarrón — capas:**

```text
Evento (hijo exit, Ctrl+C, kill -TERM)
        │
        ▼
Kernel: marca señal pendiente en struct task_struct
        │
        ▼
Al volver a user space (o al salir de syscall)
        │
        ├── acción por defecto (Term, Core, Ign, Stop)
        ├── SIG_IGN
        └── handler registrado con sigaction
```

**Señales que nos importan esta semana:**


| Señal  | Origen típico               | Acción default | Uso en MiniServ                          |
| ------- | ---------------------------- | --------------- | ---------------------------------------- |
| SIGINT  | Ctrl+C                       | Terminar        | Apagado desde terminal                   |
| SIGTERM | `kill PID`                   | Terminar        | Apagado estándar (systemd, Docker stop) |
| SIGCHLD | Hijo cambió estado          | Ignorar*        | Aviso: hacer`waitpid`                    |
| SIGPIPE | write en pipe/socket cerrado | Terminar        | Cliente cerró antes del echo            |

\*En Linux antiguo SIGCHLD no se ignoraba automáticamente; en servidores siempre instalamos handler explícito.

**Preguntas a estudiantes:**

- ¿Una señal “interrumpe” el CPU en medio de una instrucción de C? (Modelo pedagógico: en user space el handler corre en un contexto especial; detalle exacto depende del momento — lo importante es que **no** es una llamada normal de función.)
- ¿`SIGCHLD` significa “un hijo murió”? (Casi siempre nos interesa eso; técnicamente también stop/continue — en MiniServ asumimos hijos que solo hacen `_exit`.)

---

## Bloque 3 — `sigaction` y handlers seguros (25–40 min)

**API principal (no `signal()` en código nuevo):**

```c
struct sigaction sa;
sa.sa_handler = handler;
sigemptyset(&sa.sa_mask);
sa.sa_flags = 0;
sigaction(SIGINT, &sa, NULL);
```

**Reglas de oro del handler:**

1. **Async-signal-safe only:** solo funciones listadas en `signal-safety(7)`. En la práctica del curso: asignar `volatile sig_atomic_t`, escribir a un fd con `write()` (sí), evitar `printf`, `malloc`, `exit` en el handler.
2. **Trabajo pesado fuera:** cerrar sockets, `waitpid` en bucle largo con logs — acceptable en handler **solo** para `waitpid` + flag; muchos estilos mueven even `waitpid` a SIGCHLD handler (es el patrón clásico).
3. **Variables compartidas:** `volatile sig_atomic_t g_stop` para comunicar handler → main.

**Flags útiles:**

- `SA_RESTART`: reinicia syscalls interrumpidas (menos `EINTR`; puede ocultar puntos de control).
- Sin `SA_RESTART`: útil para que `accept` retorne y el loop revise `g_stop`.

**Demostración en vivo:** `programas_ejemplo/demo_sigaction_flag.c`

**Error frecuente:** llamar `printf` en el handler — puede deadlock si el main estaba en `printf`.

---

## Bloque 4 — Cierre ordenado (40–55 min)

**Objetivo de MiniServ V4 al recibir SIGINT/SIGTERM:**

```text
SIGINT / SIGTERM
       │
       ▼
 handler: g_stop = 1  (y/o g_reload = 0)
       │
       ▼
 loop principal:
   - dejar de llamar accept (o salir si listen_fd cerrado)
   - close(listen_fd)  → nuevos clientes rechazados por kernel
   - esperar hijos vivos (wait loop o SIGCHLD hasta cola vacía)
   - mensaje en stderr
   - exit(0)
```

**Diagrama de estados del padre:**

```text
        ┌───────────────┐
        │   RUNNING     │
        │ accept loop   │
        └───────┬───────┘
                │ SIGTERM / SIGINT
                ▼
        ┌───────────────┐
        │  SHUTTING     │
        │ g_stop=1      │
        │ close listen  │
        └───────┬───────┘
                │ no hijos vivos
                ▼
        ┌───────────────┐
        │   EXIT        │
        └───────────────┘
```

**Contraste:**

- **Mal:** handler llama `exit(0)` inmediatamente → hijos siguen, zombies posibles.
- **Bien:** handler solo marca flag; main cierra y espera.

**Herramienta:**

```bash
kill -TERM $(pgrep -f 'miniserv')
# comparar con kill -9 (SIGKILL, no interceptable — semana de robustez)
```

---

## Bloque 5 — SIGCHLD y reaper (55–70 min)

**Modelo:**

```text
Hijo _exit(0)
     │
     ▼
Kernel: estado ZOMBIE + envía SIGCHLD al padre
     │
     ▼
Handler SIGCHLD:
     while (waitpid(-1, &status, WNOHANG) > 0)
         ;
     │
     ▼
Zombie eliminado de la tabla de procesos
```

**Por qué `while` y no un solo `waitpid`:** una señal no garantiza un solo hijo terminado; pueden acumularse varios exit antes de entregar SIGCHLD.

**Guardar/restaurar `errno` en el handler** si el main usa syscalls que consultan `errno` tras señal.

**Contrato pedagógico V3 → V4:**

- V3: polling en loop — válido, simple.
- V4: **SIGCHLD obligatorio** como mecanismo principal; se permite llamar reaper también al apagar por redundancia.

**Demostración:** `demo_sigchld_reap.c` + en paralelo:

```bash
watch -n1 'ps -o pid,stat,cmd --ppid <PADRE>'
```

---

## Bloque 6 — EINTR (70–85 min)

Cuando una señal llega mientras el proceso está bloqueado en `accept`, `read`, `write`:

- La syscall puede fallar con `-1` y `errno == EINTR`.
- Patrón servidor:

```c
client_fd = accept(listen_fd, ...);
if (client_fd < 0) {
    if (errno == EINTR) {
        if (g_stop) break;
        continue;
    }
    perror("accept");
    break;
}
```

**Demostración:** `demo_eintr_accept.c` en puerto 9099.

**Discusión:** `SA_RESTART` vs. loop explícito — en MiniServ preferimos **control explícito** para revisar `g_stop` tras cada interrupción.

---

## Bloque 7 — Laboratorio MiniServ V4 (85–105 min)

**Checklist de implementación (padre):**

1. `setup_signals()` con `sigaction` para SIGINT, SIGTERM, SIGCHLD.
2. `volatile sig_atomic_t g_stop = 0`.
3. Handler SIGCHLD → `reap_children()` (misma lógica que V3 pero invocable desde handler).
4. Loop: si `g_stop`, break; `accept` con manejo `EINTR`.
5. Al salir del loop: `close(listen_fd)`; drenar hijos (`while (waitpid(...) > 0)` bloqueante está bien al apagar).

**Observación:**

```bash
strace -e trace=signal,accept,wait4 -f ./miniserv
```

**Predicción antes de strace:** ¿veremos SIGCHLD antes o después del `waitpid` en el handler?

---

## Bloque 8 — Discusión y cierre (105–120 min)

Repaso milestone, entrega, puente a Semana 6 (muchos procesos → scheduler).

---

# 9. Explicación teórica (profundidad)

## 9.1 ¿Qué es una señal?

1. **Qué es:** mecanismo de notificación asíncrona; cada señal tiene un número (`SIGINT` = 2, etc.) y semántica.
2. **Por qué existe:** procesos deben reaccionar a eventos externos sin busy-wait; el kernel centraliza entrega y permisos (`kill` requiere credenciales).
3. **Problema que resuelve:** coordinación kernel↔proceso y proceso↔proceso sin canal dedicado previo.
4. **Cómo funciona (modelo):** kernel asocia acciones/disposiciones por señal en el PCB; al producirse el evento, marca pending; al retornar a user mode, ejecuta handler o default.
5. **Rol del kernel:** genera señales (faults, SIGCHLD), respeta máscaras (`sigprocmask`), entrega a hilos/procesos correctos.
6. **MiniServ:** padre escucha control y lifecycle; hijos cortos pueden no registrar nada.
7. **Error conceptual:** “las señales son como mensajes en un socket” — no hay buffer de bytes ni `read()` de señales; es un bit/flag + código de handler.

## 9.2 Acciones por defecto vs. handlers

- **Term:** proceso muere (`SIGINT`, `SIGTERM`, `SIGPIPE` default).
- **Ign:** se descarta (`SIGCHLD` en muchos sistemas modernos si no hay handler — no confiar).
- **Core:** dump + term (`SIGSEGV`).

`sigaction` permite guardar disposición anterior (`sa_old`) para restaurar.

## 9.3 Async-signal-safety

El handler puede interrumpir **cualquier** punto del main. Si el main tenía un lock interno de `malloc`, `printf` no es segura.

Lista mínima segura para el curso: asignaciones a `sig_atomic_t`, `write`, `_exit`, syscalls async-safe documentadas.

## 9.4 SIGCHLD y zombies

Sin `wait`, el hijo terminado permanece **Z** hasta que el padre recolecte. SIGCHLD es la pista de que conviene llamar `waitpid`. El polling de V3 también funciona pero no escala conceptualmente a pools grandes (Semana 11).

## 9.5 SIGPIPE

Si el cliente cierra y el servidor hace `write`, el kernel puede enviar SIGPIPE. Opciones:

- Ignorar SIGPIPE (`SIG_IGN`) y manejar error en `write` (`EPIPE`).
- Default: proceso muere — inaceptable en servidor.

MiniServ V4/V3: al detectar `write < 0` con `EPIPE`, cerrar y terminar hijo; en padre considerar `SIG_IGN` para SIGPIPE (mencionar en milestone como recomendado).

## 9.6 EINTR

Interrupción de syscalls bloqueantes es comportamiento POSIX clásico. Servidores deben ser **EINTR-aware** o usar `SA_RESTART` conscientemente.

---

# 10. Diagramas

### Entrega de señal (simplificado)

```text
                    ┌─────────────────┐
                    │   Proceso       │
                    │   (user)        │
                    └────────┬────────┘
                             │ syscall bloqueante (accept)
                             ▼
                    ┌─────────────────┐
                    │     Kernel      │
                    │  hijo exit ─────┼──► marca SIGCHLD pending
                    └────────┬────────┘
                             │ retorno a user (EINTR o restart)
                             ▼
                    handler SIGCHLD o main loop
```

**Explicación:** el hijo que muere no “llama” al padre; el kernel notifica al padre.

### MiniServ V4 — señales en el padre

```text
                 ┌─────────────────────────┐
                 │      Padre MiniServ      │
                 │  sigaction:              │
                 │   SIGINT  → g_stop       │
                 │   SIGTERM → g_stop       │
                 │   SIGCHLD → waitpid loop │
                 └───────────┬─────────────┘
                             │ fork
              ┌──────────────┼──────────────┐
              ▼              ▼              ▼
           Hijo 1         Hijo 2         Hijo 3
         serve client   serve client   serve client
              │              │              │
             _exit           _exit           _exit
              └──────────────┴──────────────┘
                             │
                        SIGCHLD × N
```

---

# 11. Demostraciones en C

Los fuentes están en `proyecto/semana4/programas_ejemplo/`.

---

## Demo 1 — `demo_sigint_default.c`

### Objetivo

Mostrar acción por defecto de SIGINT (terminación).

### Compilación

```bash
gcc -Wall -Wextra demo_sigint_default.c -o demo_sigint_default
```

### Ejecución

```bash
./demo_sigint_default
# Ctrl+C
```

### Salida esperada

Proceso termina; shell muestra prompt; opcional `^C`.

### No determinismo

N/A.

### Explicación paso a paso

1. `pause()` bloquea hasta señal → syscall en kernel.
2. Ctrl+C → kernel envía SIGINT al foreground process group.
3. Sin handler → default Term → proceso muere.

---

## Demo 2 — `demo_sigaction_flag.c`

### Objetivo

Patrón `volatile sig_atomic_t` + salida ordenada.

### Compilación / ejecución

```bash
gcc -Wall -Wextra demo_sigaction_flag.c -o demo_sigaction_flag
./demo_sigaction_flag
```

### Salida esperada

Puntos cada segundo; tras Ctrl+C, mensaje de limpieza y exit 0.

### No determinismo

Timing de puntos.

### Explicación paso a paso

1. `sigaction` registra handler en kernel.
2. SIGINT → handler corre → `g_stop = 1`.
3. Main observa flag en loop cooperativo → cleanup → `return`.

---

## Demo 3 — `demo_sigchld_reap.c`

### Objetivo

Recolección vía SIGCHLD (no busy-wait en zombies).

### Compilación / ejecución

```bash
gcc -Wall -Wextra demo_sigchld_reap.c -o demo_sigchld_reap
./demo_sigchld_reap
```

### Salida esperada

Varios hijos lanzados; `ps` no muestra Z persistentes tras handler.

### No determinismo

PIDs, timing entre líneas.

### Explicación paso a paso

1. Padre instala handler SIGCHLD.
2. Cada `fork` + `_exit` en hijo → kernel envía SIGCHLD.
3. Handler drena con `waitpid(-1, WNOHANG)` en bucle.

---

## Demo 4 — `demo_eintr_accept.c`

### Objetivo

`accept` retorna `EINTR` cuando llega señal sin `SA_RESTART`.

### Compilación / ejecución

```bash
gcc -Wall -Wextra demo_eintr_accept.c -o demo_eintr_accept
./demo_eintr_accept
# otra terminal: kill -INT <pid>
```

### Salida esperada

Mensaje `accept: EINTR`, reintentos hasta `g_stop`, cierre ordenado.

### No determinismo

Momento exacto de EINTR.

---

# 12. Herramientas Linux

### `kill -l`

**Observar:** nombres y números de señales.
**Relación:** vocabulario para `strace` y debugging.
**Alumno identifica:** SIGINT=2, SIGTERM=15, SIGCHLD=17 (Linux x86_64).

### `strace -e trace=signal,accept,wait4`

**Observar:** `rt_sigaction`, `--- SIGCHLD ... ---`, secuencia `wait4`.
**Relación:** confirma handler vs. syscall de recolección.

### `ps -o pid,ppid,stat,cmd`

Tras stress de clientes + apagado ordenado: no debe quedar `<defunct>` bajo el padre.

### `/proc/<pid>/status`

Campos `SigQ`, `SigPnd`, `ShdPnd` (opcional avanzado): cola de señales.

### `cat /proc/<pid>/fd`

Al apagar: `listen_fd` debe desaparecer; hijos no deben mantener fds huérfanos innecesarios.

---

# 13. Integración con MiniServ

### Estado inicial

MiniServ **V3** (`miniserv/v3/`): fork-per-client, cierre correcto de fds, `reap_children()` con `WNOHANG` en el loop de `accept`.

### Problema

1. Apagado no controlado (SIGINT default).
2. Recolección solo cuando el padre vuelve al loop (modelo incompleto para servidores reales).
3. Syscalls bloqueantes sin política clara ante `EINTR`.

### Mecanismo del Sistema Operativo

Señales + `sigaction` + semántica de `waitpid` en handler SIGCHLD.

### Cambio arquitectónico

```text
MiniServ V3 (polling waitpid en loop)
      │
      ▼
Ctrl+C mata bruscamente; modelo reactivo incompleto
      │
      ▼
sigaction: SIGINT, SIGTERM, SIGCHLD
      │
      ▼
MiniServ V4 en miniserv/v4/
```

---

# 14. Milestone semanal — MiniServ V4

## Requisitos funcionales

- Mismo echo TCP puerto 8080 que V3.
- Ante **SIGINT** o **SIGTERM** en el padre: dejar de aceptar nuevas conexiones, terminar hijos vivos de forma ordenada (esperar su `_exit`), cerrar recursos, salir con código 0.
- Recolección de hijos mediante handler **SIGCHLD** (no depender solo del polling).

## Requisitos técnicos

- `sigaction()` para SIGINT, SIGTERM y SIGCHLD (no `signal()`).
- Flag global `volatile sig_atomic_t` para apagado solicitado.
- Handler SIGCHLD llama a función reaper con `waitpid(-1, &status, WNOHANG)` en bucle; preservar `errno`.
- Loop de `accept` maneja `EINTR` (revisar flag de apagado).
- Recomendado: `sigaction(SIGPIPE, SIG_IGN, NULL)` o manejo explícito en hijo.
- Compilar con `-Wall -Wextra`.

## Restricciones

- **No** threads.
- **No** IPC/logger (Semana 7).
- **No** prefork pool (Semana 11).
- Handlers **no** deben usar `printf`/`malloc`; logging de apagado en el main.

## Criterios de aceptación

```text
[ ] Código en miniserv/v4/
[ ] make sin warnings
[ ] Dos nc simultáneos siguen funcionando como V3
[ ] kill -TERM <pid_padre> → apagado ordenado (mensaje en stderr)
[ ] Ctrl+C en foreground → mismo comportamiento ordenado
[ ] Tras apagado, ps no muestra hijos miniserv colgados ni zombies del padre
[ ] strace muestra rt_sigaction para SIGCHLD y wait4 en handler o tras SIGCHLD
[ ] accept reinicia correctamente tras EINTR si se prueba con señal durante accept
```

## Pruebas

```bash
cd miniserv/v4 && make && ./miniserv

# Terminal 2–3: clientes
nc 127.0.0.1 8080

# Apagado
kill -TERM $(pgrep -n -f 'v4/miniserv')
# o Ctrl+C si corre en foreground

# Zombies bajo carga rápida
for i in $(seq 1 50); do nc 127.0.0.1 8080 </dev/null & done
wait
ps -o pid,stat,cmd --ppid $(pgrep -n -f 'v4/miniserv')

# Señales
strace -e trace=signal,accept,wait4 -f ./miniserv 2>&1 | head -80
```

---

# 15. Actividades de clase

### Pregunta 1

**¿Por qué no debemos llamar `printf` dentro de un signal handler?**

**Respuesta:** no es async-signal-safe; si el main fue interrumpido dentro de `printf`/`malloc`, el handler puede deadlock o corromper estado interno de libc.

**Razonamiento:** el handler es un contexto de ejecución especial, no un callback normal.

### Pregunta 2

**¿SIGCHLD reemplaza por completo a `waitpid`?**

**Respuesta:** no; SIGCHLD **avisa**; `waitpid` **recolecta** y elimina el zombie.

**Razonamiento:** la señal no transporta el exit status al user space sin `wait`.

### Pregunta 3

**¿Por qué `kill -9` no permite cierre ordenado?**

**Respuesta:** SIGKILL no se puede capturar ni ignorar; el kernel termina el proceso sin ejecutar handlers.

**Razonamiento:** útil para emergencias, no para apagado normal de servicios.

### Ejercicio 1 (15 min)

En `demo_sigaction_flag.c`, registrar también SIGTERM con el mismo handler y probar `kill -TERM`.

### Ejercicio 2 (20 min)

Esqueleto MiniServ: escribir `setup_signals()` y probar que `accept` retorna `EINTR` al recibir SIGINT sin `SA_RESTART`.

### Predicción

**Código:**

```c
volatile sig_atomic_t flag = 0;
void h(int s) { (void)s; flag = 1; printf("H\n"); }
/* sigaction(SIGINT, h, ...); */
for (;;) {
    if (flag) break;
    sleep(1);
}
```

**¿Es seguro?** No — `printf` en handler. **¿Imprime H antes de salir?** Probablemente sí, pero puede colgar en otros escenarios. **Determinista:** no.

---

# 16. Errores y misconceptions

### "Pongo `wait` dentro del handler SIGCHLD y listo, no necesito loop"

Incorrecto si varios hijos terminan juntos — un solo `waitpid` puede dejar zombies; usar **while** `WNOHANG`.

### "`signal()` y `sigaction()` son lo mismo"

En código portable/moderno usar **`sigaction`**: semántica definida, flags, máscara.

### "El hijo envía SIGCHLD al padre"

Lo genera el **kernel** cuando el hijo cambia de estado (exit).

### "SA_RESTART siempre es mejor"

Oculta puntos donde debías revisar `g_stop`; en servidores pedagógicos a menudo conviene **EINTR explícito**.

### "Ignorar SIGPIPE es trampa"

Es práctica estándar en servidores de red; alternativa es manejar `EPIPE` en cada `write`.

---

# 17. Conexión profesional

- **systemd:** envía SIGTERM al parar un servicio; timeout luego SIGKILL — mismo contrato que practicamos.
- **nginx/apache:** master procesa señales para reload (`SIGHUP`) y apagado; workers distintos del modelo fork-per-client pero la **separación master/señales** es la misma idea.
- **Docker `docker stop`:** SIGTERM al PID 1 del contenedor; por eso los servidores deben capturarlo.
- **PostgreSQL:** postmaster maneja señales para shutdown inteligente de backends.

Arquitectura común:

```text
proceso supervisor (señales, wait, reinicio)
        │
        ├── workers / hijos
        └── recursos del kernel (sockets, fds)
```

---

# 18. Historia y fun facts

1. Las señales aparecen en **Unix temprano** (V1): interrupciones software inspiradas en interrupciones de hardware.
2. **SIGINT** atada a control de terminal — por eso Ctrl+C mata el foreground job.
3. La lista async-signal-safe existe porque **stdio** no fue diseñada para reentrada desde handlers.
4. Linux implementa señales con rt signals (`rt_sigaction`) — la API POSIX vista en clase es la capa portable.

---

# 19. Tarea semanal

## Objetivo

Entregar MiniServ V4 y reporte corto sobre señales.

## Entregables

```text
miniserv/v4/
reporte-semana5.md
```

## Preguntas del reporte

1. Captura de `strace` mostrando `SIGCHLD` y `wait4` al cerrar un cliente.
2. ¿Qué ocurre si comentas el handler SIGCHLD pero mantienes polling?
3. Describe la secuencia exacta al recibir SIGTERM (fds cerrados, waits).
4. ¿Viste `EINTR` en `accept`? ¿Cómo lo manejaste?

## Empaquetado

```bash
tar czf miniserv-semana5.tar.gz miniserv/v4 reporte-semana5.md
```

---

# 20. Cierre de la semana

## Conceptos fundamentales

1. Señal = notificación asíncrona del kernel (o de otro proceso vía kernel).
2. `sigaction` registra handlers con semántica POSIX clara.
3. Handlers mínimos: `sig_atomic_t`, async-signal-safe, trabajo real en main.
4. SIGINT/SIGTERM → apagado ordenado del master MiniServ.
5. SIGCHLD → pista para `waitpid` y evitar zombies.
6. EINTR → reintentar o salir según flag de apagado.
7. SIGPIPE / EPIPE → cliente desconectado antes de escribir.

## Mapa conceptual

```text
apagado caótico / polling-only reaper
        │
        ▼
señales (SIGINT, SIGTERM, SIGCHLD)
        │
        ▼
sigaction + waitpid en handler
        │
        ▼
MiniServ V4
```

## Qué cambia en la siguiente semana

**Semana 6 — Scheduling:** con V3/V4 el servidor crea **muchos procesos** bajo carga; el kernel alterna CPU entre padre (accept) e hijos (I/O). Mediremos estados `R/S`, context switches y compararemos secuencial vs. fork-per-client con herramientas como `time`, `ps` y `/proc`.

**Pregunta puente:**

> Si tenemos veinte hijos compitiendo por CPU mientras el padre hace `accept`, ¿quién decide quién corre y por cuánto tiempo?

---

## Apéndice para el profesor

### Orden sugerido de demos

1. `demo_sigint_default.c`
2. `demo_sigaction_flag.c`
3. `demo_sigchld_reap.c`
4. `demo_eintr_accept.c`
5. MiniServ V4 (esqueleto → completo)

### Esqueleto `setup_signals` (pizarrón)

```c
static volatile sig_atomic_t g_stop = 0;

static void on_stop(int sig) { (void)sig; g_stop = 1; }

static void on_chld(int sig)
{
    int saved = errno;
    (void)sig;
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;
    errno = saved;
}

static void setup_signals(void)
{
    struct sigaction sa;

    sa.sa_handler = on_stop;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    sa.sa_handler = on_chld;
    sa.sa_flags = SA_RESTART; /* opcional: debate en clase */
    sigaction(SIGCHLD, &sa, NULL);

    sigaction(SIGPIPE, &(struct sigaction){ .sa_handler = SIG_IGN }, NULL);
}
```

### Diff pedagógico V3 → V4

- Extraer `reap_children()` compartida entre loop y SIGCHLD.
- Condición de salida del `for (;;)` cuando `g_stop`.
- Tras el loop: `close(listen_fd)` + barrido final `waitpid` bloqueante.
