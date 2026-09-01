# Semana 1 — ¿Qué hace realmente un Sistema Operativo?

**Curso:** Sistemas Operativos (16 semanas)  
**Proyecto:** MiniServ — servidor TCP/HTTP multiproceso en C sobre Linux  
**Duración:** 2 sesiones × 90 minutos  
**Pregunta central de la semana:**

> Cuando nuestro programa en C ejecuta `write(fd, buffer, size);`, ¿quién escribe realmente los datos?

---

## A. Visión general de la semana

### Objetivos de aprendizaje

Al finalizar la semana, el estudiante podrá:

1. **Explicar** por qué existe un sistema operativo como intermediario entre programas y hardware.
2. **Identificar** la frontera entre espacio de usuario (*user space*) y espacio de kernel (*kernel space*), y qué implica cruzarla.
3. **Describir** qué es un proceso como abstracción del sistema operativo (programa en ejecución con identidad y recursos).
4. **Diferenciar** entre una función de biblioteca (p. ej. `printf`), una API POSIX (p. ej. `write`) y una *system call* real.
5. **Analizar** con `strace` qué llamadas al sistema realiza un programa aparentemente simple.
6. **Implementar** la estructura inicial del proyecto MiniServ y compilarlo con `gcc -Wall -Wextra`.
7. **Diagnosticar** errores básicos de compilación y ejecución en un entorno Linux.

### Motivación

Imaginemos que ya escribimos MiniServ y queremos que, al arrancar, imprima un mensaje en la terminal:

```c
printf("MiniServ iniciado\n");
```

Parece trivial: el programa "escribe" en pantalla. Pero si observamos con `strace`, descubrimos que `printf` no toca el hardware directamente. Internamente termina invocando `write(1, "MiniServ iniciado\n", 18)` — escribir en el *file descriptor* 1 (*stdout*) — y `write` a su vez ejecuta una *system call* que entrega el control al **kernel**.

**¿Por qué importa para Sistemas Operativos?**

Porque casi todo lo que haremos en MiniServ — leer peticiones, escribir respuestas, abrir archivos, crear procesos, manejar señales — pasa por esa frontera usuario/kernel. Si no entendemos quién hace qué, depurar el servidor será adivinar en la oscuridad.

**Manifestación en MiniServ (esta semana):**

MiniServ aún no es un servidor de red. Solo es un proceso que imprime `"MiniServ iniciado"`. Pero ese simple mensaje ya nos obliga a preguntarnos: ¿qué proceso es? ¿qué syscall ejecutó? ¿qué recurso del kernel utilizó?

**Mecanismo del SO que estudiaremos:**

La arquitectura básica de Linux: procesos en modo usuario, kernel en modo privilegiado, y las *system calls* como única puerta controlada hacia recursos del sistema.

### Conexión con semanas anteriores

Esta es la primera semana; no hay contenido previo del curso. Sí partimos de prerrequisitos que los estudiantes ya traen:

```text
C básico + gcc + terminal Linux
           │
           ▼
¿Cómo se ejecuta realmente nuestro código?
           │
           ▼
Arquitectura del SO: kernel, procesos, system calls
           │
           ▼
MiniServ V0: estructura del proyecto + "MiniServ iniciado"
           │
           ▼
(Semana 2) file descriptors, read/write, sockets → servidor TCP
```

La continuidad hacia adelante: esta semana establece el **modelo mental** que usaremos las 15 semanas restantes. Cada nuevo mecanismo (fork, señales, IPC, memoria compartida) será otra forma de cruzar o compartir recursos a través del kernel.

---

## Clase 1 — 90 minutos

| Tiempo     | Actividad                          |
| ---------- | ---------------------------------- |
| 0–15 min   | Problema motivador                 |
| 15–35 min  | Modo usuario vs. modo kernel       |
| 35–50 min  | Qué es un proceso                  |
| 50–70 min  | System calls                       |
| 70–80 min  | Demostración con `strace`          |
| 80–90 min  | Preguntas y discusión              |

### Bloque 0–15 min — Problema motivador

**Qué explicar el profesor:**

- Presentar MiniServ como proyecto del semestre: un servidor que evolucionará semana a semana.
- Lanzar la pregunta central: si ejecutamos `write(fd, buf, n)`, ¿quién escribe realmente?
- Mostrar que el programa **no** accede al disco, la red ni la pantalla directamente.

**Ejemplo en pizarrón:**

```text
Programa C          ←── solo ve variables, funciones, memoria "suya"
     │
     ?  ←── ¿qué hay aquí?
     ▼
Hardware (CPU, RAM, disco, red)
```

**Preguntas a los estudiantes:**

- ¿Alguna vez un programa colgó toda la máquina? ¿Por qué hoy eso es raro?
- Si dos programas corren "al mismo tiempo", ¿quién reparte el CPU?
- ¿Puede un programa leer la memoria de otro sin permiso?

**Conexión con MiniServ:**

> "Antes de que MiniServ escuche en un puerto, necesitamos entender qué es un proceso y cómo pide servicios al SO."

---

### Bloque 15–35 min — Modo usuario vs. modo kernel

**Qué explicar:**

1. **Hardware vs. software:** CPU, RAM, dispositivos vs. programas y SO.
2. **Kernel:** núcleo del SO; gestiona recursos, protege el sistema, implementa abstracciones.
3. **Modo usuario (*user mode*):** instrucciones restringidas; no acceso directo a hardware.
4. **Modo kernel (*kernel mode* / privilegiado):** acceso completo; drivers, tablas del SO, hardware.
5. **Transición:** interrupciones, excepciones, *traps*, *system calls*.

