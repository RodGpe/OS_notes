# Prompt maestro — Generador semanal del curso de Sistemas Operativos

Actúa como profesor universitario de **Sistemas Operativos** y diseñador de material didáctico para estudiantes de Licenciatura en Ciencias de la Computación.

Tu tarea es desarrollar **una semana completa de un curso universitario de Sistemas Operativos de 16 semanas**, utilizando como hilo conductor un proyecto incremental llamado **MiniServ**, un servidor TCP/HTTP multiproceso implementado en **C sobre Linux**.

Cada semana consta de:

* **2 clases de 90 minutos**;
* explicación conceptual;
* demostraciones ejecutables;
* ejemplos en C;
* observación del comportamiento de Linux;
* preguntas para los estudiantes;
* ejercicios;
* y una evolución incremental de MiniServ cuando corresponda.

El objetivo principal no es producir una colección de secciones independientes, sino diseñar **dos clases que fluyan naturalmente de una idea a la siguiente**.

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

Cuando sea necesario introducir conceptos de redes para MiniServ, explica únicamente lo necesario para comprender el concepto de Sistemas Operativos correspondiente.

Este es un curso de **Sistemas Operativos**, no de redes.

Los sockets funcionan como una herramienta para observar:

* procesos;
* blocking;
* file descriptors;
* scheduling;
* IPC;
* concurrencia;
* manejo de recursos.

---

# 2. Principio pedagógico fundamental: construir una narrativa

La clase debe desarrollarse como una historia técnica.

No presentes los temas como:

```text
Definición
Definición
Definición
Código
Diagrama
Ejercicio
```

En cambio, construye una secuencia como:

```text
Observamos un problema
        │
        ▼
Hacemos una pregunta
        │
        ▼
Introducimos un concepto
        │
        ▼
Lo representamos visualmente
        │
        ▼
Lo vemos ocurrir en Linux
        │
        ▼
Lo reproducimos con C
        │
        ▼
Los estudiantes predicen qué ocurrirá
        │
        ▼
Lo conectamos con MiniServ
        │
        ▼
Aparece una nueva pregunta
        │
        ▼
Introducimos el siguiente concepto
```

Cada concepto importante debe surgir porque existe una pregunta o problema que requiere explicarlo.

Evita transiciones artificiales como:

> "Ahora veremos los diagramas."

o:

> "A continuación veremos ejemplos."

En lugar de ello, integra el recurso directamente:

> "Para entender qué significa que un proceso esté bloqueado, dibujemos sus posibles estados."

y después muestra el diagrama.

O:

> "Podemos comprobar que realmente existen dos procesos utilizando `ps`."

y entonces introduce el comando.

---

# 3. Patrón recomendado para enseñar cada concepto

Para cada concepto importante utiliza, cuando sea apropiado, esta secuencia:

## 1. Problema o pregunta

Presenta una situación concreta.

Por ejemplo:

> Nuestro servidor ejecuta `read()`, pero el cliente no envía nada.
> ¿Qué está haciendo el proceso durante ese tiempo?

---

## 2. Intuición inicial

Permite que el estudiante razone antes de dar la definición formal.

Puedes preguntar:

> ¿Está usando CPU?

> ¿Sigue ejecutando instrucciones?

> ¿Puede atender otro cliente?

---

## 3. Definición formal

Ahora introduce el concepto.

Por ejemplo:

> Un proceso es una instancia de un programa en ejecución junto con el estado necesario para que el sistema operativo pueda administrarlo.

---

## 4. Representación visual inmediata

El diagrama debe aparecer **en el momento en que ayuda a comprender la definición**.

Por ejemplo:

```text
Programa
  servidor

     │ exec
     ▼

┌───────────────────────┐
│       Proceso         │
│                       │
│ Código                │
│ Stack                 │
│ Heap                  │
│ Registros             │
│ File descriptors      │
│ PID                   │
│ Estado                │
└───────────────────────┘
```

Explica inmediatamente qué representa cada elemento relevante.

---

## 5. Evidencia observable en Linux

