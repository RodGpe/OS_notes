# Prompt maestro — Generador semanal del curso de Sistemas Operativos

Actúa como profesor universitario de **Sistemas Operativos** y diseñador de material didáctico para estudiantes de Licenciatura en Ciencias de la Computación.

Tu tarea es desarrollar **una clase completa de un curso universitario de Sistemas Operativos de 16 semanas**, utilizando como hilo conductor un proyecto incremental llamado **MiniServ**, un servidor TCP/HTTP multiproceso implementado en **C sobre Linux**.

Cada semana del curso consta de:

- **1 clases de 120 minutos**;
- explicación teórica;
- demostraciones ejecutables en Linux;
- ejemplos en C;
- ejercicios;
- análisis con herramientas del sistema operativo;
- y, cuando corresponda, una nueva etapa del proyecto MiniServ.

---

# 1. Contexto de los estudiantes

Los estudiantes ya conocen:

- programación básica en C;
- funciones;
- arreglos;
- `struct`;
- apuntadores;
- memoria dinámica básica;
- compilación con `gcc`;
- comandos básicos de Linux.

Sin embargo, debes asumir que están estudiando **Sistemas Operativos por primera vez**.

No asumas conocimiento previo profundo sobre:

- procesos;
- system calls;
- scheduling;
- memoria virtual;
- sincronización;
- IPC;
- sockets;
- señales;
- filesystems.

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

- file descriptors;
- procesos;
- scheduling;
- cambios de contexto;
- señales;
- IPC;
- pipes;
- memoria compartida;
- race conditions;
- sincronización;
- memoria virtual;
- copy-on-write;
- filesystem;
- I/O bloqueante;
- multiplexación;
- manejo de recursos;
- supervisión;
- tolerancia a fallas;
- desempeño.

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



MES 1 — Llamadas al sistema, procesos y servidor básico

# Semana 1 — ¿Qué hace realmente un Sistema Operativo?

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

## Sesión 1 — 90 minutos


| Tiempo     | Tema                                   |
| ---------- | -------------------------------------- |
| 0–15 min  | Motivación: ¿para qué existe un SO? |
| 15–35 min | Modo usuario vs. modo kernel           |
| 35–50 min | Qué es un proceso                     |
| 50–70 min | System calls                           |
| 70–80 min | Demostración con`strace`              |
| 80–90 min | Preguntas y discusión                 |

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
```

---

# Semana 2 — Descriptores de archivo e I/O bloqueante

## Conceptos

* file descriptor;
* `open`;
* `read`;
* `write`;
* `close`;
* sockets;
* I/O bloqueante;
* stdin;
* stdout;
* stderr.

Idea fundamental de Unix:

> Muchas abstracciones del sistema se manipulan utilizando descriptores de archivo.

Ejemplo:

```text
Proceso

Tabla de FDs
┌────┬──────────────────┐
│ 0  │ stdin            │
│ 1  │ stdout           │
│ 2  │ stderr           │
│ 3  │ socket servidor  │
│ 4  │ socket cliente   │
└────┴──────────────────┘
```

---

## Introducción mínima a sockets

Presentar:

```c
socket()
bind()
listen()
accept()
read()
write()
close()
```

Ciclo básico:

```text
socket()
   │
bind()
   │
listen()
   │
accept()
   │
read()
   │
write()
   │
close()
```

---

# Milestone 1 — Servidor TCP secuencial

El estudiante implementará un servidor al que pueda conectarse usando:

```bash
nc localhost 8080
```

Cliente:

```text
hola
```

Servidor responde:

```text
Hola desde MiniServ
```

El punto importante no es todavía TCP.

El punto importante es observar:

```c
read(client_fd, buffer, sizeof(buffer));
```

Cuando no llegan datos:

```text
RUNNING
   │
read()
   ▼
BLOCKED
   │
llegan datos
   ▼
READY
   │
scheduler
   ▼