**Diagrama en pizarrón:** (ver Sección 10, Diagrama 1)

**Ejemplos:**

- Un `while(1)` en user space no tumba el sistema; el scheduler puede quitarle el CPU.
- Un bug en un driver de kernel sí puede provocar *kernel panic*.

**Preguntas:**

- ¿Por qué no dejamos que cada programa maneje su propio disco?
- ¿Qué ventajas tiene centralizar el acceso en el kernel?

**Conexión MiniServ:**

> MiniServ correrá en modo usuario. Cuando más adelante llame a `accept()` para recibir clientes, estará pidiendo al kernel que le entregue una conexión de red.

---

### Bloque 35–50 min — Qué es un proceso

**Qué explicar:**

1. **Programa vs. proceso:** el binario en disco vs. la instancia en ejecución.
2. **PID:** identificador único asignado por el kernel.
3. **Recursos asociados:** memoria, file descriptors, contexto de CPU, credenciales.
4. **Aislamiento:** cada proceso cree tener su propia memoria (simplificación válida por ahora).
5. **Múltiples procesos:** shell, editor, navegador — todos procesos; el kernel los multiplexa.

**Ejemplo:** ejecutar `./miniserv` crea un proceso con un PID; `ps` lo muestra.

**Preguntas:**

- ¿Un mismo programa ejecutado dos veces es uno o dos procesos?
- ¿Quién asigna el PID?

**Conexión MiniServ:**

> `"MiniServ iniciado"` lo imprime **un proceso**. En semanas futuras habrá un proceso master y varios workers; por ahora hay uno solo.

---

### Bloque 50–70 min — System calls

**Qué explicar:**

1. **Definición:** interfaz controlada para que procesos en user space soliciten servicios al kernel.
2. **Cadena típica:** `printf` → `write` (libc) → syscall `write` → kernel → driver/dispositivo.
3. **API POSIX vs. syscall:** POSIX estandariza nombres y semántica; Linux implementa con syscalls concretas.
4. **Syscalls que veremos pronto:** `read`, `write`, `open`, `close` (Semana 2); hoy las reconocemos con `strace`.
5. **Costo:** cruzar user/kernel tiene overhead; por eso no se hace gratuitamente en cada operación trivial.

**Diagrama en pizarrón:** (ver Sección 10, Diagrama 2)

**Ejemplo de código en pantalla:**

```c
#include <unistd.h>

write(1, "hola\n", 5);
```

Explicar: `1` es *stdout*; la syscall transporta bytes al kernel.

**Preguntas:**

- ¿`printf` es una system call?
- ¿Qué pasa si `write` recibe un descriptor inválido?

**Conexión MiniServ:**

> Cada línea de log, cada byte enviado a un cliente, cada archivo leído pasará por syscalls. `strace` será nuestra "ventana" al kernel.

---

### Bloque 70–80 min — Demostración con `strace`

**Actividad en vivo:**

```bash
strace -e trace=write ./miniserv
```

o, si aún no existe el binario:

```bash
strace -e trace=write echo "MiniServ iniciado"
```

**Qué observar:**

- Línea `write(1, "...\n", N) = N`
- El valor de retorno (= bytes escritos)

Segundo ejemplo:

```bash
strace -e trace=write,openat,close ls
```

**Preguntas durante la demo:**

- ¿Cuántas syscalls hace `ls` para listar un directorio pequeño?
- ¿Aparece `write` aunque no abrimos archivos nosotros?

**Conexión MiniServ:**

> Repetir: "MiniServ parece hacer una sola cosa; el kernel hace varias por detrás."

---

### Bloque 80–90 min — Preguntas y discusión

**Cierre:**

- Recapitular: hardware → kernel → procesos → syscalls → programas.
- Adelantar Semana 2: file descriptors y por qué `write(1, ...)` usa el número 1.
- Recordar milestone: crear estructura del proyecto antes de la Clase 2.

**Pregunta de salida:**

> Nombren tres cosas que un programa en user space **no** puede hacer directamente y debe pedir al kernel.

**Respuesta esperada (ejemplos):** acceder a disco crudo, crear procesos, configurar interfaz de red, modificar tablas de memoria de otro proceso, deshabilitar interrupciones.

---

## Clase 2 — 90 minutos (Laboratorio)

| Tiempo     | Actividad                                      |
| ---------- | ---------------------------------------------- |
| 0–10 min   | Repaso rápido + objetivos del laboratorio      |
| 10–25 min  | `strace ls` — lectura guiada de salida         |
| 25–45 min  | `strace cat archivo.txt` — open/read/write     |
| 45–60 min  | Crear estructura MiniServ + Makefile           |
| 60–75 min  | Compilar, ejecutar, observar con `strace`      |
| 75–85 min  | Ejercicios cortos (ver Sección 15)             |
| 85–90 min  | Entrega del milestone, dudas, cierre           |

### Bloque 0–10 min — Repaso

- Dibujar de memoria la pila: aplicación → libc → syscall → kernel.
- Verificar que todos tienen Linux, `gcc`, `strace` (`sudo apt install strace` si falta).

### Bloque 10–25 min — `strace ls`

**Comando:**

```bash
strace ls -l /tmp 2>&1 | head -40
```

**Qué identificar:**

