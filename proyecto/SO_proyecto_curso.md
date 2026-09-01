# Sistemas Operativos a través de un Servidor Multiproceso

## 1. Descripción general del curso

**Duración:** 16 semanas / aproximadamente 4 meses
**Horario sugerido:** 2 sesiones de 90 minutos por semana
**Lenguaje principal:** C sobre Linux
**Proyecto central:** Construir progresivamente un servidor TCP/HTTP multiproceso inspirado en arquitecturas reales.

### Prerrequisitos

Se asume que los estudiantes ya conocen:

* programación básica en C;
* apuntadores;
* arreglos;
* estructuras;
* funciones;
* memoria dinámica básica;
* comandos básicos de Linux;
* compilación con `gcc`.

No es necesario que tengan experiencia previa con redes.

Los conceptos de sockets y TCP necesarios para el proyecto se introducen dentro del curso.

---

# 2. Proyecto central del curso

Los estudiantes comienzan con un servidor extremadamente sencillo:

```text
Cliente
   │
   ▼
┌────────────┐
│  Servidor  │
│  proceso   │
└────────────┘
```

El servidor:

1. espera una conexión;
2. acepta un cliente;
3. lee una petición;
4. responde;
5. cierra la conexión;
6. espera al siguiente cliente.

Al terminar el semestre, el sistema tendrá una arquitectura similar a:

```text
                          ┌──────────────────┐
                          │      Cliente     │
                          └────────┬─────────┘
                                   │ TCP
                                   ▼
                        ┌─────────────────────┐
                        │   Proceso Maestro   │
                        │                     │
                        │ listen()            │
                        │ señales             │
                        │ supervisión         │
                        └───────┬─────────────┘
                                │
             ┌──────────────────┼──────────────────┐
             │                  │                  │
             ▼                  ▼                  ▼
       ┌────────────┐     ┌────────────┐     ┌────────────┐
       │ Worker P1  │     │ Worker P2  │     │ Worker P3  │
       │            │     │            │     │            │
       │ accept()   │     │ accept()   │     │ accept()   │
       │ read()     │     │ read()     │     │ read()     │
       │ open()     │     │ open()     │     │ open()     │
       │ write()    │     │ write()    │     │ write()    │
       └──────┬─────┘     └──────┬─────┘     └──────┬─────┘
              │                  │                  │
              └────────────┬─────┴──────────────────┘
                           │
                    Memoria compartida
                           │
              ┌────────────▼──────────────┐
              │ peticiones               │
              │ conexiones activas       │
              │ errores                  │
              │ bytes transferidos       │
              └───────────────────────────┘

                         IPC
                          │
                          ▼
                  ┌────────────────┐
                  │ Proceso Logger │
                  └────────────────┘
```

---

# 3. ¿Qué conceptos de Sistemas Operativos cubre?

El proyecto permite estudiar de manera natural:

* llamadas al sistema;
* modo usuario y modo kernel;
* procesos;
* `fork()`;
* `exec()`;
* `wait()` y `waitpid()`;
* estados de procesos;
* scheduling;
* cambios de contexto;
* procesos zombie;
* señales;
* descriptores de archivo;
* herencia de descriptores;
* pipes;
* sockets;
* IPC;
* memoria compartida;
* exclusión mutua;
* semáforos;
* race conditions;
* memoria virtual;
* copy-on-write;
* page faults;
* sistema de archivos;
* I/O bloqueante;
* I/O no bloqueante;
* multiplexación;
* manejo de recursos;
* tolerancia a fallas;
* seguridad;
* desempeño.

La idea central del curso será:

> Cada nuevo concepto de Sistemas Operativos debe resolver un problema real que aparece en nuestro servidor.

---

# 4. Especificación final del servidor

Al finalizar el curso, el servidor puede ejecutarse de una forma similar a:

```bash
./miniserv -p 8080 -w 4 -r ./www
```

Donde:

```text
-p 8080     puerto TCP
-w 4        número de workers
-r ./www    directorio raíz del servidor
```

Ejemplo:

```bash
curl http://localhost:8080/index.html
```

El servidor deberá ser capaz de servir archivos como:

```text
GET /index.html
GET /image.jpg
GET /style.css
GET /hello.txt
```

No es necesario implementar completamente HTTP/1.1.

Se puede trabajar con un subconjunto simplificado de HTTP.

---

# 5. Objetivos generales de aprendizaje

