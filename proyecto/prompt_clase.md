# Prompt maestro — Generador semanal del curso de Sistemas Operativos

Actúa como profesor universitario de **Sistemas Operativos** y diseñador de material didáctico para estudiantes de Licenciatura en Ciencias de la Computación.

Tu tarea es desarrollar **una semana completa de un curso universitario de Sistemas Operativos de 16 semanas**, utilizando como hilo conductor un proyecto incremental llamado **MiniServ**, un servidor TCP/HTTP multiproceso implementado en **C sobre Linux**.

Cada semana del curso consta de:

* **2 clases de 90 minutos**;
* explicación teórica;
* demostraciones ejecutables en Linux;
* ejemplos en C;
* ejercicios;
* análisis con herramientas del sistema operativo;
* y, cuando corresponda, una nueva etapa del proyecto MiniServ.

---

# 1. Contexto de los estudiantes

Los estudiantes ya conocen:

* programación básica en C;
* funciones;
* arreglos;
* `struct`;
* apuntadores;
* memoria dinámica básica;
* compilación con `gcc`;
* comandos básicos de Linux.

Sin embargo, debes asumir que están estudiando **Sistemas Operativos por primera vez**.

No asumas conocimiento previo profundo sobre:

* procesos;
* system calls;
* scheduling;
* memoria virtual;
* sincronización;
* IPC;
* sockets;
* señales;
* filesystems.

Cuando sea necesario introducir conceptos de redes para MiniServ, explica solamente lo necesario para comprender el concepto de Sistemas Operativos correspondiente.

El curso **no es un curso de redes**. Los sockets son una herramienta para estudiar el comportamiento del sistema operativo.

---

# 2. Filosofía pedagógica

El proyecto MiniServ funciona como hilo conductor del curso.

Los conceptos de Sistemas Operativos no deben presentarse como temas aislados.

Siempre que sea posible utiliza esta progresión:

```text
Problema observable en MiniServ
            │
            ▼
¿Por qué ocurre?
            │
            ▼
¿Qué mecanismo ofrece el SO?
            │
            ▼
Estudiamos el mecanismo
            │
            ▼
Lo observamos en Linux
            │
            ▼
Lo implementamos
            │
            ▼
Analizamos el nuevo comportamiento
```

Por ejemplo:

```text
El servidor atiende un solo cliente
            ↓
un cliente lento bloquea el servidor
            ↓
necesitamos concurrencia
            ↓
fork()
```

Después:

```text
los procesos hijos terminan
            ↓
aparecen zombies
            ↓
wait() / waitpid() / SIGCHLD
```

Después:

```text
los workers necesitan estadísticas globales
            ↓
shared memory
            ↓
aparecen race conditions
            ↓
sincronización
```

La pregunta recurrente durante el curso debe ser:

> ¿Qué problema de nuestro servidor resuelve este mecanismo del sistema operativo?

---

# 3. Proyecto central: MiniServ

MiniServ comienza con una arquitectura muy sencilla:

```text
Cliente
   │
   ▼
Servidor secuencial
```

y progresivamente evoluciona hasta tener aproximadamente esta arquitectura:

```text
                         Clientes
                            │
                            ▼
                     Listening Socket
                            │
                            ▼
                   ┌────────────────┐
                   │     Master     │
                   │                │
                   │ señales        │
                   │ supervisión    │
                   │ workers        │
                   └───────┬────────┘
                           │
              ┌────────────┼────────────┐
              │            │            │
              ▼            ▼            ▼
           Worker 1     Worker 2     Worker 3
              │            │            │
              └────────────┼────────────┘
                           │
                   Memoria compartida
                           │
                        Stats

Workers ───── IPC ─────► Logger
                           │
                           ▼
                      access.log
```

Al final del curso MiniServ deberá haber permitido estudiar mecanismos como:

```c
socket()
bind()
listen()
accept()

read()
write()
open()
close()

fork()
exec()
wait()
waitpid()

pipe()
mmap()
sigaction()
```

además de conceptos como:

* file descriptors;
* procesos;
* scheduling;
* cambios de contexto;
* señales;
* IPC;
* pipes;
* memoria compartida;
* race conditions;
* sincronización;
* memoria virtual;
* copy-on-write;
* filesystem;
* I/O bloqueante;
* multiplexación;
* manejo de recursos;
* supervisión;
* tolerancia a fallas;
* desempeño.

---

# 4. Evolución de MiniServ

La progresión general del proyecto es:

```text
V0
Programa inicial
   │
   ▼
V1
Servidor TCP secuencial
   │
   ▼
V2
Servidor HTTP secuencial
   │
   ▼
V3
Fork-per-client
   │
   ▼
V4
Señales y administración de hijos
   │
   ▼
V5
Logger mediante IPC
   │
   ▼
V6
Estadísticas mediante memoria compartida
   │
   ▼
V7
Sincronización
   │
   ▼
V8
Prefork worker pool
   │
   ▼
V9
Servidor de archivos estáticos
   │
   ▼
V10
Robustez y administración de recursos
   │
   ▼
V11
Performance y observabilidad
   │
   ▼
V12
Servidor final
```

---

# 5. Plan general de las 16 semanas

## Semana 1 — Arquitectura del SO y llamadas al sistema

Conceptos:

* qué hace un sistema operativo;
* kernel;
* user space y kernel space;
* modo usuario vs. modo kernel;
* interrupciones;
* excepciones;
* traps;
* system calls;
* API del SO;
* introducción a POSIX;
* `read`;
* `write`;
* `open`;
* `close`;
* `strace`.

MiniServ:

* crear la estructura inicial del proyecto.

---

## Semana 2 — File descriptors, sockets e I/O bloqueante

Conceptos:

* file descriptors;
* stdin;
* stdout;
* stderr;
* `read`;
* `write`;
* `close`;
* I/O bloqueante;
* sockets;
* `socket`;
* `bind`;
* `listen`;
* `accept`.

MiniServ:

* servidor TCP secuencial.

---

## Semana 3 — Creación y administración de procesos

Conceptos:

* proceso;
* PID;
* PPID;
* `fork`;
* parent;
* child;
* `wait`;
* `waitpid`;
* árbol de procesos;
* introducción a `exec`.

MiniServ:

* pasar del servidor secuencial a **fork-per-client**.

---

## Semana 4 — Herencia de recursos y procesos zombie

Conceptos:

* herencia de file descriptors;
* referencias a objetos abiertos;
* cierre correcto de FDs;
* zombies;
* orphans;
* tabla de procesos;
* `/proc`;
* `ps`;
* `pstree`.

MiniServ:

* corregir ownership de file descriptors;
* provocar y eliminar zombies.

---

## Semana 5 — Señales

Conceptos:

* señales;
* eventos asíncronos;
* `SIGINT`;
* `SIGTERM`;
* `SIGCHLD`;
* `SIGPIPE`;
* `sigaction`;
* `EINTR`;
* signal handlers.

MiniServ:

* recolección de hijos;
* cierre ordenado del servidor.

---

## Semana 6 — Scheduling y cambios de contexto

Conceptos:

* estados de proceso;
* ready;
* running;
* blocked;
* scheduler;
* preemption;
* quantum;
* context switch;
* CPU-bound;
* I/O-bound;
* throughput;
* latency.

MiniServ:

* comparar servidor secuencial y fork-per-client.

---

## Semana 7 — IPC mediante pipes

Conceptos:

* IPC;
* `pipe`;
* read end;
* write end;
* herencia de pipes;
* bloqueo;
* productores y consumidores.

MiniServ:

* proceso logger independiente;
* workers envían mensajes mediante IPC.

---

## Semana 8 — Race conditions y sincronización

Conceptos:

* concurrencia;
* race condition;
* operaciones no atómicas;
* critical section;
* mutual exclusion;
* mutex;
* semaphore;
* deadlock.

MiniServ:

* introducir estadísticas globales;
* demostrar condiciones de carrera.

---

## Semana 9 — Memoria compartida

Conceptos:

* aislamiento de procesos;
* shared memory;
* `mmap`;
* `MAP_SHARED`;
* process-shared synchronization.

MiniServ:

mantener estadísticas como:

```text
requests
errors
bytes_sent
active_clients
```

---

## Semana 10 — Memoria virtual y Copy-on-Write

Conceptos:

* virtual address;
* physical address;
* pages;
* page tables;
* page faults;
* demand paging;
* Copy-on-Write;
* relación entre `fork()` y COW;
* RSS;
* VSZ;
* `/proc/<pid>/maps`.

MiniServ:

* analizar memoria del master y los workers.

---

## Semana 11 — Prefork worker pool

Conceptos:

* costo de creación de procesos;
* lifecycle;
* worker pool;
* reutilización;
* scheduling;
* resource management.

MiniServ:

```bash
./miniserv -w 4
```

debe crear un master y cuatro workers persistentes.

---

## Semana 12 — Filesystem

Conceptos:

* archivos;
* directorios;
* inodes;
* metadata;
* offsets;
* permisos;
* page cache;
* `open`;
* `read`;
* `stat`.

MiniServ:

* servir archivos estáticos mediante HTTP.

---

## Semana 13 — I/O bloqueante, nonblocking y multiplexación

Conceptos:

* llamadas bloqueantes;
* slow clients;
* `O_NONBLOCK`;
* `select`;
* `poll`;
* introducción a `epoll`;
* readiness;
* event-driven I/O.

MiniServ:

* analizar workers bloqueados;
* `epoll` como extensión avanzada.

---

## Semana 14 — Administración de recursos, robustez y seguridad

Conceptos:

* file descriptor exhaustion;
* `RLIMIT_NOFILE`;
* `RLIMIT_NPROC`;
* `ulimit`;
* partial reads;
* partial writes;
* errores de system calls;
* `SIGPIPE`;
* least privilege;
* path traversal.

MiniServ:

* manejar errores;
* desconexiones;
* límites;
* requests inválidos;
* evitar acceso fuera de `www/`.

---

## Semana 15 — Supervisión y tolerancia a fallas

Conceptos:

* supervisor;
* worker crashes;
* `SIGCHLD`;
* `waitpid`;
* recuperación;
* restart;
* graceful shutdown.

MiniServ:

```text
Worker muere
      ↓
SIGCHLD
      ↓
Master detecta
      ↓
fork()
      ↓
Worker nuevo
```

---

## Semana 16 — Desempeño e integración final

Conceptos:

* latency;
* throughput;
* CPU;
* memoria;
* context switches;
* system calls;
* benchmarking;
* observabilidad.

Comparar:

```text
secuencial
    vs
fork-per-client
    vs
prefork
    vs
prefork + epoll
```

Herramientas:

```bash
time
ps
top
strace
lsof
ss
/proc
```

---

# 6. Formato obligatorio para desarrollar cada semana

Utiliza la información específica de la semana proporcionada al final de este prompt y genera **las dos sesiones completas de 90 minutos**.

El resultado debe contener las siguientes secciones.

---

## A. Visión general de la semana

Incluye:

### Objetivos de aprendizaje

Define entre **4 y 8 objetivos específicos y medibles**.

Utiliza formulaciones como:

* explicar;
* identificar;
* comparar;
* implementar;
* diagnosticar;
* analizar.

---

### Motivación

Presenta primero un problema concreto.

Explica:

* qué problema aparece;
* por qué importa para Sistemas Operativos;
* cómo se manifiesta en MiniServ;
* qué mecanismo del SO permitirá estudiarlo o resolverlo.

---

### Conexión con semanas anteriores

Muestra claramente la continuidad.

Por ejemplo:

```text
Concepto aprendido anteriormente
           ↓
nuevo problema
           ↓
concepto de esta semana
           ↓
nuevo MiniServ
```

---

# 7. Clase 1 — 90 minutos

Construye un plan detallado utilizando una tabla similar a:


| Tiempo     | Actividad                 |
| ---------- | ------------------------- |
| 0–10 min  | Problema motivador        |
| 10–25 min | Concepto teórico         |
| 25–40 min | Explicación con diagrama |
| 40–55 min | Demostración             |
| 55–70 min | Segundo concepto          |
| 70–82 min | Ejercicio                 |
| 82–90 min | Discusión y cierre       |

Adapta los tiempos según el tema.

Para cada bloque explica:

* qué debe explicar el profesor;
* qué ejemplos utilizar;
* qué dibujar en el pizarrón;
* qué preguntas hacer a los estudiantes;
* cómo conectarlo con MiniServ.

---

# 8. Clase 2 — 90 minutos

Diseña la segunda sesión completa.

Debe tener un componente práctico mayor cuando el tema lo permita.

Puede incluir:

* laboratorio;
* programación;
* modificación de MiniServ;
* análisis con herramientas Linux;
* experimentos;
* debugging;
* predicción de comportamiento;
* comparación de resultados.

Incluye también la distribución exacta de los 90 minutos.

---

# 9. Explicación teórica

Desarrolla con profundidad universitaria todos los conceptos importantes de la semana.

Para cada concepto explica:

1. qué es;
2. por qué existe;
3. qué problema resuelve;
4. cómo funciona;
5. qué participación tiene el kernel;
6. cómo se relaciona con MiniServ;
7. errores conceptuales frecuentes.

No asumas que el estudiante conoce previamente el concepto.

---

# 10. Diagramas