| Syscall      | Propósito observado                          |
| ------------ | -------------------------------------------- |
| `openat`     | Abrir directorio o archivo                   |
| `getdents64` | Leer entradas de directorio (kernel)         |
| `write`      | Escribir nombres a stdout (fd 1)             |
| `close`      | Liberar descriptor                           |
| `mmap`       | Mapear memoria (libc/kernel optimizaciones)  |

**Actividad:** los estudiantes marcan en papel cuántas veces aparece `write`.

### Bloque 25–45 min — `strace cat archivo.txt`

**Preparación:**

```bash
echo "linea de prueba para SO" > /tmp/so-demo.txt
strace cat /tmp/so-demo.txt
```

**Secuencia esperada (simplificada):**

```text
openat(..., "/tmp/so-demo.txt", O_RDONLY) = 3
read(3, "linea de prueba...\n", 131072) = 23
write(1, "linea de prueba...\n", 23)    = 23
read(3, "", 131072)                     = 0   ← EOF
close(3)                                = 0
```

**Discusión:**

- ¿Por qué `read` puede pedir muchos bytes pero devolver menos?
- ¿Por qué hay dos `read`?
- Relación con MiniServ: leer petición / escribir respuesta será este mismo patrón.

### Bloque 45–60 min — Estructura del proyecto

**Crear árbol:**

```text
miniserv/
├── Makefile
├── src/
│   └── server.c
├── include/
└── README.md
```

**Makefile mínimo sugerido:**

```makefile
CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11
TARGET  = miniserv

$(TARGET): src/server.c
	$(CC) $(CFLAGS) src/server.c -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: clean
```

**README.md:** nombre del proyecto, autor, cómo compilar y ejecutar.

### Bloque 60–75 min — Compilar y observar

```bash
make
./miniserv
strace -e trace=write ./miniserv
```

**Verificar:** salida `MiniServ iniciado` y al menos una syscall `write`.

### Bloque 75–85 min — Ejercicios en parejas

Ver Sección 15 (ejercicios cortos y predicción).

### Bloque 85–90 min — Cierre

- Checklist del milestone (Sección 14).
- Recordar tarea semanal (Sección 19).

---

## Explicación teórica

### 1. Hardware vs. software

**Qué es:** El hardware (CPU, RAM, almacenamiento, red, periféricos) ejecuta y almacena bits. El software (programas, bibliotecas, kernel) son instrucciones y datos que organizan ese hardware.

**Por qué existe la distinción:** Los cursos de SO estudian cómo el software — especialmente el kernel — hace usable y seguro el hardware compartido entre muchos programas.

**Problema que resuelve:** Sin capas de software, cada aplicación necesitaría drivers propios y acceso exclusivo a la máquina.

**Cómo funciona:** El kernel contiene drivers y políticas; las aplicaciones solicitan servicios abstractos ("lee este archivo") en lugar de señales eléctricas al disco.

**Rol del kernel:** Traducir solicitudes abstractas en operaciones concretas sobre hardware, con permisos y scheduling.

**Relación con MiniServ:** MiniServ es software en user space; cuando sirva archivos, el kernel traducirá `read()` en lecturas del almacenamiento.

**Errores frecuentes:** Creer que "el programa escribe al disco" sin pasar por el kernel; confundir el binario en disco con el proceso en memoria.

---

### 2. Kernel y arquitectura básica de Linux

**Qué es:** El kernel es el núcleo del sistema operativo: permanece residente en memoria, ejecuta en modo privilegiado y gestiona procesos, memoria, filesystem, red, dispositivos.

**Por qué existe:** Centralizar control evita caos (dos programas escribiendo al mismo sector) y permite abstracciones uniformes.

**Problema que resuelve:** Multiplexar hardware entre procesos, aislar fallos, aplicar permisos.

**Cómo funciona (simplificado):**

```text
┌─────────────────────────────────────┐
│  Procesos en user space             │
│  (shell, MiniServ, editor, ...)     │
├─────────────────────────────────────┤
│  System call interface              │
├─────────────────────────────────────┤
│  Kernel Linux                       │
│  scheduler │ VM │ VFS │ network ... │
├─────────────────────────────────────┤
│  Hardware                           │
└─────────────────────────────────────┘
```

**Rol del kernel:** Árbitro único de recursos; implementa syscalls.

**Relación con MiniServ:** Todo lo que MiniServ "hace" con el mundo exterior lo hace **pidiendo** al kernel.

**Errores frecuentes:** Pensar que Linux "es la terminal" o "es el escritorio"; la GUI también es user space sobre el mismo kernel.

---

### 3. Espacio de usuario y espacio de kernel

**Qué es:** Separación de memoria y privilegios de instrucción. User space: código de aplicaciones. Kernel space: código del kernel.

**Por qué existe:** Protección y estabilidad — un error en MiniServ no debe corromper estructuras internas del kernel.

**Problema que resuelve:** Impedir acceso arbitrario a memoria física, registros del CPU o dispositivos.

**Cómo funciona:** El hardware (MMU, rings de privilegio en x86-64: ring 3 = user, ring 0 = kernel) refuerza la frontera. Solo en syscall/interrupción el CPU eleva privilegio.

**Rol del kernel:** Valida cada solicitud en la frontera.

**Relación con MiniServ:** MiniServ nunca ejecuta código en kernel space; solo invoca syscalls.

**Errores frecuentes:** Creer que `malloc` está en el kernel (está en libc, en user space); confundir "biblioteca del sistema" con "kernel".

---

### 4. Modo usuario vs. modo privilegiado

**Qué es:** Dos modos de ejecución del CPU. Modo usuario: restricciones. Modo kernel: sin restricciones de protección normales.