Después de presentar el modelo conceptual, muestra cómo podemos observar una parte del modelo en un sistema real.

Por ejemplo:

```bash
ps -o pid,ppid,state,cmd
```

o:

```bash
ls -l /proc/<pid>/fd
```

Explica:

* qué columna observar;
* qué significa;
* cómo se relaciona con el diagrama anterior.

---

## 6. Demostración pequeña en C

Después utiliza un programa corto que reproduzca el fenómeno.

No introduzcas código antes de que el concepto que pretende demostrar haya sido explicado.

---

## 7. Predicción

Antes de ejecutar el programa pregunta:

> ¿Qué creen que ocurrirá?

> ¿Quién imprimirá primero?

> ¿Cuántos procesos veremos?

> ¿Qué file descriptors tendrá el hijo?

---

## 8. Ejecución y análisis

Ejecuta el programa.

Contrasta:

```text
predicción
    vs
resultado observado
```

Explica cualquier comportamiento no determinista.

---

## 9. Conexión con MiniServ

Finalmente pregunta:

> ¿Dónde aparece exactamente este problema en nuestro servidor?

y conecta el concepto con una parte concreta del proyecto.

---

## 10. Transición

El concepto debe generar naturalmente la siguiente pregunta.

Ejemplo:

```text
fork() permite atender varios clientes
             │
             ▼
pero los hijos terminan
             │
             ▼
¿qué ocurre con ellos?
             │
             ▼
zombies
             │
             ▼
wait()
```

---

# 4. Filosofía del proyecto MiniServ

MiniServ funciona como hilo conductor de todo el curso.

Los conceptos de Sistemas Operativos no deben presentarse como capítulos completamente aislados.

Siempre que sea posible utiliza:

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
Medimos o inspeccionamos el resultado
```

Ejemplo:

```text
MiniServ atiende un solo cliente
            ↓
un cliente lento bloquea el servidor
            ↓
necesitamos concurrencia
            ↓
procesos
            ↓
fork()
```

Después:

```text
fork() crea procesos hijos
            ↓
los hijos terminan
            ↓
aparecen zombies
            ↓
wait() / waitpid()
```

Después:

```text
los workers necesitan estadísticas globales
            ↓
los procesos tienen memorias independientes
            ↓
shared memory
            ↓
varios procesos modifican los mismos datos
            ↓
race conditions
            ↓
sincronización
```

La pregunta recurrente del curso debe ser:

> **¿Qué problema de MiniServ nos ayuda a comprender o resolver este mecanismo del sistema operativo?**

---

# 5. Proyecto central: MiniServ

MiniServ comienza con:

```text
Cliente
   │
   ▼
Servidor secuencial
```

y evoluciona aproximadamente hacia:

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

Al final del curso MiniServ deberá permitir estudiar mecanismos como:

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

y conceptos como:

* kernel;
* user mode / kernel mode;
* system calls;
* file descriptors;
* procesos;
* scheduling;
* context switching;
* señales;
* IPC;
* pipes;
* memoria compartida;
* race conditions;
* sincronización;
* memoria virtual;
* Copy-on-Write;
* filesystem;
* blocking I/O;
* nonblocking I/O;
* multiplexación;
* manejo de recursos;
* supervisión;
* tolerancia a fallas;
* performance.

---

# 6. Evolución general de MiniServ

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

# 7. Plan general de las 16 semanas

## Semana 1 — Arquitectura del SO y llamadas al sistema

Conceptos:

* qué hace un sistema operativo;
* kernel;
* user space;
* kernel space;
* user mode;
* kernel mode;
* interrupciones;
* excepciones;
* traps;
* system calls;
* API del SO;
* POSIX;
* `read`;
* `write`;
* `open`;
* `close`;
* `strace`.

MiniServ:

* estructura inicial del proyecto.

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
* blocking I/O;
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

* programa vs. proceso;
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

* pasar de servidor secuencial a **fork-per-client**.

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
* cierre ordenado.

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

* proceso logger;
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

* estadísticas globales;
* demostrar condiciones de carrera.

---

## Semana 9 — Memoria compartida

Conceptos:

* aislamiento entre procesos;
* shared memory;
* `mmap`;
* `MAP_SHARED`;
* process-shared synchronization.

MiniServ:

mantener:

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
* `fork()` y COW;
* RSS;
* VSZ;
* `/proc/<pid>/maps`.

MiniServ:

* analizar memoria del master y workers.

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

creará un master y cuatro workers persistentes.

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

* servir archivos estáticos.

---

## Semana 13 — Blocking I/O, nonblocking I/O y multiplexación

Conceptos:

* blocking calls;
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
* worker crash;
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

## Semana 16 — Desempeño e integración

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

# 8. Cómo construir las dos clases de la semana

Genera **dos sesiones completas de 90 minutos**.

La planeación debe indicar tiempos, pero el material principal debe escribirse siguiendo el **orden real en que sería explicado durante la clase**.

No produzcas primero toda la teoría y posteriormente todas las demostraciones.

En cada momento de la clase intercala:

```text
explicación
   ↓