RUNNING
```

---

# Semana 3 — Creación de procesos

## Conceptos

* `fork()`;
* proceso padre;
* proceso hijo;
* PID;
* PPID;
* árbol de procesos;
* espacio de direcciones;
* `wait()`;
* `waitpid()`.

Programa demostrativo:

```c
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        printf("Hijo: PID=%d PPID=%d\n",
               getpid(), getppid());
        return 0;
    }

    printf("Padre: PID=%d hijo=%d\n",
           getpid(), pid);

    wait(NULL);

    return 0;
}
```

Compilar:

```bash
gcc -Wall -Wextra fork_demo.c -o fork_demo
```

Ejecutar:

```bash
./fork_demo
```

Posible salida:

```text
Padre: PID=4000 hijo=4001
Hijo: PID=4001 PPID=4000
```

También podría aparecer primero el mensaje del hijo.

Eso es correcto.

El orden depende del scheduler.

---

# Milestone 2 — Fork por cliente

Transformar:

```text
un servidor
```

en:

```text
               Parent
                 │
                 │ fork()
                 │
       ┌─────────┼─────────┐
       │         │         │
     Child     Child     Child
       │         │         │
 Client A    Client B    Client C
```

Estructura conceptual:

```c
while (1) {

    client_fd = accept(...);

    pid_t pid = fork();

    if (pid == 0) {

        close(server_fd);

        handle_client(client_fd);

        close(client_fd);

        _exit(0);
    }

    close(client_fd);
}
```

Ahora múltiples clientes pueden ser atendidos simultáneamente.

---

# Semana 4 — Herencia de descriptores y procesos zombie

Después de:

```c
client_fd = accept(...);
fork();
```

los descriptores se heredan.

```text
Antes de fork()

Parent
FD 3 → listening socket
FD 4 → client socket

           fork()
             │
      ┌──────┴──────┐

Parent              Child

FD3 → server        FD3 → server
FD4 → client        FD4 → client
```

Por eso:

Padre:

```c
close(client_fd);
```

Hijo:

```c
close(server_fd);
```

---

## Procesos zombie

```text
Parent
  │
  ├── Child
  │      │
  │     exit()
  │      │
  │      ▼
  │    ZOMBIE
  │
 wait()
  │
  ▼
Información recolectada
```

Observar con:

```bash
ps -o pid,ppid,state,cmd
```

Un zombie aparecerá con estado:

```text
Z
```

### Actividad

Los estudiantes deben:

1. generar zombies intencionalmente;
2. observarlos;
3. explicar por qué existen;
4. arreglar el problema usando `wait()` o `waitpid()`.

---

# MES 2 — Señales, IPC y sincronización

# Semana 5 — Señales

## Conceptos

* señales;
* eventos asíncronos;
* `SIGINT`;
* `SIGTERM`;
* `SIGCHLD`;
* `SIGPIPE`;
* `sigaction()`;
* `EINTR`.

Ejemplo:

```bash
Ctrl+C
```

produce típicamente:

```text
SIGINT
```

El servidor no debería simplemente desaparecer.

Queremos:

```text
SIGTERM / SIGINT
       │
       ▼
     Master
       │
       ├── detener nuevos clientes
       ├── notificar workers
       ├── esperar workers
       ├── cerrar sockets
       └── terminar
```

---

# Milestone 3 — Manejo de señales

Implementar:

* `SIGINT`;
* `SIGTERM`;
* `SIGCHLD`;
* cierre ordenado;
* recolección de hijos.

Usar:

```c
sigaction()
```

como API principal.

---

# Semana 6 — Scheduling y cambios de contexto

Ahora tenemos suficientes procesos para entender por qué existe el scheduler.

## Conceptos

* scheduler;
* ready queue;
* running;
* waiting;
* blocked;
* preemption;
* quantum;
* context switch;
* proceso CPU-bound;
* proceso I/O-bound.

```text
Ready Queue

P2 → P5 → P8 → P3
          │
          ▼
         CPU
       ┌─────┐
       │ P1  │
       └─────┘