**Por qué existe:** Evitar que aplicaciones maliciosas o buggy manipulen el sistema.

**Problema que resuelve:** Seguridad y confiabilidad multitarea.

**Cómo funciona:** Instrucciones sensibles (p. ej. modificar tablas de páginas ciertas) solo en modo kernel. Syscall = trap controlado a código del kernel.

**Rol del kernel:** Ejecutar la syscall, retornar resultado o error al proceso.

**Relación con MiniServ:** Cada `write()` en MiniServ implica un cambio temporal a modo kernel.

**Errores frecuentes:** Asumir que syscalls "son gratis"; tienen costo de cambio de contexto user/kernel.

---

### 5. Interrupciones, excepciones y traps

**Qué es:**

- **Interrupción:** evento externo asíncrono (tecla, temporizador del timer).
- **Excepción:** evento síncrono ligado a una instrucción ( división por cero, page fault ).
- **Trap:** excepción intencional (syscall).

**Por qué existen:** El CPU debe atender hardware y permitir transición controlada al kernel.

**Problema que resuelve:** Multitarea (timer interrupt → scheduler), protección (page fault), interfaz syscall.

**Cómo funciona:** Hardware guarda PC y flags, salta a rutina del kernel, kernel procesa y retorna.

**Rol del kernel:** Manejadores de interrupción, tabla de syscalls, devolución a user space.

**Relación con MiniServ:** Cuando MiniServ espera datos de red (semanas futuras), el proceso estará bloqueado hasta una interrupción del dispositivo de red.

**Errores frecuentes:** Usar "interrupción" y "señal" como sinónimos (las señales son abstracción del SO en user space; relacionadas pero distintas).

---

### 6. Procesos

**Qué es:** Abstracción de "programa en ejecución" con identidad (PID), espacio de memoria virtual, contexto de CPU, tabla de file descriptors, credenciales.

**Por qué existe:** El SO necesita una unidad de contabilidad, aislamiento y scheduling.

**Problema que resuelve:** Ejecutar muchos programas "simultáneamente" en CPU(s) limitada(s).

**Cómo funciona:** El kernel mantiene `task_struct` (Linux) por proceso; scheduler elige cuál corre; context switch cambia registros y MMU.

**Rol del kernel:** Crear (`fork`/`exec` futuro), destruir (`exit`/`wait`), planificar, contabilizar recursos.

**Relación con MiniServ:** V0 = un proceso. V3+ = varios procesos cooperando.

**Errores frecuentes:** Confundir thread con proceso (no usamos threads en este curso inicialmente); creer que PID se recicla instantáneamente sin reglas (sí se reutilizan, pero con cuidado del kernel).

---

### 7. System calls

**Qué es:** Punto de entrada numerado desde user space al kernel (en x86-64 Linux vía `syscall` o `int 0x80` legacy). Ejemplos: `read`, `write`, `open`, `close`, `getpid`.

**Por qué existen:** Única API controlada para servicios privilegiados.

**Problema que resuelve:** Servicios uniformes y seguros (mismo `open` para archivos, sockets en otro nivel).

**Cómo funciona:**

1. Programa llama wrapper en libc (`write`).
2. Libc coloca número de syscall y argumentos en registros.
3. Instrucción `syscall` → CPU entra en kernel.
4. Kernel valida, ejecuta, copia datos user/kernel si aplica.
5. Retorna código de error o valor positivo.

**Rol del kernel:** Implementación real; permisos; bloqueo si no hay datos.

**Relación con MiniServ:** Toda la I/O del servidor son syscalls.

**Errores frecuentes:** Creer que `fopen` es syscall (`fopen` es stdio; usa `open` por debajo); ignorar valores de retorno (-1 + `errno`).

---

### 8. API POSIX e introducción a `read`, `write`, `open`, `close`

**Qué es:** POSIX estandariza interfaces Unix-like. `unistd.h` declara I/O de bajo nivel.

**Por qué existe:** Portabilidad y semántica clara entre sistemas Unix-like.

**Problema que resuelve:** Programas que compilan en Linux y otros Unix con mínimos cambios.

**Cómo funciona (vista previa Semana 2):**

- `open(path, flags)` → devuelve file descriptor (entero ≥ 0).
- `read(fd, buf, count)` → lee hasta `count` bytes.
- `write(fd, buf, count)` → escribe hasta `count` bytes.
- `close(fd)` → libera descriptor.

**Rol del kernel:** Objetos reales (archivo abierto, socket) viven en kernel; FD es índice en tabla del proceso.

**Relación con MiniServ:** Patrón universal read/write en servidor.

**Errores frecuentes:** No incluir `<unistd.h>`; mezclar `stdio` (`fread`) con syscalls sin entender capas.

---

## Diagramas

### Diagrama 1 — Capas: aplicación, libc, kernel, hardware

```text
┌──────────────────────────────────────────┐
│  Aplicación (MiniServ, ls, cat)          │
│  printf(), lógica del servidor           │
└──────────────────┬───────────────────────┘
                   │ llamadas a libc
                   ▼
┌──────────────────────────────────────────┐
│  Biblioteca C (glibc)                    │
│  write(), read(), wrappers               │
└──────────────────┬───────────────────────┘
                   │ syscall (trap)
═══════════════════╪═══════════════════════  ← frontera user / kernel
                   ▼
┌──────────────────────────────────────────┐
│  Kernel Linux                            │
│  VFS, scheduler, drivers, red            │
└──────────────────┬───────────────────────┘
                   │
                   ▼
┌──────────────────────────────────────────┐
│  Hardware: CPU, RAM, disco, NIC, ...     │
└──────────────────────────────────────────┘
```