diagrama
   ↓
pregunta
   ↓
demo
   ↓
observación
   ↓
MiniServ
```

según resulte pedagógicamente apropiado.

---

# 9. Clase 1 — 90 minutos

La primera clase debe centrarse principalmente en construir el modelo conceptual.

Comienza preferentemente con un problema concreto.

Ejemplo:

```text
MiniServ tiene cierto comportamiento
            │
            ▼
¿Por qué ocurre?
```

Después desarrolla los conceptos progresivamente.

Una posible estructura es:


| Tiempo     | Flujo                                                  |
| ---------- | ------------------------------------------------------ |
| 0–10 min  | Problema motivador                                     |
| 10–25 min | Primer concepto + diagrama                             |
| 25–40 min | Observación/demo                                      |
| 40–55 min | Segundo concepto derivado del anterior                 |
| 55–70 min | Código + predicción                                  |
| 70–82 min | Aplicación a MiniServ                                 |
| 82–90 min | Recapitulación + pregunta que abre la siguiente clase |

No tienes que utilizar exactamente estos bloques.

Adáptalos al tema.

Lo importante es que haya **transiciones explícitas entre conceptos**.

Para cada transición explica brevemente por qué estamos pasando al siguiente tema.

Ejemplo:

> Ya sabemos que `fork()` crea otro proceso. Sin embargo, esto genera una nueva pregunta: ¿qué recursos del padre recibe el hijo?

Esa pregunta abre naturalmente el siguiente concepto.

---

# 10. Clase 2 — 90 minutos

La segunda clase debe consolidar los conceptos mediante experimentación.

Debe incluir cuando sea apropiado:

* modificación de MiniServ;
* programas pequeños;
* comandos Linux;
* debugging;
* experimentos;
* predicción;
* comparación;
* interpretación de resultados.

La práctica no debe estar desconectada de la teoría.

Antes de cada experimento indica:

> ¿Qué hipótesis estamos comprobando?

Después del experimento:

> ¿Qué evidencia observamos?

Y finalmente:

> ¿Qué significa esto para MiniServ?

---

# 11. Integración obligatoria de diagramas

Incluye como mínimo **3 diagramas ASCII útiles durante la semana**, pero NO los agrupes necesariamente en una sección separada.

Coloca cada diagrama exactamente donde ayude a introducir o aclarar un concepto.

Ejemplo al definir proceso:

```text
Programa en disco
      │
      │ ejecución
      ▼
┌─────────────────────┐
│       Proceso       │
│                     │
│ código              │
│ stack               │
│ heap                │
│ registros           │
│ file descriptors    │
│ PID                 │
│ estado              │
└─────────────────────┘
```

Inmediatamente después explica el diagrama.

Ejemplo al explicar estados:

```text
             scheduled
 READY ─────────────────► RUNNING
   ▲                         │
   │                         │ read()
   │                         ▼
   └──────────────────── BLOCKED
        datos disponibles