```

El servidor suele ser principalmente:

```text
I/O-bound
```

porque pasa mucho tiempo esperando:

```text
network
disk
clients
```

---

## Experimento

Comparar:

```text
Servidor secuencial
```

con:

```text
fork-per-client
```

Generar múltiples clientes:

```bash
seq 1 20 | xargs -n1 -P20 -I{} \
curl -s http://127.0.0.1:8080/
```

Medir:

```bash
time comando
```

Discutir:

* concurrencia;
* throughput;
* cambio de contexto;
* uso de CPU;
* uso de memoria.

---

# Semana 7 — Comunicación entre procesos: Pipes

## Problema

Supongamos que todos los workers escriben directamente en:

```text
server.log
```

Podemos separar responsabilidades usando IPC.

```text
Worker 1 ───┐
Worker 2 ───┼──► Logger ───► access.log
Worker 3 ───┘
```

## Conceptos

* IPC;
* pipe;
* extremo de lectura;
* extremo de escritura;
* `pipe()`;
* mensajes;
* productores;
* consumidor.

---

# Milestone 4 — Logger independiente

Agregar:

```text
logger.c
logger.h
```

Los workers generan mensajes como:

```text
GET /index.html 200 1024
```

y los envían al proceso logger.

El logger es el único proceso que modifica:

```text
access.log
```

---

# Semana 8 — Race conditions y sincronización

Supongamos que tenemos:

```c
requests++;
```

No debemos asumir que es una única operación indivisible.

Conceptualmente:

```text
LOAD requests
ADD 1
STORE requests
```

Dos procesos pueden hacer:

```text
Worker A                    Worker B

LOAD 10
                            LOAD 10

ADD 1
                            ADD 1

STORE 11
                            STORE 11
```

Resultado esperado:

```text
12
```

Resultado posible:

```text
11
```

---

## Conceptos

* race condition;
* critical section;
* atomicidad;
* mutual exclusion;
* semaphore;
* mutex;
* deadlock.

---

# Semana 9 — Memoria compartida

Normalmente:

```text
Proceso A              Proceso B

counter = 10           counter = 10

     memorias independientes
```

Pero podemos crear:

```text
Proceso A
    │
    ▼
┌──────────────────┐
│ Memoria          │
│ compartida       │
│                  │
│ requests = 100   │
└──────────────────┘
    ▲
    │
Proceso B
```

Ejemplo mediante:

```c
mmap(
    NULL,
    sizeof(struct server_stats),
    PROT_READ | PROT_WRITE,
    MAP_SHARED | MAP_ANONYMOUS,
    -1,
    0
);
```

---

## Estructura de estadísticas

```c
struct server_stats {
    unsigned long requests;
    unsigned long errors;
    unsigned long bytes_sent;
    unsigned int active_clients;
};
```

---

# Milestone 5 — Estadísticas compartidas

Los workers actualizan:

```text
requests
errors
bytes_sent
active_clients
```

Primero sin sincronización.

Realizar pruebas.

Después proteger los datos.

El alumno deberá demostrar experimentalmente que una race condition puede producir resultados incorrectos.

---

# MES 3 — Memoria, worker pools y filesystem

# Semana 10 — Memoria virtual y Copy-on-Write

Hasta ahora hemos utilizado:

```c
fork()
```

sin estudiar profundamente qué ocurre con la memoria.

Una idea ingenua sería:

```text
fork()

copiar toda la memoria
```

Pero eso sería muy costoso.

Linux y otros sistemas modernos utilizan técnicas como:

```text
Copy-on-Write
```

Inicialmente:

```text
Parent virtual memory
          │
          ▼
        Page A
          ▲
          │
Child virtual memory
```

Ambos pueden referenciar inicialmente la misma página física.

Cuando uno modifica:

```text
Parent                Child

Page A                Page B
```

---

## Conceptos

* dirección virtual;
* dirección física;
* página;
* page table;
* page fault;
* copy-on-write;
* demand paging;
* RSS;
* VSZ.

---

## Observación en Linux

```bash
cat /proc/<PID>/maps
```

y:

```bash
cat /proc/<PID>/status
```

También:

```bash
ps -o pid,ppid,vsz,rss,stat,cmd
```

Preguntar:

> Si copy-on-write evita copiar inmediatamente toda la memoria, ¿por qué no crear 100,000 procesos?

Esto lleva a discutir:

* estructuras del kernel;
* page tables;
* scheduling;
* file descriptors;
* memoria física;
* límites de procesos.

---

# Semana 11 — Pool de procesos prefork

Problema actual:

```text
request
   │