**Explicación:** Toda solicitud de recurso desciende hasta el kernel. MiniServ vive en la capa superior; nunca salta la frontera excepto vía syscall.

---

### Diagrama 2 — Flujo de `write(1, buf, n)`

```text
  MiniServ: write(1, "OK\n", 3)
        │
        ▼
  glibc prepara: nr=SYS_write, fd=1, buf, count=3
        │
        ▼
  CPU: instrucción SYSCALL  ──►  modo kernel
        │
        ▼
  sys_write() en kernel
    ├─ ¿fd=1 válido para este proceso?
    ├─ ¿permiso de escritura?
    └─ copiar bytes desde user space
        │
        ▼
  subsistema terminal (tty) / pipe / archivo
        │
        ▼
  retorno: 3 (bytes escritos)  ──►  user space
```

**Explicación:** El "3" de retorno no es el trabajo del hardware visible; es la confirmación del kernel de cuántos bytes aceptó.

---

### Diagrama 3 — Múltiples procesos y el kernel

```text
   Proceso A (bash)     Proceso B (MiniServ)    Proceso C (editor)
   PID 1200             PID 3500                PID 3501
        │                      │                        │
        └──────────────────────┼────────────────────────┘
                               │ syscalls
                               ▼
                    ┌─────────────────────┐
                    │   Kernel Linux      │
                    │   Scheduler         │
                    │   Tabla de procesos │
                    └──────────┬──────────┘
                               │
                               ▼
                         CPU / RAM / I/O
```

**Explicación:** Cada proceso parece ejecutarse solo; el kernel alterna y multiplexa. MiniServ V0 es un solo rectángulo; más adelante habrá varios.

---

### Diagrama 4 — Programa vs. proceso

```text
  Disco                          RAM (cuando ejecuta)
 ┌─────────────┐                ┌─────────────────────┐
 │ miniserv    │   exec/load    │ Proceso miniserv    │
 │ (binario)   │ ─────────────► │ PID=4521            │
 │ bytes ELF   │                │ PC, stack, heap, FDs  │
 └─────────────┘                └─────────────────────┘
     archivo                         instancia viva
```

**Explicación:** `./miniserv` transforma el archivo en proceso. `strace ./miniserv` observa al proceso, no al archivo en disco.

---

## Demostraciones en C

### Demo 1 — `write()` directo a stdout

#### Objetivo

Mostrar que podemos imprimir sin `printf`, usando la API POSIX `write` y el file descriptor 1.

#### Código completo

```c
#include <unistd.h>

int main(void)
{
    const char msg[] = "Hola desde write()\n";
    write(1, msg, sizeof(msg) - 1);
    return 0;
}
```

#### Compilación

```bash
gcc -Wall -Wextra demo_write.c -o demo_write
```

#### Ejecución

```bash
./demo_write
```

#### Salida esperada

```text
Hola desde write()
```

#### No determinismo

Ninguno relevante: salida determinista.

#### Explicación paso a paso

1. El programa corre en user space.
2. `write(1, ...)` llama al wrapper de glibc.
3. glibc ejecuta syscall `write`.
4. Kernel valida fd 1 (stdout del proceso) y copia bytes al terminal driver.
5. Retorna 20 (longitud del mensaje sin `\0`).
6. Observamos texto en pantalla — trabajo conjunto user + kernel.

---

### Demo 2 — `getpid()` y identidad del proceso

#### Objetivo

Introducir que el proceso tiene identidad asignada por el kernel.

#### Código completo

```c
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    printf("MiniServ demo: PID=%d\n", getpid());
    return 0;
}
```

#### Compilación

```bash
gcc -Wall -Wextra demo_pid.c -o demo_pid
```

#### Ejecución

```bash
./demo_pid
./demo_pid &
./demo_pid
```

#### Salida esperada

```text
MiniServ demo: PID=12345
MiniServ demo: PID=12346
MiniServ demo: PID=12347
```

(Los números varían.)

#### No determinismo

**Sí:** los PIDs cambian entre ejecuciones y dependen del estado del sistema.

#### Explicación paso a paso

1. Al ejecutar `./demo_pid`, el kernel crea un proceso nuevo (`execve` al cargar el binario).
2. Asigna un PID libre.
3. `getpid()` es syscall que devuelve ese PID al programa.
4. `printf` internamente usará `write` hacia stdout.
5. Ejecutar varias veces demuestra: mismo programa, distintos procesos.

---

### Demo 3 — `open`, `read`, `write`, `close`

#### Objetivo

Reproducir en C lo observado con `strace cat`: ciclo de lectura de archivo.

#### Código completo

```c
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    const char *path = "/tmp/so-demo.txt";
    char buf[128];
    ssize_t n;

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        if (write(1, buf, n) != n) {
            perror("write");
            close(fd);
            return 1;
        }
    }

    if (n < 0) {
        perror("read");
        close(fd);
        return 1;
    }

    if (close(fd) < 0) {
        perror("close");
        return 1;
    }

    return 0;
}
```

**Preparación:**

```bash
echo "linea de prueba para SO" > /tmp/so-demo.txt
```

#### Compilación

```bash
gcc -Wall -Wextra demo_cat.c -o demo_cat
```

#### Ejecución

```bash
./demo_cat
strace ./demo_cat
```

#### Salida esperada

```text
linea de prueba para SO
```

#### No determinismo