```

No muestres un diagrama antes de que el estudiante sepa qué problema intenta explicar.

No muestres diagramas decorativos.

---

# 12. Integración obligatoria de ejemplos en C

Los programas en C también deben aparecer en el punto exacto donde ayudan a validar una idea.

Ejemplo:

1. explicar conceptualmente `fork()`;
2. dibujar parent/child;
3. preguntar cuántos procesos existirán;
4. mostrar el programa C;
5. pedir una predicción;
6. ejecutarlo;
7. observarlo con `ps` o `pstree`;
8. relacionarlo con MiniServ.

Evita presentar cinco programas consecutivos sin narrativa entre ellos.

Cada programa debe tener un propósito pedagógico claro.

Para cada demostración incluye:

### Objetivo

Qué hipótesis o concepto estamos comprobando.

### Código completo

Incluye todos los headers necesarios.

### Compilación

```bash
gcc -Wall -Wextra demo.c -o demo
```

Agrega opciones como:

```bash
-pthread
```

cuando corresponda.

### Ejecución

```bash
./demo
```

### Salida esperada

Incluye una salida representativa.

### No determinismo

Explica cuando puedan variar:

* PIDs;
* orden;
* scheduling;
* timing;
* direcciones;
* resultados de concurrencia.

### Interpretación

No te limites a explicar el código línea por línea.

Relaciona lo observado con el modelo conceptual presentado inmediatamente antes.

---

# 13. Integración de herramientas Linux

Las herramientas deben aparecer como instrumentos para **comprobar algo que acabamos de explicar**.

Por ejemplo:

Después de definir PID:

```bash
ps -o pid,ppid,state,cmd
```

Después de explicar árboles de procesos:

```bash
pstree -p
```

Después de explicar file descriptors:

```bash
ls -l /proc/<pid>/fd
```

Después de explicar memoria virtual:

```bash
cat /proc/<pid>/maps
```

Después de explicar system calls:

```bash
strace
```

Puedes utilizar:

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

* qué acabamos de afirmar teóricamente;
* qué evidencia buscamos;
* dónde aparece en la salida;
* cómo interpretar esa evidencia.

---

# 14. MiniServ debe aparecer durante la explicación, no solamente al final

No reserves MiniServ únicamente para una sección final.

Haz referencia al servidor a lo largo de ambas clases.

Por ejemplo:

> Acabamos de ver que `read()` puede bloquear. Regresemos a MiniServ: ¿qué ocurre si nuestro único proceso ejecuta ese `read()` y el cliente nunca manda información?

Después introduce el siguiente concepto.

Al final de la semana sí debe existir una síntesis explícita:

```text
MiniServ anterior
      │
      ▼
problema observado
      │
      ▼
concepto aprendido
      │
      ▼
modificación
      │
      ▼
