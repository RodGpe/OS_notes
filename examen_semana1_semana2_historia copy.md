# Examen — Sistemas Operativos (Semanas 1–2 e Historia)

**Instrucciones sugeridas:** responda con precisión. Donde aplique, distinga *user space* / *kernel*, proceso / programa y abstracción / hardware.

---

## Bloque A — Historia de los sistemas operativos (6 preguntas)

**1.** En las primeras computadoras electrónicas (1940–1950), ¿qué tareas realizaban con frecuencia los *humanos* que más adelante asumiría el sistema operativo? Mencione al menos tres.

**2.** Explique la relación entre el **alto costo** de las computadoras centrales y la aparición del **procesamiento por lotes** (*batch processing*). ¿Qué problema económico intentaba resolver?

**3.** ¿Qué problema de utilización del CPU motivó la **multiprogramación**? Describa un escenario concreto en el que un programa está esperando I/O y otro puede ejecutarse.

**4.** La multiprogramación obligó a resolver nuevas cuestiones de diseño. Enumere **cuatro** áreas del temario típico de un curso de SO que surgen directamente de esos problemas (por ejemplo: scheduling, …).

**5.** Contraste **polling** e **interrupciones** desde el punto de vista del CPU cuando un dispositivo de I/O termina una operación. ¿Por qué las interrupciones fueron clave para sistemas más eficientes?

**6.** Según el marco del curso, un sistema operativo cumple dos funciones fundamentales. Enuncielas y dé **un ejemplo** de abstracción moderna (proceso, archivo, socket, memoria virtual, etc.) y la realidad física que oculta.

---

## Bloque B — Semana 1: kernel, procesos y system calls (7 preguntas)

**7.** Cuando un programa C ejecuta `write(fd, buffer, size);`, ¿quién escribe *realmente* los bytes hacia el recurso asociado al descriptor? Trace el camino en no más de cinco pasos: aplicación → biblioteca → frontera user/kernel → kernel → dispositivo o subsistema.

**8.** Diferencie **modo usuario** y **modo kernel**. ¿Por qué un bug en MiniServ en user space raramente tumba todo el sistema, mientras que un bug en un *driver* del kernel puede provocar *kernel panic*?

**9.** Clasifique cada evento como **interrupción**, **excepción** o **trap** (system call), justificando en una línea:

- (a) Temporizador del CPU que expira para el *scheduler*
- (b) División por cero en un programa
- (c) Instrucción `syscall` invocada desde `write()`

**10.** Defina **proceso** y contrástelo con **programa** (binario en disco). Si ejecuta `./miniserv` dos veces seguidas, ¿es el mismo proceso? ¿Por qué los PIDs suelen ser distintos?

**11.** ¿`printf("Hola\n")` es una system call? Explique las capas involucradas hasta llegar a la syscall real que escribe en la terminal.

**12.** Al usar `strace -e trace=write ./demo_write`, ¿qué información aportan los argumentos y el valor de retorno de una línea como `write(1, "...\n", 18) = 18`? ¿Qué error conceptual comete quien cree que `strace` muestra el código fuente línea por línea?

**13.** ¿Quién asigna el **PID** de un proceso y en qué momento del ciclo de vida ocurre? ¿Puede un programa en C “elegir” su propio PID antes de arrancar?

---

## Bloque C — Semana 2: file descriptors, I/O bloqueante y sockets (7 preguntas)

**14.** ¿Qué es un **file descriptor** en Linux? Explique por qué decir “el fd *es* el archivo `/tmp/x`” es incorrecto. ¿Qué representa el entero que devuelve `open()`?

**15.** Al nacer un proceso en Linux, ¿qué fds estándar hereda y a qué suelen estar asociados? ¿Por qué `write(1, …)` y `write(archivo_fd, …)` usan la misma syscall pero pueden llegar a destinos distintos?

**16.** Indique el significado de estos retornos de `read()`:

- (a) valor positivo *n*
- (b) `0`
- (c) `-1` con `errno` configurado

¿Por qué no debe asumirse que una sola llamada a `read()` o `write()` transfiere siempre el `count` completo solicitado?

**17.** Un servidor MiniServ V1 está bloqueado en `accept(listen_fd, …)` sin clientes conectados.

- (a) ¿En qué estado del proceso suele aparecer en `ps` (p. ej. `S`)?
- (b) ¿Consume CPU de forma significativa mientras espera?
- (c) Contraste con `while (1) { }` en términos de scheduling.

**18.** Ordene y describa brevemente el rol de: `socket`, `bind`, `listen`, `accept`, `read`/`write`, `close` en un servidor TCP secuencial. ¿Cuál de estas llamadas suele **bloquear** cuando no hay conexiones pendientes?

**19.** ¿Por qué `accept()` devuelve un **nuevo** file descriptor en lugar de reutilizar `listen_fd`? ¿Qué error comete quien hace `read(listen_fd, buf, n)` esperando datos del cliente?