accept()
   │
fork()
   │
serve
   │
exit()
```

Por cada petición creamos un proceso.

Eso tiene costo.

Nueva arquitectura:

```text
            Master
              │
       fork al inicio
              │
     ┌────────┼────────┐
     │        │        │
 Worker 1  Worker 2  Worker 3
     │        │        │
 accept()  accept()  accept()
```

---

## Comparación

### Fork-per-client

```text
accept
fork
serve
exit
```

### Pre-fork

```text
startup

fork
fork
fork
fork

después:

accept
serve
accept
serve
accept
serve
```

---

# Milestone 6 — Pool de workers

El servidor debe aceptar:

```bash
./miniserv -w 4
```

y crear cuatro workers.

Observar:

```bash
ps -ef | grep miniserv
```

Posible resultado:

```text
5010 miniserv master
5011 miniserv worker
5012 miniserv worker
5013 miniserv worker
5014 miniserv worker
```

---

## Conexión profesional

Arquitecturas prefork han sido utilizadas por servidores reales.

Por ejemplo, Apache HTTP Server históricamente ha utilizado distintos modelos de procesamiento, incluidos modelos basados en múltiples procesos.

Esto permite conectar directamente:

```text
teoría del SO
     │
     ▼
arquitectura de software real
```

---

# Semana 12 — Sistemas de archivos

Ahora el servidor comenzará a servir archivos reales.

Petición:

```text
GET /hello.txt
```

Flujo:

```text
HTTP Request
     │
     ▼
parsear path
     │
     ▼
open()
     │
     ▼
filesystem
     │
     ▼
read()
     │
     ▼
write(socket)
```

---

## Conceptos

* archivo;
* directorio;
* inode;
* metadata;
* file offset;
* permisos;
* filesystem;
* page cache;
* `stat()`.

Ejemplo:

```c
int fd = open("www/index.html", O_RDONLY);

ssize_t n = read(fd, buffer, sizeof(buffer));

write(client_fd, buffer, n);

close(fd);
```

---

# Milestone 7 — Servidor de archivos estáticos

Estructura:

```text
www/
├── index.html
├── style.css
├── image.jpg
└── hello.txt
```

Debe servir:

```text
HTML
CSS
TXT
JPEG
PNG
```

---

## Error conceptual importante

Para datos binarios no debemos hacer:

```c
strlen(buffer);
```

para determinar cuánto se leyó.

Debemos utilizar el valor regresado por:

```c
read()
```

por ejemplo:

```c
ssize_t n = read(fd, buffer, sizeof(buffer));
```

y posteriormente:

```c
write(client_fd, buffer, n);
```

---

# Semana 13 — I/O bloqueante y multiplexación

Problema:

```text
Worker
   │
accept()
   │
read()
   │
BLOCKED
```

Si el cliente conecta pero nunca manda datos:

```text
Cliente lento
      │
      ▼
   Worker
      │
   read()
      │
   BLOQUEADO