MiniServ nuevo
```

---

# 15. Milestone semanal

Define claramente la modificación que deben implementar.

Incluye:

## Requisitos funcionales

Qué debe hacer MiniServ.

## Requisitos técnicos

Qué APIs o mecanismos deben utilizar.

## Restricciones

Qué mecanismos de semanas futuras todavía no deben utilizarse.

Ejemplo:

> No utilizar threads; el objetivo de esta semana es comprender procesos.

## Criterios de aceptación

Por ejemplo:

```text
[ ] Compila con -Wall -Wextra
[ ] Utiliza fork()
[ ] Padre e hijo cumplen responsabilidades distintas
[ ] Los file descriptors se cierran correctamente
[ ] El comportamiento puede observarse con ps
```

Incluye comandos concretos para verificar cada criterio.

---

# 16. Preguntas y participación

Incluye al menos **3 preguntas conceptuales importantes**, pero intégralas en los momentos apropiados de la clase.

No las agrupes necesariamente al final.

Las preguntas deben aparecer antes de explicar aquello que queremos que el estudiante razone.

Por ejemplo:

> Hemos ejecutado `fork()`. ¿Qué valor creen que tiene `pid` en el hijo?

Después de permitir la predicción, explica el resultado.

Para cada pregunta proporciona al profesor:

* respuesta esperada;
* razonamiento;
* errores comunes que podrían responder los alumnos.

---

# 17. Actividades de predicción

Incluye al menos una actividad explícita de:

```text
predicción → ejecución → explicación
```

Por ejemplo:

```c
printf("A\n");
fork();
printf("B\n");
```

Pregunta:

> ¿Cuántas veces aparecerá `A`?

> ¿Cuántas veces aparecerá `B`?

> ¿En qué orden?

Después analiza:

* qué es determinista;
* qué no lo es;
* por qué.

---

# 18. Ejercicios cortos

Incluye al menos **2 ejercicios de programación de 10–25 minutos**.

Insértalos cuando los estudiantes ya tengan suficiente conocimiento para resolverlos.

No los pongas antes de haber enseñado los mecanismos necesarios.

Para cada ejercicio incluye:

* enunciado;
* objetivo;
* conocimientos utilizados;
* solución para el profesor;
* resultado esperado;
* posibles errores.

---

# 19. Errores conceptuales

Incluye como mínimo **5 misconceptions**, pero introdúcelos preferentemente junto al concepto correspondiente.

Ejemplo al enseñar `fork()`:

> **Error frecuente:** "`fork()` crea un thread."

Explica inmediatamente por qué es incorrecto.

Otro:

> **Error frecuente:** "El hijo continúa usando exactamente el mismo `pid`."

No reserves todos los errores para un apéndice final si pueden ayudar a evitar una confusión durante la explicación.

Al final puedes incluir una pequeña recapitulación de los errores más importantes.

---

# 20. Conexión profesional

Cuando un concepto haya quedado claro, muestra brevemente dónde aparece en sistemas reales.

Puedes relacionar cuando sea pertinente con:

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

Por ejemplo:

Después de explicar worker pools, puede mencionarse cómo distintos servidores utilizan arquitecturas basadas en workers.

La conexión debe ayudar a responder:

> ¿Por qué debería importarme este concepto fuera de esta clase?

No conviertas esta parte en publicidad ni en una lista de tecnologías.

---

# 21. Historia y fun facts

Incluye entre **2 y 5 datos interesantes** cuando aporten contexto.

Insértalos preferentemente cerca del concepto relacionado.

Pueden relacionarse con:

* Unix;
* POSIX;
* Linux;
* evolución del hardware;
* arquitectura de servidores;
* decisiones históricas de diseño.

Incluye contexto geopolítico solamente cuando realmente ayude a comprender la evolución tecnológica.

---

# 22. Tarea semanal

Diseña una tarea que continúe naturalmente el trabajo realizado durante las dos clases.

Debe incluir:

* objetivo;
* descripción;
* requisitos funcionales;
* requisitos técnicos;
* restricciones;
* archivos a entregar;
* ejemplo de ejecución;
* criterios de aceptación;
* preguntas para un pequeño reporte.

Preferentemente la tarea debe hacer avanzar MiniServ.

---

# 23. Cierre de la semana

Los últimos minutos de la segunda clase deben regresar a la pregunta con la que comenzó la semana.

Ejemplo:

```text
Al inicio:

¿Por qué MiniServ no puede atender
otro cliente mientras está bloqueado?

                ↓

Al final:

Ahora podemos explicarlo utilizando:

process states
blocking
scheduler
fork()
```

Termina con:

### Conceptos fundamentales

Entre 5 y 10 conceptos que el alumno debe recordar.

### Mapa conceptual

```text
problema observado
       │
       ▼
concepto
       │
       ▼
mecanismo del SO
       │
       ▼
evidencia en Linux
       │
       ▼
cambio en MiniServ
```

### Puente hacia la siguiente semana

No simplemente digas:

> La siguiente semana veremos señales.

Plantea un problema.

Por ejemplo:

> Ahora MiniServ puede crear muchos hijos. Sin embargo, ¿qué ocurre cuando esos hijos terminan?

Esa pregunta debe dejar preparado el siguiente tema.

---

# 24. Resultado esperado del material generado

El resultado final debe poder utilizarse casi directamente para impartir las clases.

Debe sentirse como una **narrativa docente**, no como documentación técnica.

Una sección típica de la clase debería verse aproximadamente así:

```text
PROBLEMA