**20.** Relacione tres ideas del curso con la historia: (i) API de **sockets BSD** (~1983) y el modelo “todo es un archivo”, (ii) I/O **bloqueante** en `accept`/`read` y la **multiprogramación**, (iii) **file descriptors** como abstracción frente al hardware de red. Redacte un párrafo coherente (máximo 8 líneas).

---

### Distribución sugerida para el profesor


| Bloque   | Preguntas | Peso sugerido    |
| -------- | --------- | ---------------- |
| Historia | 1–6      | 30 %             |
| Semana 1 | 7–13     | 35 %             |
| Semana 2 | 14–20    | 35 %<br /><br /> |

**

### Tipos de Sistemas Operativos

1. ¿Cuáles son las cuatro funciones esenciales del kernel en un sistema operativo?
2. ¿Cuál es la diferencia fundamental entre un sistema de tiempo real estricto (Hard RTOS) y uno flexible (Soft RTOS) en términos de consecuencias ante retrasos?
3. ¿Qué características diferencian a un sistema operativo embebido de uno de escritorio tradicional?

### Introducción a la Virtualización

1. ¿Cuál es la diferencia principal en la forma de operar y el rendimiento entre un hipervisor de Tipo 1 (bare-metal) y uno de Tipo 2 (hosted)?
2. ¿Por qué una configuración de arranque dual (dual-boot) no es considerada una forma de virtualización?
3. ¿En qué categoría de hipervisor se clasifica VirtualBox?

### Elementos y Estructuras de los Sistemas Operativos

1. ¿Por qué existe una separación entre el modo usuario (Ring 3) y el modo kernel (Ring 0) en la arquitectura del procesador?
2. ¿Qué ventajas y desventajas en rendimiento y tolerancia a fallos presenta una arquitectura de kernel monolítico en comparación con un microkernel?
3. ¿Qué problema común puede ocurrir en un kernel monolítico si un controlador o componente falla?

### Llamadas al Sistema

1. ¿Qué es una llamada al sistema (syscall) y qué mecanismo utiliza el procesador a bajo nivel para cambiar de espacio de usuario al kernel?
2. ¿Qué valores devuelve la llamada fork() tanto en el proceso padre como en el proceso hijo, y cómo se aprovecha esta diferencia en la lógica del código?
3. ¿Por qué en los sistemas UNIX se acostumbra a utilizar el patrón combinado fork() + execve() en lugar de invocar únicamente execve() directamente desde el proceso original?

### Procesos

1. ¿Qué diferencia formal existe entre un programa y un proceso?
2. En el modelo clásico de 5 estados, ¿por qué un proceso en estado Bloqueado no puede pasar directamente a Ejecutando?
3. ¿Qué es un proceso zombie y un proceso huérfano, y de qué manera el sistema operativo resuelve o limpia la presencia de cada uno?

### Hilos

1. ¿Qué recursos comparte un hilo con los demás hilos del mismo proceso y qué elementos conserva de forma estrictamente privada?
2. En la biblioteca Pthreads, ¿qué función básica se utiliza para crear un nuevo hilo?
3. ¿Cuál es la diferencia conceptual entre concurrencia y paralelismo en la ejecución de hilos o tareas?

**



**

1. Elabora un diagrama donde expliques qué sucede desde que presionas el botón de encendido de una computadora hasta que aparece la pantalla de inicio de sesión de Linux.
2. ¿Qué función tiene la partición EFI?
3. ¿Qué directorio del FILESYSTEM contiene la configuración específica del sistema?
4. ¿Qué herramienta permite consultar la documentación oficial de un comando?
5. ¿Qué syscall esperarías encontrar al abrir un archivo?
6. ¿Qué syscall esperarías encontrar al mostrar texto en pantalla?
7. Menciona tres recursos a los que un proceso no puede acceder directamente.
8. ¿Qué información almacena el PCB?
9. ¿Qué información debe guardar el sistema operativo antes de cambiar de proceso?
10. Un proceso A debe enviar información a un proceso B y recibir una respuesta. El proceso B no es hijo de A. ¿Qué mecanismo IPC utilizarías?
11. Si ejecutas ls | grep ".c" ¿cuantos procesos participan?, ¿Qué proceso escribe en el pipe? ¿Qué proceso lee del pipe?
12. Realiza el árbol de procesos del siguiente código:


#include <stdio.h>
#include <unistd.h>

int main()
{
/* fork a child process */
fork();

/* fork another child process */
fork();

/* and fork another */
fork();

return 0;
}

1. Diseñe un chat entre dos terminales independientes de Linux.

El usuario A escribe mensajes en una terminal y el usuario B los recibe en otra.

¿Podría utilizar un pipe anónimo? Justifique.

¿Cómo lograría una comunicación bidireccional?

Dibuje un diagrama de los procesos y canales de comunicación.

**