Incluye como mínimo **3 diagramas ASCII técnicamente útiles**.

Deben poder utilizarse directamente en clase o reproducirse en el pizarrón.

Por ejemplo:

```text
Aplicación
    │
 syscall
    │
──── frontera user/kernel ────
    │
    ▼
Kernel
```

o:

```text
Parent
   │
 fork()
   │
 ┌─┴────────┐
 ▼          ▼
Parent     Child
```

Cada diagrama debe tener una explicación.

No incluyas diagramas puramente decorativos.

---

# 11. Demostraciones en C

Incluye las demostraciones necesarias para comprender los conceptos.

Cada demostración debe:

* ser pequeña;
* poder escribirse o explicarse en clase;
* compilar realmente;
* mostrar principalmente un concepto.

Para **cada programa** incluye obligatoriamente:

### Objetivo

Qué concepto demuestra.

### Código completo

Incluye todos los headers.

### Compilación

Ejemplo:

```bash
gcc -Wall -Wextra demo.c -o demo
```

Agrega opciones como:

```bash
-pthread
```

cuando correspondan.

### Ejecución

Ejemplo:

```bash
./demo
```

### Salida esperada

Incluye una salida representativa.

### No determinismo

Indica explícitamente si pueden variar:

* PIDs;
* orden de impresión;
* timing;
* scheduling;
* direcciones;
* resultados de una race condition.

### Explicación paso a paso

Describe:

1. qué hace el programa;
2. cuándo cruza al kernel;
3. qué syscall se ejecuta;
4. qué ocurre con el proceso;
5. qué recurso del kernel se utiliza;
6. qué resultado observamos.

---

# 12. Herramientas Linux

Utiliza herramientas que permitan **observar el concepto estudiado**, no solamente mencionarlas.

Puedes utilizar según corresponda:

```bash
ps
pstree
strace
lsof
ss
top
htop
time

cat /proc/<pid>/status
cat /proc/<pid>/maps
cat /proc/<pid>/stat
ls -l /proc/<pid>/fd
```

Para cada comando explica:

* qué debemos observar;
* qué significa la salida;
* qué relación tiene con el concepto estudiado;
* qué debería identificar el alumno.

---

# 13. Integración con MiniServ

Esta sección debe conectar explícitamente teoría y proyecto.

Presenta:

### Estado inicial

¿Qué puede hacer MiniServ al iniciar la semana?

### Problema

¿Qué limitación tiene?

### Mecanismo del Sistema Operativo

¿Qué concepto estudiado permite resolverla o comprenderla?

### Cambio arquitectónico

¿Qué cambia en MiniServ?

Representa la transición como:

```text
MiniServ actual
      │
      ▼
problema
      │
      ▼
concepto del SO
      │
      ▼
modificación
      │
      ▼
MiniServ nuevo
```

---

# 14. Milestone semanal

Define exactamente qué deben implementar los estudiantes.

Incluye:

## Requisitos funcionales

Qué comportamiento debe tener.

## Requisitos técnicos

Qué APIs o mecanismos deben utilizar.

## Restricciones

Indica mecanismos que todavía no deben utilizarse si perjudican el objetivo pedagógico.

Por ejemplo:

> En este milestone no se permite utilizar threads.

## Criterios de aceptación

Usa un checklist como:

```text
[ ] Compila con -Wall -Wextra
[ ] Se ejecuta correctamente
[ ] Utiliza fork()
[ ] No deja zombies
[ ] Cierra correctamente los file descriptors
```

## Pruebas

Incluye comandos concretos para comprobar el milestone.

---

# 15. Actividades de clase

Incluye como mínimo:

### 3 preguntas conceptuales

Para cada una proporciona:

* pregunta;
* respuesta esperada;
* razonamiento.

Las preguntas deben requerir comprensión, no solamente memorización.

---

### 2 ejercicios cortos de programación

Pensados para aproximadamente:

```text
10–25 minutos
```

Incluye:

* enunciado;
* objetivo;
* solución para el profesor;
* resultado esperado.

---

### 1 actividad de predicción

Presenta código o una situación y pregunta:

> ¿Qué creen que ocurrirá antes de ejecutarlo?

Después explica:

* qué partes son deterministas;
* qué partes no;
* por qué.

---

# 16. Errores y misconceptions

Incluye como mínimo **5 errores frecuentes**.

Por ejemplo:

### "`fork()` crea un thread."

Incorrecto.

Explica:

* por qué;
* qué ocurre realmente;
* cómo demostrarlo.

Incluye tanto:

* errores conceptuales;
* errores de programación;
* malas interpretaciones de resultados de Linux.

---

# 17. Conexión profesional

Explica dónde aparecen estos mecanismos en sistemas reales.

Cuando sea apropiado utiliza ejemplos como:

* Linux;
* Unix;
* Nginx;
* Apache;
* PostgreSQL;
* Redis;
* Docker;
* systemd;
* shells;
* servidores modernos.

La conexión debe ser arquitectónica y pedagógica.

No conviertas esta sección en publicidad o historia empresarial.

---

# 18. Historia y fun facts

Incluye entre **2 y 5 datos interesantes** cuando realmente aporten contexto.

Pueden relacionarse con:

* Unix;
* POSIX;
* Linux;
* evolución del hardware;
* evolución de arquitecturas de servidores;
* decisiones históricas de diseño.

Agrega contexto geopolítico únicamente cuando ayude genuinamente a comprender la evolución tecnológica.

---

# 19. Tarea semanal

Diseña una tarea directamente relacionada con el contenido de la semana y, preferentemente, con MiniServ.

Incluye:

* objetivo;
* descripción;
* requisitos funcionales;
* requisitos técnicos;
* archivos a entregar;
* ejemplo de ejecución;
* criterios de aceptación;
* preguntas para un pequeño reporte.

---

# 20. Cierre de la semana

Termina con:

## Conceptos fundamentales

Lista los **5–10 conceptos** que el alumno debe recordar.

## Mapa conceptual

Incluye un pequeño diagrama como:

```text
problema
   │
   ▼
concepto del SO
   │
   ▼
mecanismo
   │
   ▼
MiniServ
```

## Qué cambia en la siguiente semana

Explica brevemente qué nuevo problema queda abierto y cómo conduce naturalmente al siguiente tema del curso.

---

# 21. Requisitos técnicos generales

Todo el material debe estar escrito en **español**.

Mantén términos técnicos estándar en inglés cuando resulte natural, por ejemplo:

* file descriptor;
* system call;
* scheduler;
* context switch;
* race condition;
* shared memory;
* blocking I/O.

Explícalos en español cuando aparezcan por primera vez.

Usa:

* C real;
* Linux;
* APIs POSIX/Linux;
* programas compilables;
* comportamiento técnicamente correcto.

En los ejemplos de C:

* incluye headers;
* verifica errores relevantes;
* usa `perror()` cuando sea apropiado;
* explica valores de retorno importantes;
* evita prácticas inseguras como solución definitiva.

Cuando simplifiques un comportamiento real por razones didácticas, indícalo.

Prioriza siempre el estudio de:

```text
procesos
system calls
kernel
scheduling
I/O
IPC
sincronización
memoria
filesystem
resource management
```

por encima de detalles secundarios de HTTP o redes.

---

# 22. Continuidad obligatoria

No desarrolles esta semana como una clase independiente.

Debes asumir que forma parte del curso descrito anteriormente.

Reutiliza conceptos ya aprendidos.

No introduzcas mecanismos de semanas futuras como solución principal salvo que sea necesario mencionarlos para motivar lo que viene después.

La sensación que debe quedar al estudiante cada semana es:

> Ahora entiendo una capa más de lo que realmente está haciendo MiniServ y el sistema operativo debajo de él.

---

# ÚNICA SECCIÓN QUE SE MODIFICA PARA CADA SEMANA

## Semana a desarrollar

**Semana:** [NÚMERO]

**Nombre de la semana:** [NOMBRE]

**Temas principales:**

* [TEMA 1]
* [TEMA 2]
* [TEMA 3]
* [TEMA 4]
* [TEMA 5]

**MiniServ al iniciar la semana:**

[Describe brevemente qué versión de MiniServ ya tienen los estudiantes y qué funcionalidades fueron implementadas durante las semanas anteriores.]

**Problema que motiva esta semana:**

[Describe la limitación concreta que ahora aparece en MiniServ.]

**Milestone que debe existir al terminar la semana:**

[Describe exactamente qué nueva funcionalidad o modificación arquitectónica deberán implementar.]

**Conceptos de semanas anteriores que puedes asumir:**

* [CONCEPTO 1]
* [CONCEPTO 2]
* [CONCEPTO 3]

**Conceptos que NO debes asumir todavía:**

* [CONCEPTO FUTURO 1]
* [CONCEPTO FUTURO 2]
* [CONCEPTO FUTURO 3]

Genera el material completo de esta semana siguiendo todas las instrucciones anteriores.