Ninguno en contenido; el fd numérico (3) suele ser estable si stdin/stdout/stderr son 0/1/2.

#### Explicación paso a paso

1. `open` → syscall; kernel abre archivo, devuelve FD.
2. `read` → syscall; copia bytes del archivo a `buf` en user space.
3. `write(1, ...)` → syscall; envía a stdout.
4. Segundo `read` devuelve 0 (EOF).
5. `close` → syscall; libera FD en tabla del proceso.
6. Mismo patrón que usará MiniServ para archivos estáticos (Semana 12).

---

### Demo 4 — MiniServ V0

#### Objetivo

Punto de partida del proyecto del curso.

#### Código completo

```c
#include <stdio.h>

int main(void)
{
    printf("MiniServ iniciado\n");
    return 0;
}
```

#### Compilación

```bash
gcc -Wall -Wextra src/server.c -o miniserv
```

(o `make` con el Makefile del milestone.)

#### Ejecución

```bash
./miniserv
strace -e trace=write ./miniserv
```

#### Salida esperada

```text
MiniServ iniciado
```

Ejemplo `strace` (fragmento):

```text
write(1, "MiniServ iniciado\n", 18) = 18
+++ exited with 0 +++
```

#### No determinismo

PID varía; la línea `write` es estable.

#### Explicación paso a paso

1. Shell ejecuta `execve` del binario → nuevo proceso.
2. `main` corre en user space.
3. `printf` formatea y llama `write` internamente.
4. Una syscall escribe 18 bytes.
5. `return 0` → syscall `exit_group`; proceso termina.

---

## Herramientas Linux

### `strace`

**Qué observar:** Cada línea `nombre(args...) = resultado` es una syscall.

**Significado:** Argumentos en sintaxis C; resultado positivo = éxito (a menudo bytes o FD); `-1` = error (ver `errno`).

**Relación con el concepto:** Ventana directa al límite user/kernel.

**Qué debe identificar el alumno:** `write`, `openat`, `read`, `close`, `execve`, `getpid`.

**Ejemplos:**

```bash
strace ./miniserv
strace -e trace=write,read,openat,close cat /etc/hostname
strace -c ls          # resumen por conteo — útil para "costo"
```

---

### `ps`

**Qué observar:** Lista procesos; columnas PID, CMD, STAT, TIME.

**Significado:** Cada fila es un proceso gestionado por el kernel.

**Relación:** MiniServ en ejecución aparece como `./miniserv`.

**Qué identificar el alumno:** PID propio vs. otros procesos.

```bash
ps aux | grep miniserv
```

---

### `/proc/<pid>/`

**Qué observar:**

```bash
cat /proc/self/status    # Name, State, PPid
ls -l /proc/self/fd      # file descriptors abiertos
```

**Significado:** Interfaz pseudo-filesystem que exporta estado del kernel sobre procesos.

**Relación:** Refuerza que el kernel mantiene metadata por proceso.

**Qué identificar:** `State: R (running)` o `S (sleeping)`; fd 0,1,2 en proceso típico.

---

### `time`

**Qué observar:**

```bash
time ./miniserv
```

**Significado:** Tiempo real, user, sys — `sys` incluye tiempo en kernel atendiendo syscalls.

**Relación:** Aunque MiniServ V0 es trivial, establece que el kernel consume tiempo medible.

---

### `gcc -Wall -Wextra`

**Qué observar:** Warnings de tipos, variables sin usar, declaraciones implícitas.

**Relación:** Buen hábito antes de programar syscalls donde errores de tipos causan bugs sutiles.

---

## Integración con MiniServ

### Estado inicial

No existe MiniServ al inicio de la semana. Al final de la semana:

```text
miniserv/
├── Makefile
├── src/server.c   → imprime "MiniServ iniciado"
├── include/       → vacío (reservado)
└── README.md
```

Un proceso, sin red, sin concurrencia.

### Problema

Aún no hay "problema de servidor" — el problema pedagógico es **epistemológico**: creemos que nuestro programa "hace todo" cuando en realidad **delega** en el SO.

### Mecanismo del Sistema Operativo

Procesos + system calls + separación user/kernel explican quién ejecuta realmente cada operación.

### Cambio arquitectónico

```text
MiniServ (idea en papel)
      │
      ▼
¿Quién imprime "MiniServ iniciado"?
      │
      ▼
Proceso + syscall write vía kernel
      │
      ▼
Milestone 0: binario compilable + estructura de proyecto
      │
      ▼
MiniServ V0 (proceso mínimo observable con strace)
```

### Transición hacia Semana 2

```text
MiniServ V0 solo imprime
      │
      ▼
Necesitamos escuchar conexiones de red
      │
      ▼
File descriptors + sockets + read/write bloqueante
      │
      ▼
MiniServ V1 — servidor TCP secuencial
```

---

## Milestone 0 — Estructura inicial del proyecto

### Requisitos funcionales

- Al ejecutar `./miniserv`, imprimir exactamente una línea: `MiniServ iniciado` (con salto de línea).
- El programa termina con código de salida 0.

### Requisitos técnicos

- Estructura de directorios según especificación.
- `Makefile` con target `miniserv` y `clean`.
- Compilación con `gcc -Wall -Wextra -std=c11`.
- `README.md` con instrucciones de compilación y ejecución.

### Restricciones

- No usar sockets, `fork`, threads ni señales.
- No es necesario parsear argumentos de línea de comandos todavía.

### Criterios de aceptación