Al finalizar el curso, el estudiante será capaz de:

1. Explicar la diferencia entre modo usuario y modo kernel.
2. Explicar por qué existen las llamadas al sistema.
3. Utilizar system calls de Linux desde C.
4. Explicar la abstracción de file descriptor de Unix.
5. Crear procesos utilizando `fork()`.
6. Reemplazar imágenes de proceso mediante `exec()`.
7. Esperar procesos hijos utilizando `wait()` y `waitpid()`.
8. Explicar estados de procesos.
9. Explicar conceptos básicos de planificación de CPU.
10. Identificar y solucionar procesos zombie.
11. Utilizar señales.
12. Explicar qué significa una llamada bloqueante.
13. Implementar IPC mediante pipes.
14. Implementar memoria compartida.
15. Identificar condiciones de carrera.
16. Utilizar mecanismos de sincronización.
17. Explicar memoria virtual.
18. Explicar copy-on-write.
19. Trabajar con sistemas de archivos.
20. Diseñar una arquitectura multiproceso.
21. Analizar el costo de crear procesos.
22. Analizar cambios de contexto.
23. Medir desempeño.
24. Utilizar herramientas Linux para observar procesos.
25. Relacionar estos conceptos con servidores reales.

---

# 6. Evolución del proyecto

El servidor tendrá aproximadamente 12 versiones.

```text
V0
Hola Mundo
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
Servidor fork-per-client
    │
    ▼
V4
Servidor con señales
    │
    ▼
V5
Servidor + proceso logger
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
Pool de workers prefork
    │
    ▼
V9
Servidor de archivos
    │
    ▼
V10
Servidor robusto
    │
    ▼
V11
Medición de desempeño
    │
    ▼
V12
Servidor final
```

---

# MES 1 — Llamadas al sistema, procesos y servidor básico

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

# 7. Arquitectura final

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

# 8. Arquitectura del código

```text
miniserv/
│
├── Makefile
│
├── README.md
│
├── include/
│   ├── server.h
│   ├── worker.h
│   ├── http.h
│   ├── logger.h
│   ├── stats.h
│   └── signals.h
│
├── src/
│   ├── main.c
│   ├── server.c
│   ├── worker.c
│   ├── http.c
│   ├── logger.c
│   ├── stats.c
│   └── signals.c
│
├── www/
│   ├── index.html
│   ├── style.css
│   └── image.jpg
│
└── tests/
    ├── basic.sh
    ├── concurrency.sh
    ├── zombies.sh
    └── stress.sh
```

---

# 9. Milestones del proyecto


| Milestone | Semana | Entregable              |
| --------- | -----: | ----------------------- |
| M0        |      1 | Estructura inicial      |
| M1        |      2 | Servidor TCP secuencial |
| M2        |      3 | Fork-per-client         |
| M3        |      5 | Señales + zombies      |
| M4        |      7 | Logger mediante IPC     |
| M5        |      9 | Memoria compartida      |
| M6        |     11 | Prefork worker pool     |
| M7        |     12 | HTTP + archivos         |
| M8        |     14 | Robustez y seguridad    |
| M9        |     15 | Supervisión de workers |
| Final     |     16 | Servidor completo       |

Es recomendable evaluar cada milestone.

No solamente el proyecto final.

---

# 10. Propuesta de evaluación del curso


| Componente                       | Porcentaje |
| -------------------------------- | ---------: |
| Laboratorios                     |        20% |
| Quizzes / preguntas conceptuales |        10% |
| Milestones del proyecto          |        35% |
| Proyecto final                   |        20% |
| Reporte técnico                 |        10% |
| Presentación / defensa          |         5% |

---

# 11. Rúbrica del proyecto final

## Manejo de procesos — 20%

Correcto uso de:

* `fork()`;
* workers;
* `waitpid()`;
* prevención de zombies;
* reinicio de workers.

---

## Manejo de file descriptors — 15%

Correcto manejo de:

* listening socket;
* client sockets;
* archivos;
* pipes;
* `close()`.

No debe haber fugas evidentes de descriptores.

---

## IPC — 15%

Debe existir comunicación entre procesos utilizando:

* pipes;
* shared memory;
* u otro mecanismo autorizado.

---

## Sincronización — 10%

Los datos compartidos deben estar protegidos correctamente.

El estudiante debe explicar:

> ¿Qué race condition existiría si quitamos la sincronización?

---

## Señales — 10%

Manejo adecuado de:

```text
SIGTERM
SIGINT
SIGCHLD
```

Opcional:

```text
SIGHUP
```

---

## Filesystem / HTTP — 10%

Debe servir archivos estáticos correctamente.

---

## Robustez — 10%

Debe manejar:

* requests inválidos;
* desconexiones;
* archivos inexistentes;
* errores;
* workers muertos.

---

## Análisis — 10%

Reporte de desempeño y explicación arquitectónica.

---

# 12. Experimentos obligatorios

## Experimento A — Secuencial vs. concurrente

Comparar:

```text
Sequential
    vs
Fork-per-client
    vs
Prefork
```

Responder:

* ¿Cuál tiene menor latencia?
* ¿Cuál tiene mayor throughput?
* ¿Cuál consume más memoria?
* ¿Cuál crea más procesos?

---

# Experimento B — Cantidad de workers

Probar:

```text
1 worker
2 workers
4 workers
8 workers
16 workers
```

Preguntar:

> ¿Duplicar workers duplica el rendimiento?

La respuesta debería ser:

```text
No necesariamente.
```

Porque aparecen factores como:

* número de CPUs;
* scheduling;
* context switching;
* cache;
* contención;
* I/O.

---

# Experimento C — Memoria

Ejecutar:

```bash
ps -o pid,ppid,rss,vsz,stat,cmd
```

Comparar workers.

Relacionar con:

```text
Virtual Memory
Copy-on-Write
Physical Memory
Page Tables
```

---

# Experimento D — System calls

Ejecutar:

```bash
strace -f ./miniserv -p 8080 -w 4 -r ./www
```

Buscar:

```text
socket
bind
listen
accept
clone
read
write
openat
close
wait
```

El estudiante debe relacionar cada syscall con una parte de la arquitectura del servidor.

---

# 13. Terminología obligatoria

Al terminar el curso, los estudiantes deberían poder definir:

```text
Kernel
User mode
Kernel mode
System call
Trap
Interrupt
Process
PID
PPID
PCB
Process state
Ready queue
Scheduler
Context switch
fork
exec
wait
Zombie
Orphan
File descriptor
Pipe
Socket
IPC
Signal
Race condition
Critical section
Semaphore
Mutex
Deadlock
Virtual memory
Page
Page table
Page fault
Copy-on-write
Shared memory
Filesystem
Inode
Blocking I/O
Nonblocking I/O
Multiplexing
Throughput
Latency
Resource limit
```

---

# 14. Errores conceptuales comunes

## "`fork()` crea un thread"

Incorrecto.

`fork()` crea otro:

```text
proceso
```

---

# "`fork()` ejecuta otro programa"

No necesariamente.

El proceso hijo continúa desde prácticamente el mismo punto del programa.

Para ejecutar otro programa normalmente utilizamos:

```c
exec()
```

---

# "`write()` escribe directamente al hardware"

No.

Normalmente ocurre:

```text
programa
   │
write()
   │
kernel
   │
filesystem/socket/device
```

---

# "Cada cliente usa el listening socket"

Incorrecto.

Normalmente:

```text
Listening socket
     │
   accept()
     │
     ▼
Connected socket
```

El listening socket sigue esperando nuevas conexiones.

---

# "Cuando un hijo termina desaparece inmediatamente"

No necesariamente.

Puede permanecer como:

```text
zombie
```

hasta que el padre recoja su estado.

---

# "`counter++` es atómico"

No debe asumirse.

Puede existir una race condition.

---

# "Los procesos no pueden compartir memoria"

Por defecto sus espacios de direcciones están aislados.

Pero el SO permite crear:

```text
shared memory
```

de forma explícita.

---

# "Más procesos siempre significa más velocidad"

Incorrecto.

Más procesos también significan:

```text
más scheduling
más memoria
más context switches
más estructuras del kernel
más contención
```

---

# "Blocking siempre es malo"

No.

El I/O bloqueante puede ser sencillo y eficiente.

El problema depende de la arquitectura.

---

# 15. Tres preguntas para discutir en clase

## Pregunta 1

Un worker ejecuta:

```c
read(client_fd, buffer, 1024);
```

pero el cliente no manda datos.

¿Qué ocurre?

Respuesta esperada:

```text
RUNNING
   │
   ▼
BLOCKED
   │
llegan datos
   │
   ▼
READY
   │
scheduler
   │
   ▼
RUNNING
```

---

## Pregunta 2