Nuestro servidor está bloqueado esperando un cliente.

        ↓

PREGUNTA AL GRUPO

¿Está usando CPU mientras espera?

        ↓

CONCEPTO

Estado BLOCKED / WAITING.

        ↓

DIAGRAMA

READY → RUNNING → BLOCKED

        ↓

DEMO

Programa con read().

        ↓

OBSERVACIÓN

ps muestra el estado del proceso.

        ↓

INTERPRETACIÓN

El kernel quitó temporalmente el proceso de la CPU.

        ↓

MINISERV

Esto explica por qué nuestro único servidor
no puede atender otro cliente.

        ↓

SIGUIENTE PREGUNTA

¿Cómo podemos tener otro flujo de ejecución?

        ↓

PROCESOS / fork()
```

Utiliza este estilo conceptual como referencia.

No es obligatorio utilizar exactamente estos encabezados en todo momento, pero sí mantener esa progresión pedagógica.

---

# 25. Requisitos técnicos generales

Todo el material debe estar escrito en **español**.

Mantén términos técnicos estándar en inglés cuando resulte natural:

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

En los ejemplos C:

* incluye todos los headers;
* comprueba errores relevantes;
* utiliza `perror()` cuando corresponda;
* explica valores de retorno;
* evita prácticas inseguras como solución definitiva.

Cuando simplifiques algún comportamiento por motivos pedagógicos, indícalo.

La prioridad conceptual debe permanecer en:

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

y no en detalles secundarios de HTTP.

---

# 26. Continuidad obligatoria

La semana forma parte de un curso continuo.

No la diseñes como una clase independiente.

Reutiliza conceptos ya vistos.

No utilices mecanismos de semanas futuras como solución principal, salvo para mencionarlos brevemente como algo que será estudiado posteriormente.

Cada concepto nuevo debe tener una conexión con:

```text
algo que ya sabemos
       ↓
una nueva pregunta
       ↓
algo nuevo que necesitamos entender
```

La sensación que debe quedar al estudiante es:

> **Ahora entiendo una capa más de lo que realmente está haciendo MiniServ y el sistema operativo debajo de él.**

---


## Semana a desarrollar

Semana 1 — ¿Qué hace realmente un Sistema Operativo?

## Temas

* hardware vs. software;
* kernel;
* espacio de usuario;
* espacio de kernel;
* modo usuario;
* modo privilegiado;
* procesos;
* system calls;
* arquitectura básica de Linux.

### Pregunta central

Cuando nuestro programa en C ejecuta:

```c
write(fd, buffer, size);
```

¿quién escribe realmente los datos?

El programa no accede directamente al hardware.

```text
Aplicación
     │
     │ write()
     ▼
   libc
     │
     │ syscall
     ▼
──────── frontera usuario/kernel ────────
     │
     ▼
Kernel Linux
     │
     ▼
Filesystem / socket / dispositivo
```

---



 Tema                                   |
 -------------------------------------- |
 Motivación: ¿para qué existe un SO? |
 Modo usuario vs. modo kernel           |
 Qué es un proceso                     |
 System calls                           |
 Demostración con`strace`              |
 Preguntas y discusión                 |

---

## Sesión 2 — Laboratorio

Observar:

```bash
strace ls
```

y:

```bash
strace cat archivo.txt
```

Identificar llamadas como:

```text
openat()
read()
write()
close()
mmap()
```

### Milestone 0

Crear la estructura del proyecto:

```text
miniserv/
├── Makefile
├── src/
│   └── server.c
├── include/
└── README.md
```

Programa inicial:

```c
#include <stdio.h>

int main(void)
{
    printf("MiniServ iniciado\n");
    return 0;
}
```

Compilar:

```bash
gcc -Wall -Wextra src/server.c -o miniserv
```

Ejecutar:

```bash
./miniserv
```

Salida esperada:

```text
MiniServ iniciado

Genera las dos clases completas de 90 minutos siguiendo el enfoque narrativo, progresivo e integrado descrito anteriormente.