```text
[ ] Estructura miniserv/{Makefile,src/server.c,include/,README.md}
[ ] Compila con make sin warnings (-Wall -Wextra)
[ ] ./miniserv imprime "MiniServ iniciado"
[ ] strace -e trace=write ./miniserv muestra write(1, ...)
[ ] make clean elimina el binario
```

### Pruebas

```bash
cd miniserv
make
./miniserv
echo $?                    # debe ser 0
strace -e trace=write ./miniserv 2>&1 | grep MiniServ
make clean
test ! -f miniserv && echo OK
```

---

## Actividades de clase

### Preguntas conceptuales

#### Pregunta 1

**P:** ¿`printf("hola")` es una system call? Explique la cadena completa hasta el kernel.

**Respuesta esperada:** No. `printf` es función de biblioteca estándar C. Formatea la cadena y eventualmente llama a `write` (u otra función de bajo nivel). `write` es wrapper POSIX que ejecuta la syscall `write`. El kernel es quien finalmente envía bytes al dispositivo asociado al fd 1.

**Razonamiento:** Distingue capas — confundir stdio con syscalls es error clásico de principiantes.

---

#### Pregunta 2

**P:** ¿Por qué un bug en MiniServ raramente tumba todo Linux, pero un bug en un driver de kernel puede hacerlo?

**Respuesta esperada:** MiniServ corre en modo usuario con permisos limitados; accesos inválidos a memoria provocan que el kernel **mate el proceso** (segmentation fault). Un driver corre en kernel space; corrupción afecta estructuras globales del sistema → posible *kernel panic*.

**Razonamiento:** Refuerza el propósito de la separación de privilegios.

---

#### Pregunta 3

**P:** Ejecutamos `./miniserv` dos veces seguidas. ¿Es el mismo proceso? ¿Mismo PID?

**Respuesta esperada:** No es el mismo proceso. Cada ejecución crea un proceso nuevo. El PID generalmente difiere (salvo reutilización posterior de números, pero no es la "misma instancia").

**Razonamiento:** Separa programa/binario de instancia/proceso.

---

### Ejercicio corto 1 — Conteo de `write`

**Enunciado (10 min):** Ejecuten `strace -e write echo uno dos tres 2>&1`. ¿Cuántas syscalls `write` aparecen? ¿A qué file descriptors?

**Objetivo:** Leer salida de `strace` sin abrumarse.

**Solución profesor:** Al menos una `write` a fd 1 con `"uno dos tres\n"`. Puede haber writes adicionales de libc al cargar; discutir variabilidad. fd 1 = stdout.

**Resultado esperado:** Identificación de fd y conteo ≥ 1.

---

### Ejercicio corto 2 — Mensaje con `write` solamente

**Enunciado (15 min):** Modificar `server.c` para imprimir `MiniServ iniciado` usando **solo** `write` (sin `printf`). Recompilar y verificar con `strace`.

**Objetivo:** Practicar API POSIX directa.

**Solución profesor:**

```c
#include <unistd.h>

int main(void)
{
    const char msg[] = "MiniServ iniciado\n";
    if (write(1, msg, sizeof(msg) - 1) < 0)
        return 1;
    return 0;
}
```

**Resultado:** Misma salida visible; `strace` muestra un `write` explícito en código fuente.

---

### Actividad de predicción

**Situación:**

```c
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    printf("A\n");
    write(1, "B\n", 2);
    printf("C\n");
    return 0;
}
```

**Pregunta:** ¿Qué orden exacto verán en pantalla antes de ejecutar?

**Respuesta:** Casi siempre `A`, `B`, `C` en líneas separadas — pero `printf` puede bufferizar stdout cuando no es terminal.

**Determinista vs. no:**

- **Determinista** si stdout está line-buffered (terminal): cada `\n` en `printf` hace flush → orden A, B, C.
- **Posible sorpresa** si stdout está fully buffered (redirección a archivo): `A` y `C` pueden aparecer juntos después de `B`, o reordenarse aparentemente al final.

**Demostración:**

```bash
./prediccion          # terminal
./prediccion > out.txt
cat out.txt
```

**Por qué:** Buffering es comportamiento de **stdio en user space**, no del kernel. MiniServ más adelante usará `write` directo en rutas críticas o `fflush` consciente — tema relacionado Semana 14.

---

## Errores y misconceptions

### 1. "`printf` escribe directamente al hardware"

**Incorrecto.** `printf` es stdio; bufferiza y llama funciones de más bajo nivel. El kernel interviene en la syscall subyacente.

**Demostración:** `strace -e write` muestra cuándo ocurre la syscall real.

---

### 2. "El kernel es un compilador / es la shell"

**Incorrecto.** El kernel no compila C ni interpreta comandos. La shell es un proceso user space que solicita al kernel crear otros procesos.

---

### 3. "Las system calls son opcionales si uso bibliotecas"

**Incorrecto.** Las bibliotecas **encapsulan** syscalls, no las eliminan. Todo acceso real a recursos termina en kernel.

---

### 4. "Un programa = un proceso siempre"

**Incorrecto.** Un programa puede tener cero procesos (no ejecutándose) o muchos (varias instancias). Un proceso puede reemplazar su imagen con `exec` (futuro).

---

### 5. "Si `write` devuelve 3, escribí 3 bytes al disco"

**Incorrecto.** Depende del fd: fd 1 puede ser terminal o pipe; fd de archivo puede ir a page cache. El retorno indica bytes **aceptados** por el kernel para ese fd, no necesariamente persistidos en disco.

---

### 6. "strace muestra lo que hace mi código C línea por línea"