Después de:

```c
int client_fd = accept(...);

fork();
```

¿quién tiene `client_fd`?

Respuesta:

Inicialmente tanto el padre como el hijo tienen un descriptor válido que referencia al socket.

---

## Pregunta 3

¿Por qué:

```text
4 workers
```

pueden rendir mejor que:

```text
1000 workers
```

aunque existan 1000 clientes?

Conceptos esperados:

* context switching;
* scheduling;
* memoria;
* número de CPUs;
* cache;
* contención;
* I/O bloqueante.

---

# 16. Dos ejercicios cortos de programación

# Ejercicio 1 — Crear zombies deliberadamente

Modificar el servidor para que los hijos terminen pero el padre no ejecute `wait()`.

Generar clientes:

```bash
for i in $(seq 1 10); do
    curl http://localhost:8080/ &
done
```

Observar:

```bash
ps -o pid,ppid,state,cmd
```

Buscar:

```text
Z
```

Después corregirlo.

El estudiante debe explicar:

1. qué es un zombie;
2. por qué aparece;
3. qué información conserva el kernel;
4. cómo se elimina.

---

# Ejercicio 2 — Race condition

Crear memoria compartida:

```c
unsigned long counter;
```

Crear cuatro procesos.

Cada uno ejecuta:

```c
for (int i = 0; i < 1000000; i++)
    counter++;
```

Resultado esperado matemáticamente:

```text
4,000,000
```

Ejecutarlo varias veces.

El resultado puede ser:

```text
3,948,272
3,721,381
3,997,182
```

Los valores son no deterministas.

Después proteger la sección crítica mediante un semáforo.

---

# 17. Tarea / proyecto final

# MiniServ

Construir un servidor HTTP multiproceso estilo Unix.

El servidor debe:

1. Crear un socket TCP.
2. Hacer `bind()`.
3. Ejecutar `listen()`.
4. Aceptar conexiones.
5. Servir archivos.
6. Utilizar un número configurable de workers.
7. Utilizar un modelo prefork.
8. Manejar correctamente file descriptors.
9. Manejar `SIGINT`.
10. Manejar `SIGTERM`.
11. Manejar `SIGCHLD`.
12. Reemplazar workers muertos.
13. Mantener estadísticas en memoria compartida.
14. Sincronizar dichas estadísticas.
15. Enviar logs a otro proceso mediante IPC.
16. Terminar de forma ordenada.
17. Evitar zombies.
18. Evitar path traversal.
19. Manejar clientes desconectados.
20. Manejar system calls fallidas.

Debe regresar respuestas razonables como:

```text
200 OK
400 Bad Request
404 Not Found
500 Internal Server Error
```

---

# Reporte final

El estudiante deberá entregar un documento explicando:

```text
1. Arquitectura
2. Modelo de procesos
3. System calls utilizadas
4. Propiedad de file descriptors
5. IPC
6. Memoria compartida
7. Sincronización
8. Manejo de señales
9. Memoria virtual
10. Filesystem
11. Desempeño
12. Problemas encontrados
13. Decisiones arquitectónicas
14. Limitaciones
15. Trabajo futuro
```

---

# 18. Defensa oral

Es recomendable realizar una defensa individual breve.

Ejemplos de preguntas:

> ¿Por qué este proceso hace `close(client_fd)`?

> ¿Qué ocurriría si quitamos este `waitpid()`?

> ¿Qué proceso tiene abierto este pipe?

> ¿Qué sucede si dos workers ejecutan esta línea al mismo tiempo?

> ¿Qué syscall puede bloquear al worker aquí?

> ¿Qué ocurre dentro del SO cuando llamamos `accept()`?

> ¿Por qué cuatro workers funcionaron mejor que uno?

> ¿Por qué 32 workers no fueron mejores que ocho?

> ¿Qué ocurre con la memoria después de `fork()`?

> ¿Qué ocurre si este worker muere?

Esto obliga a que el estudiante comprenda su propia implementación.

---

# 19. Evolución arquitectónica del semestre

## Inicio

```text
Cliente
   │
   ▼
Servidor
   │
 read()
   │
 write()
```

---

## Procesos

```text
              Server
                │
              fork()
          ┌─────┼─────┐
          ▼     ▼     ▼
          P1    P2    P3
```

---

## IPC

```text
Workers ─────────► Logger
```

---

## Shared memory

```text
Workers
   │
   ▼
Shared Memory
```