```

Ese worker no puede atender otro cliente.

---

## Introducir

* blocking I/O;
* nonblocking I/O;
* `select()`;
* `poll()`;
* `epoll()`;
* readiness;
* event-driven I/O.

---

## Arquitectura tradicional

```text
Cliente A ─── Worker A
Cliente B ─── Worker B
Cliente C ─── Worker C
```

---

## Arquitectura multiplexada

```text
Cliente A ─┐
Cliente B ─┼──► epoll ───► Worker
Cliente C ─┘
Cliente D ─┘
```

---

## Milestone opcional avanzado

Cada worker puede manejar múltiples conexiones mediante:

```c
epoll()
```

No es necesario para aprobar el proyecto.

Puede utilizarse como:

```text
bonus
proyecto avanzado
trabajo final sobresaliente
```

---

# MES 4 — Robustez, seguridad y desempeño

# Semana 14 — Administración de recursos

Los estudiantes intentarán romper su propio servidor.

## Conceptos

* file descriptor exhaustion;
* límites de procesos;
* límites de memoria;
* `RLIMIT_NOFILE`;
* `RLIMIT_NPROC`;
* `SIGPIPE`;
* partial reads;
* partial writes;
* errores de system calls.

Observar:

```bash
ulimit -a
```

Descriptores abiertos:

```bash
ls /proc/<PID>/fd
```

Sockets:

```bash
ss -ltnp
```

También puede utilizarse:

```bash
lsof -p <PID>
```

---

# Seguridad básica

Considerar una petición como:

```text
GET /../../../../etc/passwd
```

¿Qué ocurriría si simplemente hacemos?

```c
open(requested_path, O_RDONLY);
```

Podríamos permitir acceso a archivos fuera de:

```text
./www
```

Esto introduce:

* path traversal;
* validación de entrada;
* least privilege;
* separación de privilegios;
* límites de recursos.

---

# Milestone 8 — Robustez

El servidor debe sobrevivir a:

* peticiones inválidas;
* clientes que se desconectan;
* archivos inexistentes;
* conexiones rápidas;
* requests incompletos;
* workers muertos;
* errores de system calls.

---

# Semana 15 — Supervisión de procesos y tolerancia a fallas

Provocar intencionalmente:

```c
abort();
```

dentro de un worker.

Mala arquitectura:

```text
Worker muere
     │
     ▼
capacidad del servidor disminuye
```

Mejor:

```text
Worker muere
     │
     ▼
SIGCHLD
     │
     ▼
Master
     │
 detecta muerte
     │
     ▼
fork()
     │
     ▼
nuevo Worker
```

Diagrama:

```text
                Master
                  │
          ┌───────┼───────┐
          ▼       ▼       ▼
          W1      W2      W3
                  │
                CRASH
                  │
                  ▼
               SIGCHLD
                  │
                  ▼
                Master
                  │
                fork()
                  │
                  ▼
                 W4
```

---

# Milestone 9 — Supervisión

Si se configura:

```bash
./miniserv -w 4
```

deben existir siempre aproximadamente cuatro workers.

Ejemplo:

```bash
kill -9 <worker_pid>
```

Después:

```bash
ps -ef | grep miniserv
```

deberá aparecer un worker nuevo.

---

# Semana 16 — Desempeño e integración final

Comparar tres arquitecturas.

## Arquitectura A

```text
Servidor secuencial
```

## Arquitectura B

```text
fork-per-client
```

## Arquitectura C

```text
prefork worker pool
```

Opcional:

```text
prefork + epoll
```

---

# Métricas

Medir:

* throughput;
* latency;
* CPU;
* memoria;
* número de procesos;
* context switches;
* system calls.

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

Opcionalmente:

```bash
perf
```

si se desea profundizar.

---

# Arquitectura final

```text
                         CLIENTES
                            │
                            ▼
                    Listening Socket
                            │
                            ▼
                    ┌──────────────┐
                    │    MASTER    │
                    │              │
                    │ signals      │
                    │ supervision  │
                    │ workers      │
                    └──────┬───────┘
                           │
             ┌─────────────┼─────────────┐
             │             │             │
             ▼             ▼             ▼
          Worker 1      Worker 2      Worker 3
             │             │             │
             └──────┬──────┴──────┬──────┘
                    │             │
                    ▼             ▼
             Memoria compartida   Pipe
                    │             │
                    ▼             ▼
               Estadísticas      Logger
                                   │
                                   ▼
                              access.log
```

Cada worker:

```text
accept()
   │
read()
   │
parse request
   │
open()
   │
read file
   │
write response
   │
actualizar stats
   │