**Incorrecto.** Muestra syscalls del **proceso**, incluyendo actividad de libc al iniciar (`mmap`, `openat` de librerías). Hay mucho más que el `main` visible.

---

### 7. Ignorar el valor de retorno de `write`

**Error de programación.** `write` puede escribir menos bytes de los pedidos (*partial write*) o fallar (-1). MiniServ robusto debe comprobar (Semana 14).

---

## Conexión profesional

| Mecanismo              | Dónde aparece                                              |
| ---------------------- | ---------------------------------------------------------- |
| Syscalls POSIX         | Todo servicio en Linux: Nginx, PostgreSQL, Redis, Docker   |
| Separación user/kernel | Contenedores comparten kernel; VMs tienen kernel guest     |
| Modelo de procesos     | Apache prefork, PostgreSQL postmaster + backends           |
| `strace`               | Depuración en producción/staging cuando no hay código fuente |
| Abstracción FD         | Nginx maneja conexiones como fds; event loops con `epoll`  |

**Arquitectura Nginx (visión adelantada):** master process + workers — mismo patrón que buscaremos en MiniServ V8/V15. La diferencia de desempeño entre servidores suele estar en **cuántas syscalls** y **cuántos context switches** hacen por petición.

**systemd:** PID 1 es proceso especial arrancado por el kernel; supervisa otros servicios — conexión con Semana 15.

**Docker:** procesos normales con aislamiento extra (namespaces, cgroups) — el kernel sigue siendo el árbitro.

---

## Historia y fun facts

1. **Multics y Unix (1960s–70s):** La idea de "todo es un archivo" y los file descriptors nace en Unix. Linux adopta ese modelo POSIX. MiniServ hereda 50 años de diseño.

2. **Linus Torvalds (1991):** Linux comenzó como kernel para PC x86; hoy corre en servidores, móviles (Android) y supercomputadoras. Las syscalls que trazamos con `strace` son la interfaz estable entre millones de programas y ese kernel.

3. **strace:** Evolucionó de `ptrace`, la syscall que permite a un proceso inspeccionar otro — meta: usar el SO para estudiar el SO.

4. **Costo de syscall:** En hardware moderno, user→kernel→user puede costar cientos de nanosegundos — irrelevante para MiniServ V0, crítico a millones de peticiones/segundo (Semana 16).

5. **Rings de privilegio x86:** Intel definió 4 rings; Linux usa ring 3 (user) y ring 0 (kernel). La frontera que dibujamos en clase tiene soporte hardware real.

---

## Tarea semanal

### Objetivo

Consolidar la observación de syscalls y entregar MiniServ V0 documentado.

### Descripción

1. Completar Milestone 0 si no quedó listo en clase.
2. Ejecutar tres trazas `strace` y documentar resultados.
3. Responder preguntas breves en reporte.

### Requisitos funcionales

- MiniServ imprime `MiniServ iniciado` y termina con éxito.

### Requisitos técnicos

- Entregar árbol de proyecto completo.
- Reporte en PDF o Markdown.

### Archivos a entregar

```text
miniserv/          (código + Makefile + README)
reporte-semana1.md o .pdf
```

### Trazas obligatorias en el reporte

```bash
strace -e trace=write ./miniserv
strace ls /tmp
strace cat /etc/hostname
```

Para cada una: listar syscalls identificadas y explicar en 2–3 oraciones qué hace el kernel.

### Ejemplo de ejecución

```bash
tar czf miniserv-semana1.tar.gz miniserv/ reporte-semana1.md
```

### Criterios de aceptación

```text
[ ] Milestone 0 cumple checklist
[ ] Reporte incluye capturas o copia de strace de los 3 comandos
[ ] Explicación correcta de al menos openat/read/write/close en cat
[ ] Respuestas a las 4 preguntas del reporte
```

### Preguntas para el reporte

1. ¿Qué diferencia hay entre el binario `miniserv` en disco y el proceso `./miniserv` en ejecución?
2. Cuando `cat` lee un archivo, ¿el buffer pasa por user space, kernel space o ambos? Justifique con lo visto en `strace`.
3. ¿Por qué `ls` llama a `write` si "solo lista" archivos?
4. ¿Qué syscall observarían si pudieran seguir el `printf` de MiniServ hasta el kernel?

---

## Cierre de la semana

### Conceptos fundamentales

1. El SO es intermediario entre programas y hardware.
2. Kernel vs. user space — frontera de privilegio.
3. Proceso = programa en ejecución con PID y recursos.
4. System call = puerta controlada al kernel.
5. libc envuelve syscalls (`write`, `read`, …).
6. `strace` observa syscalls en tiempo real.
7. `write(1, …)` escribe a stdout vía kernel.
8. MiniServ V0 ya es un proceso real observable.

### Mapa conceptual

```text
"MiniServ iniciado"
        │
        ▼
   ¿quién escribe?
        │
        ▼
  proceso + syscall write
        │
        ▼
  kernel → terminal driver
        │
        ▼
  MiniServ V0 (base del proyecto)
```

### Qué cambia en la Semana 2

MiniServ debe **escuchar en un puerto** y hablar con clientes. Eso introduce:

- **File descriptors** como tabla del proceso.
- **Sockets** como objetos del kernel accedidos vía fds.
- **I/O bloqueante:** el servidor se detiene en `accept`/`read` hasta que el kernel tenga datos.

La pregunta de la Semana 2 será:

> ¿Por qué el servidor "espera" cuando no hay clientes, y quién decide cuándo despertarlo?

---

*Fin de la Semana 1.*