---

## Synchronization

```text
Workers
   │
Semaphore
   │
   ▼
Shared Memory
```

---

## Supervisión

```text
Master
  │
  ├── Worker
  ├── Worker
  ├── Worker
  └── Logger
```

---

## Sistema final

```text
                  Master
             /      |      \
           W1       W2      W3
            \       |       /
             Shared Memory
                   │
                 Stats

Workers ─── IPC ───► Logger

Workers
   │
syscalls
   │
   ▼
Linux Kernel
   │
   ├── Scheduler
   ├── Networking
   ├── Virtual Memory
   └── Filesystem
```

---

# 20. Mapa conceptual del curso

```text
                    SISTEMAS OPERATIVOS
                           │
       ┌───────────────────┼────────────────────┐
       │                   │                    │
    Procesos             Memoria               I/O
       │                   │                    │
     fork()          Memoria Virtual      File Descriptors
       │                   │                    │
     wait()              Pages                Files
       │                   │                    │
    Signals              COW                 Sockets
       │                   │                    │
   Scheduling        Shared Memory            Pipes
       │                   │                    │
       └──────────── Sincronización ───────────┘
                           │
                           ▼
                        Server
                           │
                           ▼
                    Sistema Multiproceso
```

---

# 21. Conexión profesional

Este proyecto permite relacionar directamente los conceptos clásicos con software real.

Cuando un estudiante escucha hablar de servidores como:

```text
Apache
Nginx
PostgreSQL
Redis
HAProxy
Gunicorn
uWSGI
```

ya puede comenzar a hacerse preguntas como:

```text
¿Cuántos procesos tienen?

¿Usan threads?

¿Usan un worker pool?

¿Quién acepta las conexiones?

¿Cómo manejan señales?

¿Cómo reinician workers?

¿Cómo comparten estado?

¿Cómo manejan miles de file descriptors?

¿Usan blocking I/O?

¿Usan epoll?
```

Ese cambio de perspectiva es muy importante.

El alumno deja de ver Sistemas Operativos como una colección de conceptos abstractos y comienza a verlos como decisiones arquitectónicas reales.

---

# 22. Fun facts que pueden utilizarse durante el curso

### Unix y file descriptors

Una de las ideas más poderosas de Unix fue presentar muchos recursos mediante una interfaz relativamente uniforme:

```text
open
read
write
close
```

Esta filosofía influyó profundamente en Linux y otros sistemas Unix-like.

---

### `fork()` parece extraño para muchos programadores modernos

En muchas plataformas se piensa:

```text
"crear proceso → ejecutar programa"
```

En Unix, conceptualmente:

```text
crear copia del proceso → opcionalmente reemplazarla con exec()
```

De ahí viene el patrón:

```text
fork()
   │
   ▼
exec()
```

que sigue siendo fundamental.

---

### Los zombies no están ejecutándose

Un proceso zombie:

```text
NO está usando CPU
NO está ejecutando instrucciones
```

Es principalmente información de terminación que el kernel conserva para el padre.

---

### El servidor puede convertirse en una herramienta para estudiar todo Linux

Con un solo proyecto podemos inspeccionar:

```text
/proc/<pid>/fd
/proc/<pid>/maps
/proc/<pid>/status
/proc/<pid>/stat
```

y observar directamente muchas de las abstracciones vistas en clase.

---

# 23. Idea pedagógica central

No presentar:

```text
Semana 3: fork()
Semana 5: signals
Semana 8: semáforos
```

como temas independientes.

En cambio:

```text
Tenemos un problema en el servidor.
           │
           ▼
¿Qué mecanismo ofrece el SO?
           │
           ▼
Aprendemos el mecanismo.
           │
           ▼
Lo implementamos.
           │
           ▼
Medimos qué cambió.
```

Por ejemplo:

```text
El servidor atiende un solo cliente
            │
            ▼
Necesitamos concurrencia
            │
            ▼
fork()
```

Después:

```text
Los hijos terminan
       │
       ▼
aparecen zombies
       │
       ▼
wait() + SIGCHLD
```

Después:

```text
Los workers necesitan compartir estadísticas
              │
              ▼
        Shared Memory
              │
              ▼
aparecen race conditions
              │
              ▼
          Semáforos
```

Después:

```text
Crear un proceso por request es costoso
              │
              ▼
          Worker Pool
```

De esta manera, el proyecto se convierte en el hilo conductor del curso completo.