enviar log
```

---
---

# 6. Formato obligatorio para desarrollar cada semana

Utiliza la información específica de la semana proporcionada al final de este prompt y genera **la sesión completas de 120 minutos**.

El resultado debe contener las siguientes secciones.

---

## A. Visión general de la semana

Incluye:

### Objetivos de aprendizaje

Define entre **4 y 8 objetivos específicos y medibles**.

Utiliza formulaciones como:

- explicar;
- identificar;
- comparar;
- implementar;
- diagnosticar;
- analizar.

---

### Motivación

Presenta primero un problema concreto.

Explica:

- qué problema aparece;
- por qué importa para Sistemas Operativos;
- cómo se manifiesta en MiniServ;
- qué mecanismo del SO permitirá estudiarlo o resolverlo.

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

# 7. Clase  — 120 min


| Actividad           |
| ------------------- |
| Problema motivador  |
| Concepto teórico   |
| Demostración       |
| Ejercicio           |
| Discusión y cierre |

Adapta los tiempos según el tema.

Para cada bloque explica:

- qué debe explicar el profesor;
- qué ejemplos utilizar;
- qué dibujar en el pizarrón;
- qué preguntas hacer a los estudiantes;
- cómo conectarlo con MiniServ.

Debe tener un componente práctico mayor cuando el tema lo permita.

Puede incluir:

- laboratorio;
- programación;
- modificación de MiniServ;
- análisis con herramientas Linux;
- experimentos;
- debugging;
- predicción de comportamiento;
- comparación de resultados.

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

Incluye  **diagramas ASCII técnicamente útiles, únicamente cuando realmente ayuden al entendimiento de un tema**.

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

- ser pequeña;
- poder escribirse o explicarse en clase;
- compilar realmente;
- mostrar principalmente un concepto.

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

- PIDs;
- orden de impresión;
- timing;
- scheduling;
- direcciones;
- resultados de una race condition.

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

- qué debemos observar;
- qué significa la salida;
- qué relación tiene con el concepto estudiado;
- qué debería identificar el alumno.

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

Incluye cuando sea relevante:

### 3 preguntas conceptuales

Para cada una proporciona:

- pregunta;
- respuesta esperada;
- razonamiento.

Las preguntas deben requerir comprensión, no solamente memorización.

---

### 2 ejercicios cortos de programación

Pensados para aproximadamente:

```text
10–25 minutos
```

Incluye:

- enunciado;
- objetivo;
- solución para el profesor;
- resultado esperado.

---

### 1 actividad de predicción

Presenta código o una situación y pregunta:

> ¿Qué creen que ocurrirá antes de ejecutarlo?

Después explica:

- qué partes son deterministas;
- qué partes no;
- por qué.

---

# 16. Errores y misconceptions

Incluye cuando sea relevante **errores frecuentes**.

Por ejemplo:

### "`fork()` crea un thread."

Incorrecto.

Explica:

- por qué;
- qué ocurre realmente;
- cómo demostrarlo.

Incluye tanto:

- errores conceptuales;
- errores de programación;
- malas interpretaciones de resultados de Linux.

---

# 17. Conexión profesional

Explica dónde aparecen estos mecanismos en sistemas reales.

Cuando sea apropiado utiliza ejemplos como:

- Linux;
- Unix;
- Nginx;
- Apache;
- PostgreSQL;
- Redis;
- Docker;
- systemd;
- shells;
- servidores modernos.

La conexión debe ser arquitectónica y pedagógica.

No conviertas esta sección en publicidad o historia empresarial.

---

# 18. Historia y fun facts

Incluye entre  **datos interesantes** cuando realmente aporten contexto.

Pueden relacionarse con:

- Unix;
- POSIX;
- Linux;
- evolución del hardware;
- evolución de arquitecturas de servidores;
- decisiones históricas de diseño.

Agrega contexto geopolítico únicamente cuando ayude genuinamente a comprender la evolución tecnológica.

---

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

- file descriptor;
- system call;
- scheduler;
- context switch;
- race condition;
- shared memory;
- blocking I/O.

Explícalos en español cuando aparezcan por primera vez.

Usa:

- C real;
- Linux;
- APIs POSIX/Linux;
- programas compilables;
- comportamiento técnicamente correcto.

En los ejemplos de C:

- incluye headers;
- verifica errores relevantes;
- usa `perror()` cuando sea apropiado;
- explica valores de retorno importantes;
- evita prácticas inseguras como solución definitiva.

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



DESARROLLA LOS TEMAS DE LA SEMANA 5 en la carpeta semana 4