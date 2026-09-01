# Sistemas Operativos — Arquitectura del SO y llamadas al sistema

**Duración:** 90 minutos
**Nivel:** Licenciatura — Ciencias de la Computación
**Prerrequisitos:** C básico, terminal Linux, compilación con `gcc`
**Tema central:** Modo usuario/kernel, interrupciones, excepciones, traps, system calls y API POSIX.

---

# 1. Objetivos de aprendizaje

Al terminar la clase, el estudiante podrá:

1. Explicar por qué un sistema operativo separa la ejecución entre **modo usuario** y **modo kernel**.
2. Describir el ciclo básico:
   **fetch → decode → execute**
   y relacionarlo con la ejecución de procesos y la intervención del sistema operativo.
3. Explicar el modelo teórico **Random Access Machine (RAM)** y sus limitaciones frente a una computadora real.
4. Diferenciar:
   * interrupción,
   * excepción,
   * trap,
   * system call.
5. Explicar la diferencia entre:
   * API del sistema operativo,
   * función wrapper de biblioteca,
   * llamada al sistema.
6. Describir qué ocurre cuando un programa cruza el límite **user space → kernel space**.
7. Utilizar llamadas POSIX como:
   * `read()`
   * `write()`
   * `open()`
   * `close()`
8. Utilizar `strace` para observar las llamadas al sistema realizadas por un proceso.
9. Explicar qué significa que una llamada sea **blocking**.
10. Relacionar el costo de las llamadas al sistema y la I/O síncrona con problemas reales de rendimiento.

---

# 2. Plan de clase — 90 minutos


| Tiempo     | Tema                                                        |
| ---------- | ----------------------------------------------------------- |
| 0–8 min   | Motivación: ¿por qué necesitamos system calls?           |
| 8–20 min  | Arquitectura del SO: modo usuario/kernel, fetch cycle y RAM |
| 20–32 min | Interrupciones, excepciones y traps                         |
| 32–45 min | API → libc wrapper → system call → kernel                |
| 45–57 min | Práctica 1:`write()`y`read()`                              |
| 57–70 min | Práctica 2:`open()`,`read()`,`write()`,`close()`+`strace`  |
| 70–80 min | Blocking calls, costo user/kernel y rendimiento de I/O      |
| 80–87 min | Ejercicios y discusión                                     |
| 87–90 min | Resumen + tarea                                             |

---

# 3. Motivación

Comienza con una pregunta:

> Si un programa en C quiere leer un archivo del disco, ¿por qué no accede directamente al SSD?

Una respuesta posible sería:

> Porque permitir que cualquier programa controle directamente el hardware destruiría el aislamiento y la seguridad del sistema.

Un programa normal no puede decidir:

```text
"Voy a leer directamente el sector 812934 del SSD."
```

Tampoco debería poder:

```text
"Voy a modificar la memoria perteneciente a Firefox."
```

o:

```text
"Voy a desactivar las interrupciones del CPU."
```

El sistema operativo funciona como intermediario.

Un programa dice:

```text
Quiero abrir este archivo.
Quiero escribir estos bytes.
Quiero crear otro proceso.
Quiero solicitar memoria.
```

El kernel decide:

```text
¿Está permitido?

¿Qué recurso corresponde?

¿Está disponible?

¿Tengo que esperar?

¿Qué driver debe utilizarse?
```

Esta separación es una de las ideas fundamentales de los sistemas operativos modernos.

---

# 4. El CPU y el ciclo Fetch–Decode–Execute

Antes de hablar del kernel, recordemos qué hace fundamentalmente un procesador.

En una simplificación:

```text
          ┌─────────────┐
          │ Program     │
          │ Counter PC  │
          └──────┬──────┘
                 │
                 ▼
          ┌─────────────┐
          │    FETCH    │
          │ obtener     │
          │ instrucción │
          └──────┬──────┘
                 │
                 ▼
          ┌─────────────┐
          │   DECODE    │
          │ interpretar │
          └──────┬──────┘
                 │
                 ▼
          ┌─────────────┐
          │   EXECUTE   │
          │ ejecutar    │
          └──────┬──────┘
                 │
                 └──────────► siguiente instrucción
```

Por ejemplo:

```asm
mov eax, 5
add eax, 3
```

Conceptualmente el CPU:

1. Lee la instrucción apuntada por el Program Counter.
2. La decodifica.
3. Ejecuta la operación.
4. Actualiza el Program Counter.
5. Continúa con la siguiente instrucción.

Mientras un proceso está ejecutándose, el CPU sigue realizando este ciclo millones o miles de millones de veces por segundo.

Pero existe un problema importante:

> ¿Cómo puede intervenir el sistema operativo si el CPU solamente sigue ejecutando instrucciones del programa?

Aquí aparecen:

* interrupciones,
* excepciones,
* traps.

---

# 5. Modelo teórico Random Access Machine

En teoría de computación y análisis de algoritmos suele utilizarse el modelo **Random Access Machine**, o RAM.

La idea simplificada es una computadora con:

* una CPU,
* registros,
* memoria direccionable,
* instrucciones,
* un Program Counter.

Asumimos que operaciones como:

```text
LOAD
STORE
ADD
SUB
JUMP
```

tienen un costo aproximadamente constante.

También se suele asumir que acceder a:

```text
memoria[10]
```

cuesta esencialmente lo mismo que:

```text
memoria[10 000 000]
```

De ahí el nombre:

**Random Access Machine.**

Este modelo es extremadamente útil para estudiar algoritmos.

Pero una computadora real es mucho más complicada.

```text
CPU
 │
 ├── registros
 │
 ├── cache L1
 │
 ├── cache L2
 │
 ├── cache L3
 │
 └── RAM
      │
      └── almacenamiento
```

Acceder a cada nivel tiene costos muy diferentes.

Además existen:

* TLBs,
* page faults,
* dispositivos,
* buses,
* interrupciones,
* DMA,
* caches,
* memoria virtual.

### Idea importante

El modelo RAM nos permite pensar:

> "Un programa ejecuta una secuencia de instrucciones sobre memoria."

El sistema operativo agrega otra dimensión:

> "Ese programa no controla libremente todos los recursos físicos."

---

# 6. Modo usuario vs. modo kernel

Los CPUs modernos tienen diferentes **niveles de privilegio**.

Para introducir el concepto podemos simplificarlo a dos:

```text
┌───────────────────────────────┐
│          USER SPACE           │
│                               │
│ Firefox                       │
│ gcc                           │
│ bash                          │
│ nuestro programa en C         │
│                               │
│ privilegios limitados         │
└───────────────┬───────────────┘
                │
                │ system call
                ▼
═══════════════════════════════════
      frontera de privilegios
═══════════════════════════════════
                │
                ▼
┌───────────────────────────────┐
│         KERNEL SPACE          │
│                               │
│ scheduler                     │
│ virtual memory                │
│ filesystem                    │
│ networking                    │
│ device drivers                │
│                               │
│ privilegios elevados          │
└───────────────┬───────────────┘
                │
                ▼
             Hardware
```

## Modo usuario

Un programa ejecutándose en modo usuario tiene restricciones.

Normalmente no puede:

* ejecutar instrucciones privilegiadas;
* modificar directamente las tablas de páginas;
* controlar arbitrariamente hardware;
* acceder a memoria del kernel;
* deshabilitar interrupciones;
* modificar memoria de otros procesos.

## Modo kernel

El kernel puede realizar operaciones privilegiadas.

Por ejemplo:

* configurar memoria virtual;
* manejar dispositivos;
* programar timers;
* administrar procesos;
* responder interrupciones;
* cambiar el proceso que está ejecutándose.

---

# 7. ¿Quién activa el modo kernel?

Existen varias formas.

Una clasificación útil es:

```text
                   Entrada al kernel
                         │
          ┌──────────────┼──────────────┐
          │              │              │
          ▼              ▼              ▼
   Interrupción      Excepción        Trap /
    hardware          CPU          system call
```

La terminología exacta cambia ligeramente dependiendo de la arquitectura y del libro utilizado, pero conceptualmente esta clasificación es útil.

---

# 8. Interrupciones

Una **interrupción** suele originarse fuera de la instrucción que está ejecutando actualmente el CPU.

Ejemplo:

```text
CPU ejecutando programa
        │
        │
        │        teclado
        │           │
        │           └──── interrupt
        ▼
────────────────────────────────
        CPU entra al kernel
────────────────────────────────
        │
        ▼
  interrupt handler
```

Algunos dispositivos que pueden generar interrupciones son:

* teclado,
* tarjeta de red,
* almacenamiento,
* timers,
* controladores USB.

## Ejemplo conceptual

El CPU está ejecutando:

```text
instrucción 100
instrucción 101
instrucción 102
```

Llega una interrupción del timer.

El CPU guarda información necesaria para poder regresar y ejecuta código del kernel.

Después puede:

```text
volver al mismo proceso
```

o incluso:

```text
ejecutar otro proceso
```

Esto permite, entre otras cosas, implementar multitarea preemptiva.

---

# 9. Excepciones

Una **excepción** ocurre como consecuencia directa de una instrucción ejecutada por el programa.

Ejemplos:

```text
división entre cero
```

```text
acceso inválido a memoria
```

```text
instrucción inválida
```

Ejemplo:

```c
int *p = NULL;
*p = 10;
```

El CPU intenta acceder a una dirección que no está correctamente mapeada.

Se genera una excepción.

El kernel recibe el control.

En Linux, eventualmente el proceso puede recibir:

```text
SIGSEGV
```

---

# 10. Traps

Un **trap** suele describirse como una transferencia síncrona e intencional de control.

El ejemplo clásico:

> una llamada al sistema.

El programa deliberadamente solicita que el CPU entre al kernel.

En x86-64 Linux normalmente se utiliza una instrucción especializada:

```asm
syscall
```

Conceptualmente:

```text
programa
   │
   │ syscall
   ▼
CPU cambia a modo kernel
   │
   ▼
kernel identifica syscall
   │
   ▼
ejecuta servicio
   │
   ▼
regresa al programa
```

---

# 11. Tres conceptos que NO son lo mismo

Una confusión frecuente es decir:

> "`write()` es una syscall."

Es una simplificación razonable durante conversaciones informales, pero técnicamente conviene distinguir tres niveles.

## Nivel 1 — API

POSIX define una interfaz como:

```c
ssize_t write(int fd, const void *buf, size_t count);
```

Esto describe cómo un programa solicita una operación.

---

# 12. Nivel 2 — Wrapper de biblioteca

Normalmente nuestro programa está enlazado con una biblioteca como:

```text
glibc
```

Cuando escribimos:

```c
write(fd, buffer, n);
```

normalmente estamos llamando primero a una función wrapper proporcionada por la biblioteca C.

---

# 13. Nivel 3 — System call

La biblioteca finalmente realiza la transición al kernel mediante el mecanismo apropiado de la arquitectura.

Conceptualmente:

```text
Programa C
    │
    ▼
POSIX API
    │
    ▼
glibc write()
    │
    ▼
syscall instruction
    │
════════════════════════
      KERNEL
════════════════════════
    │
    ▼
sys_write
    │
    ▼
VFS / driver / terminal
```

Éste es uno de los diagramas más importantes de la clase:

```text
┌─────────────────────────────┐
│           programa          │
│                             │
│ write(1, "Hola\n", 5);      │
└──────────────┬──────────────┘
               │
               ▼
┌─────────────────────────────┐
│           libc              │
│       wrapper write()       │
└──────────────┬──────────────┘
               │
               │ syscall
               ▼
════════════════════════════════
      USER MODE → KERNEL MODE
════════════════════════════════
               │
               ▼
┌─────────────────────────────┐
│        Linux kernel         │
│                             │
│ syscall dispatcher          │
│ filesystem / terminal       │
│ drivers                     │
└──────────────┬──────────────┘
               │
               ▼
            hardware
```

---

# 14. POSIX

**POSIX** significa:

**Portable Operating System Interface**

Es una familia de estándares que define interfaces para sistemas tipo Unix.

Entre otras cosas proporciona interfaces relacionadas con:

```c
open()
close()
read()
write()
fork()
exec()
wait()
pthread_create()
```

POSIX permite que muchos programas escritos para sistemas Unix-like puedan portarse entre:

* Linux,
* macOS,
* BSD,
* otros sistemas compatibles.

### Importante

POSIX no es Linux.

Linux implementa una gran cantidad de interfaces POSIX, pero también tiene funcionalidades propias.

---

# 15. File descriptors

Antes de utilizar `read()` y `write()`, necesitamos recordar los **file descriptors**.

Un file descriptor es un número entero que identifica un recurso abierto para un proceso.

Los primeros tres normalmente son:

```text
0 → stdin
1 → stdout
2 → stderr
```

Por eso:

```c
write(1, "Hola\n", 5);
```

significa aproximadamente:

> escribir 5 bytes en stdout.

---

# 16. Programa 1 — escribir directamente con `write()`

Archivo:

```text
write_demo.c
```

```c
#include <unistd.h>

int main(void)
{
    const char msg[] = "Hola desde write()\n";

    write(STDOUT_FILENO, msg, sizeof(msg) - 1);

    return 0;
}
```

## Headers

```c
#include <unistd.h>
```

`unistd.h` declara:

```c
write()
read()
close()
```

y constantes como:

```c
STDIN_FILENO
STDOUT_FILENO
STDERR_FILENO
```

---

# 17. Compilación

```bash
gcc -Wall -Wextra write_demo.c -o write_demo
```

Ejecución:

```bash
./write_demo
```

Salida esperada:

```text
Hola desde write()
```

No existe nondeterminismo significativo en este ejemplo.

---

# 18. ¿Qué ocurre paso por paso?

Cuando ejecutamos:

```bash
./write_demo
```

ocurren muchas más cosas de las que vemos.

Simplificando:

### Paso 1

El shell solicita al kernel ejecutar:

```text
./write_demo
```

### Paso 2

Linux crea/prepara el nuevo proceso y carga el ejecutable.

### Paso 3

Nuestro código llega a:

```c
write(STDOUT_FILENO, msg, sizeof(msg) - 1);
```

`STDOUT_FILENO` vale normalmente:

```text
1
```

### Paso 4

La función wrapper de libc prepara los argumentos necesarios.

Conceptualmente:

```text
fd = 1
buffer = dirección de "Hola desde write()\n"
count = 19
```

### Paso 5

Se ejecuta el mecanismo de syscall.

El CPU entra en modo kernel.

### Paso 6

El kernel revisa qué representa el descriptor:

```text
1
```

Probablemente está conectado a una terminal.

### Paso 7

El kernel envía los datos hacia el subsistema correspondiente.

### Paso 8

La llamada termina.

El CPU regresa a modo usuario.

### Paso 9

El programa continúa:

```c
return 0;
```

Finalmente termina el proceso.

---

# 19. Observarlo con `strace`

Ejecutar:

```bash
strace ./write_demo
```

Habrá muchas llamadas porque el cargador dinámico y libc realizan operaciones antes de nuestro `main()`.

Busca una línea similar a:

```text
write(1, "Hola desde write()\n", 19) = 19
```

Esto significa:

```text
write(
    fd = 1,
    buffer = "Hola desde write()\n",
    count = 19
)
```

y:

```text
= 19
```

significa que se escribieron 19 bytes.

Una forma mucho más limpia:

```bash
strace -e trace=write ./write_demo
```

Salida aproximada:

```text
write(1, "Hola desde write()\n", 19Hola desde write()
) = 19
+++ exited with 0 +++
```

El formato exacto puede cambiar según la versión de `strace`, terminal y sistema.

---

# 20. Programa 2 — `read()`

Ahora escribiremos un programa mínimo que lea desde teclado.

```c
#include <unistd.h>

int main(void)
{
    char buffer[100];

    ssize_t n = read(STDIN_FILENO, buffer, sizeof(buffer));

    if (n > 0) {
        write(STDOUT_FILENO, buffer, n);
    }

    return 0;
}
```

Guardar como:

```text
read_demo.c
```

---

# 21. Compilar

```bash
gcc -Wall -Wextra read_demo.c -o read_demo
```

Ejecutar:

```bash
./read_demo
```

Escribir:

```text
hola
```

y presionar Enter.

Salida aproximada:

```text
hola
hola
```

La primera línea corresponde a lo mostrado por la terminal mientras escribes.

La segunda corresponde al `write()` de nuestro programa.

---

# 22. ¿Qué ocurre paso por paso?

### Paso 1

El proceso ejecuta:

```c
read(STDIN_FILENO, buffer, sizeof(buffer));
```

### Paso 2

`STDIN_FILENO` corresponde normalmente al descriptor:

```text
0
```

### Paso 3

El programa pide:

> Dame hasta 100 bytes provenientes de stdin.

### Paso 4

Si todavía no escribiste nada, normalmente no hay datos disponibles.

Entonces:

```text
read()
```

puede bloquearse.

El proceso deja de avanzar.

### Paso 5

Mientras espera, el CPU no tiene por qué permanecer desperdiciando ciclos ejecutando ese proceso.

El scheduler puede ejecutar otro proceso.

### Paso 6

Escribes:

```text
hola
```

y presionas Enter.

La terminal proporciona los datos.

### Paso 7

El kernel despierta al proceso.

### Paso 8

`read()` copia bytes hacia:

```c
buffer
```

y devuelve la cantidad recibida.

Por ejemplo:

```text
5
```

correspondientes a:

```text
h o l a \n
```

### Paso 9

Nuestro programa ejecuta:

```c
write(STDOUT_FILENO, buffer, n);
```

y vuelve a mostrar esos bytes.

---

# 23. Observar `read()` y `write()`

```bash
strace -e trace=read,write ./read_demo
```

Después escribe:

```text
linux
```

Podrías observar algo parecido a:

```text
read(0, "linux\n", 100) = 6
write(1, "linux\n", 6) = 6
```

El contenido y número de bytes dependen de lo que introduzca el usuario, por lo que esta ejecución sí contiene entrada variable.

---

# 24. ¿Qué significa blocking call?

Una función **blocking** es aquella cuya ejecución puede provocar que el proceso tenga que esperar.

Por ejemplo:

```c
read(fd, buffer, 100);
```

Si `fd` representa:

* teclado,
* socket,
* pipe,

puede no haber información disponible todavía.

El proceso puede quedar:

```text
RUNNING
   │
   │ read()
   ▼
WAITING / SLEEPING
   │
   │ datos disponibles
   ▼
RUNNABLE
   │
   ▼
RUNNING
```

### Blocking NO significa

```text
todo Linux se congela
```

Significa que:

> ese thread de ejecución no puede continuar hasta que ocurra algo.

---

# 25. Programa 3 — `open()`, `read()`, `write()` y `close()`

Crear primero:

```bash
echo "Sistemas Operativos" > mensaje.txt
```

Programa:

```c
#include <fcntl.h>
#include <unistd.h>

int main(void)
{
    char buffer[128];

    int fd = open("mensaje.txt", O_RDONLY);

    if (fd == -1) {
        return 1;
    }

    ssize_t n = read(fd, buffer, sizeof(buffer));

    if (n > 0) {
        write(STDOUT_FILENO, buffer, n);
    }

    close(fd);

    return 0;
}
```

Guardar como:

```text
file_demo.c
```

---

# 26. Headers utilizados

```c
#include <fcntl.h>
```

Necesario para:

```c
open()
O_RDONLY
```

Y:

```c
#include <unistd.h>
```

para:

```c
read()
write()
close()
```

---

# 27. Compilar

```bash
gcc -Wall -Wextra file_demo.c -o file_demo
```

Ejecutar:

```bash
./file_demo
```

Salida:

```text
Sistemas Operativos
```

---

# 28. ¿Qué ocurre paso por paso?

## Paso 1 — open

Se ejecuta:

```c
int fd = open("mensaje.txt", O_RDONLY);
```

Nuestro proceso solicita:

> Abre `mensaje.txt` solamente para lectura.

El kernel:

1. interpreta la ruta;
2. consulta el filesystem;
3. verifica permisos;
4. localiza el archivo;
5. crea estructuras internas;
6. asigna un descriptor al proceso.

Podría devolver:

```text
3
```

¿Por qué 3?

Porque normalmente ya existen:

```text
0 stdin
1 stdout
2 stderr
```

El siguiente descriptor disponible suele ser 3.

No debemos asumir que siempre será 3.

---

# 29. Paso 2 — read

```c
ssize_t n = read(fd, buffer, sizeof(buffer));
```

Supongamos:

```text
fd = 3
```

El programa pide hasta 128 bytes.

El kernel obtiene los datos.

Es importante comprender que esto **no necesariamente significa que el disco físico sea leído en ese momento**.

Linux utiliza page cache.

Los datos podrían ya estar en RAM.

---

# 30. Paso 3 — write

```c
write(STDOUT_FILENO, buffer, n);
```

Los bytes obtenidos se escriben en stdout.

---

# 31. Paso 4 — close

```c
close(fd);
```

El proceso informa al kernel:

> Ya no necesito este file descriptor.

El kernel puede liberar la referencia correspondiente.

---

# 32. Inspeccionar con `strace`

```bash
strace -e trace=openat,read,write,close ./file_demo
```

En Linux moderno probablemente observarás `openat()` en lugar de `open()`.

Algo parecido:

```text
openat(AT_FDCWD, "mensaje.txt", O_RDONLY) = 3
read(3, "Sistemas Operativos\n", 128) = 19
write(1, "Sistemas Operativos\n", 19) = 19
close(3) = 0
```

Esto introduce una observación importante:

> La función que escribimos en C y la syscall observada no siempre tienen exactamente el mismo nombre.

glibc puede implementar una interfaz POSIX utilizando otra syscall equivalente.

---

# 33. Pregunta para el grupo

El programa contiene:

```c
open("mensaje.txt", O_RDONLY);
```

pero `strace` muestra:

```text
openat(...)
```

Pregunta:

> ¿Significa esto que `strace` está equivocado?

Respuesta:

No.

Esto demuestra precisamente la separación entre:

```text
API
```

y:

```text
implementación mediante syscalls
```

---

# 34. Ver los file descriptors usando `/proc`

Ejecuta otra terminal.

Podemos inspeccionar nuestros propios descriptores:

```bash
ls -l /proc/$$/fd
```

`$$` contiene el PID del shell actual.

Podrías observar:

```text
0 -> /dev/pts/0
1 -> /dev/pts/0
2 -> /dev/pts/0
```

Esto demuestra que:

```text
stdin
stdout
stderr
```

también son descriptores.

---

# 35. `/proc/<PID>/status`

También podemos observar información de un proceso:

```bash
cat /proc/$$/status
```

Algunos campos interesantes:

```text
Name:
Pid:
PPid:
State:
Threads:
VmSize:
VmRSS:
```

Buscar solamente algunos:

```bash
grep -E 'Name|Pid|PPid|State|Threads' /proc/$$/status
```

---

# 36. Observar syscalls de un programa existente

Por ejemplo:

```bash
strace ls
```

La cantidad de operaciones puede resultar sorprendente.

Una versión resumida:

```bash
strace -c ls
```

Ejemplo conceptual:

```text
% time     seconds  usecs/call     calls syscall
------ ----------- ----------- --------- --------
 30.00    ...            ...        ... openat
 20.00    ...            ...        ... close
 15.00    ...            ...        ... newfstatat
 ...
```

Los resultados exactos serán diferentes dependiendo de:

* versión de Linux;
* filesystem;
* libc;
* versión de `ls`;
* archivos del directorio;
* estado de caches;
* arquitectura.

Por lo tanto, el conteo y tiempos son **nondeterministas entre diferentes ejecuciones y máquinas**.

---

# 37. ¿Cuánto cuesta una system call?

Supongamos este código:

```c
int x = a + b;
```

La operación ocurre completamente en modo usuario.

Ahora:

```c
write(fd, buffer, n);
```

requiere una interacción con el kernel.

Conceptualmente:

```text
user mode
   │
   │ preparar argumentos
   │
   ▼
syscall
   │
   ▼
kernel mode
   │
   │ validar parámetros
   │ localizar objeto
   │ permisos
   │ filesystem
   │ driver / buffers
   │ ...
   ▼
return
   │
   ▼
user mode
```

Cruzar esta frontera tiene un costo.

No porque el kernel sea "lento", sino porque hay más trabajo que ejecutar una instrucción ordinaria.

Puede implicar:

* cambio de privilegio;
* ejecución del syscall dispatcher;
* validación de argumentos;
* acceso a estructuras del kernel;
* copias o mapeos de datos;
* sincronización;
* interacción con dispositivos;
* posible bloqueo;
* intervención del scheduler.

---

# 38. No confundir syscall con context switch

Este punto es muy importante.

Una syscall produce:

```text
user mode → kernel mode → user mode
```

pero **no necesariamente cambia de proceso**.

Es posible:

```text
Proceso A user mode
       │
       ▼
Proceso A kernel mode
       │
       ▼
Proceso A user mode
```

No hubo cambio al proceso B.

Un **context switch**, en cambio, sería algo como:

```text
Proceso A
   │
   ▼
kernel
   │
   ▼
Proceso B
```

Una syscall puede provocar un context switch si se bloquea, pero no es obligatorio.

---

# 39. I/O síncrona masiva como cuello de botella

Supongamos que queremos escribir:

```text
1 MB
```

Una forma terrible sería realizar una syscall por byte:

```c
for (...) {
    write(fd, &buffer[i], 1);
}
```

Eso podría producir:

```text
1 000 000 bytes
≈
1 000 000 system calls
```

Mientras que:

```c
write(fd, buffer, 1000000);
```

solicita la misma cantidad de datos utilizando muchas menos transiciones.

Conceptualmente:

```text
write 1 byte
kernel
return

write 1 byte
kernel
return

write 1 byte
kernel
return

...
```

frente a:

```text
write 1 MB
      │
      ▼
    kernel
      │
      ▼
    return
```

---

# 40. Conexión profesional

Esta idea aparece constantemente en sistemas reales.

## Servidores web

Un servidor que realiza muchas operaciones bloqueantes puede desperdiciar threads esperando:

```text
disco
red
base de datos
```

## Bases de datos

Los sistemas intentan agrupar escrituras y minimizar I/O innecesaria.

## Logging

Código como:

```text
write(log, mensaje, ...);
write(log, mensaje, ...);
write(log, mensaje, ...);
```

en rutas extremadamente calientes puede producir overhead considerable.

Por ello existen:

* buffering;
* batching;
* asynchronous I/O;
* event loops;
* `epoll`;
* `io_uring`.

No estudiaremos estos mecanismos todavía, pero todos nacen de un problema que ya podemos entender:

> Esperar I/O y cruzar constantemente al kernel tiene costo.

---

# 41. Comparación con `printf()`

Pregunta:

> ¿`printf()` es una syscall?

No.

`printf()` es una función de biblioteca.

Por ejemplo:

```c
printf("Hola");
printf(" mundo");
printf("\n");
```

no necesariamente genera tres syscalls `write()`.

La biblioteca C puede mantener un buffer y posteriormente realizar:

```text
write(...)
```

con varios datos juntos.

Podemos observarlo.

Crear:

```c
#include <stdio.h>

int main(void)
{
    printf("Hola ");
    printf("Sistemas ");
    printf("Operativos\n");

    return 0;
}
```

Compilar:

```bash
gcc -Wall -Wextra printf_demo.c -o printf_demo
```

Inspeccionar:

```bash
strace -e trace=write ./printf_demo
```

Es muy probable observar aproximadamente:

```text
write(1, "Hola Sistemas Operativos\n", 25) = 25
```

en lugar de tres llamadas diferentes.

Esto demuestra la utilidad de:

```text
buffering
```

---

# 42. API vs syscall — resumen visual

```text
Código del programador
        │
        ▼
printf()
        │
        │ biblioteca stdio
        ▼
buffer
        │
        ▼
write()
        │
        │ libc
        ▼
system call
        │
══════════════════════════
        KERNEL
══════════════════════════
        │
        ▼
terminal / archivo / pipe
```

---

# 43. ¿Podemos invocar una syscall más directamente?

Linux también proporciona:

```c
syscall()
```

Ejemplo educativo:

```c
#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>

int main(void)
{
    const char msg[] = "Syscall directa\n";

    syscall(SYS_write,
            STDOUT_FILENO,
            msg,
            sizeof(msg) - 1);

    return 0;
}
```

Compilar:

```bash
gcc -Wall -Wextra direct_syscall.c -o direct_syscall
```

Ejecutar:

```bash
./direct_syscall
```

Salida:

```text
Syscall directa
```

Inspeccionar:

```bash
strace -e trace=write ./direct_syscall
```

Veremos igualmente algo como:

```text
write(1, "Syscall directa\n", 16) = 16
```

### Mensaje importante

En programas normales:

> Preferimos utilizar las APIs y wrappers estándar.

No existe ventaja general en sustituir:

```c
write()
```

por:

```c
syscall(SYS_write, ...)
```

El ejemplo solamente sirve para revelar las capas.

---

# 44. Errores de las system calls

Las llamadas pueden fallar.

Por ejemplo:

```c
int fd = open("no_existe.txt", O_RDONLY);
```

puede devolver:

```text
-1
```

y establecer:

```c
errno
```

Podemos escribir:

```c
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    int fd = open("no_existe.txt", O_RDONLY);

    if (fd == -1) {
        perror("open");
        return 1;
    }

    close(fd);

    return 0;
}
```

Compilar:

```bash
gcc -Wall -Wextra error_demo.c -o error_demo
```

Ejecutar:

```bash
./error_demo
```

Salida aproximada:

```text
open: No such file or directory
```

Y con:

```bash
strace -e trace=openat ./error_demo
```

podremos observar algo semejante a:

```text
openat(..., "no_existe.txt", O_RDONLY) = -1 ENOENT
```

---

# 45. Interrupciones y system calls juntas

Es útil conectar todo.

Supongamos:

```c
read(0, buffer, 100);
```

y el usuario todavía no ha escrito nada.

```text
Programa
   │
   │ read()
   ▼
Kernel
   │
   │ no hay datos
   ▼
Proceso bloqueado
```

El scheduler puede ejecutar otro proceso.

Después:

```text
usuario presiona tecla
          │
          ▼
     dispositivo
          │
          ▼
      INTERRUPT
          │
          ▼
        kernel
          │
          ▼
datos disponibles
          │
          ▼
despertar proceso
```

Después el proceso puede continuar.

Aquí tenemos en el mismo ejemplo:

* system call,
* bloqueo,
* scheduler,
* interrupción hardware,
* regreso al proceso.

---

# 46. Tres preguntas para los estudiantes

## Pregunta 1

Tenemos:

```c
int x = 10 + 20;
```

y:

```c
write(1, "hola", 4);
```

¿Cuál necesita entrar al kernel?

### Respuesta esperada

```text
write()
```

La suma puede ejecutarse completamente en modo usuario.

---

## Pregunta 2

Un programa llama:

```c
read(fd, buffer, 100);
```

pero todavía no existen datos.

¿Qué debería hacer el sistema operativo?

Opciones:

A. Apagar el CPU.
B. Ejecutar constantemente `read()` hasta que haya datos.
C. Bloquear el proceso y permitir que otro proceso utilice el CPU.
D. Terminar el programa.

### Respuesta esperada

```text
C
```

---

## Pregunta 3

Ejecutamos:

```c
open("archivo.txt", O_RDONLY);
```

y obtenemos:

```text
fd = 5
```

¿Qué significa el número 5?

### Respuesta esperada

No es:

* dirección física;
* número de sector;
* posición del archivo.

Es un identificador dentro de la tabla de file descriptors del proceso.

---

# 47. Ejercicio en clase 1 — mini `cat`

Implementar:

```text
mini_cat archivo.txt
```

que:

1. abra el archivo con `open()`;
2. lea bloques de máximo 128 bytes con `read()`;
3. los escriba a stdout utilizando `write()`;
4. cierre el archivo.

No utilizar:

```c
printf()
fopen()
fread()
```

Compilar:

```bash
gcc -Wall -Wextra mini_cat.c -o mini_cat
```

Ejecutar:

```bash
./mini_cat mensaje.txt
```

Comparar con:

```bash
cat mensaje.txt
```

Finalmente:

```bash
strace -e trace=openat,read,write,close ./mini_cat mensaje.txt
```

### Punto que debe descubrir el alumno

No puede asumir:

```c
read(fd, buffer, sizeof(buffer))
```

que siempre devolverá exactamente:

```text
sizeof(buffer)
```

bytes.

Debe utilizar el valor retornado por `read()`.

---

# 48. Ejercicio en clase 2 — contar syscalls

Escribir dos programas.

### Programa A

Realizar:

```c
write(STDOUT_FILENO, "A", 1);
```

100 veces.

### Programa B

Construir:

```text
AAAAAAAAAAAAAAAA....
```

en un buffer de 100 caracteres y realizar un solo:

```c
write()
```

Después comparar:

```bash
strace -c ./programa_a
```

contra:

```bash
strace -c ./programa_b
```

Pregunta:

> ¿Cuál realiza más llamadas al sistema?

Respuesta esperada:

Programa A.

El tiempo exacto observado puede variar entre ejecuciones, por lo que el timing es nondeterminista y no debe interpretarse como una medición rigurosa con una única ejecución.

La diferencia importante es el **número de llamadas**.

---

# 49. Errores y misconceptions frecuentes

## Error 1

> "Todo lo que hace un programa pasa constantemente por el kernel."

Incorrecto.

Operaciones como:

```c
a = b + c;
array[i] = 10;
```

normalmente se ejecutan directamente en modo usuario.

---

## Error 2

> "Una syscall es igual a un context switch."

Incorrecto.

Una syscall cambia privilegios.

Puede regresar al mismo proceso.

---

## Error 3

> "`printf()` es una syscall."

Incorrecto.

Es una función de biblioteca.

Probablemente terminará utilizando alguna syscall como:

```text
write
```

pero no son lo mismo.

---

## Error 4

> "`open()` necesariamente lee el archivo desde el disco."

Incorrecto.

Puede interactuar principalmente con metadata y estructuras del filesystem.

Además Linux utiliza caches agresivamente.

---

## Error 5

> "`read()` siempre devuelve la cantidad solicitada."

Incorrecto.

Si hacemos:

```c
read(fd, buffer, 100);
```

puede devolver:

```text
100
40
1
0
-1
```

dependiendo del recurso y las condiciones.

`0` normalmente representa EOF en un archivo o cierre ordenado en ciertos streams.

---

## Error 6

> "`write()` siempre escribe todos los bytes solicitados."

No debe asumirse en código robusto.

Existen situaciones en las que puede producirse una escritura parcial.

---

## Error 7

> "Blocking significa que el CPU se queda esperando sin hacer nada."

Incorrecto.

El thread puede quedar dormido mientras el scheduler ejecuta otro trabajo.

---

## Error 8

> "Un file descriptor identifica globalmente un archivo."

Incorrecto.

El número pertenece al espacio de descriptores del proceso.

Dos procesos pueden tener:

```text
fd 3
```

apuntando a recursos completamente diferentes.

---

# 50. Terminología importante


| Término        | Significado                                                         |
| --------------- | ------------------------------------------------------------------- |
| User mode       | Modo de ejecución con privilegios limitados                        |
| Kernel mode     | Modo privilegiado del sistema operativo                             |
| Kernel          | Núcleo del sistema operativo                                       |
| User space      | Memoria y ejecución correspondiente a procesos de usuario          |
| Kernel space    | Región protegida donde funciona el kernel                          |
| System call     | Mecanismo mediante el cual un proceso solicita servicios al kernel  |
| POSIX           | Estándar de interfaces para sistemas Unix-like                     |
| libc            | Biblioteca C utilizada por programas                                |
| Wrapper         | Función que abstrae o prepara una operación subyacente            |
| File descriptor | Entero que identifica un recurso abierto                            |
| Interrupt       | Evento normalmente externo/asíncrono que provoca entrada al kernel |
| Exception       | Evento síncrono provocado por la ejecución de una instrucción    |
| Trap            | Transferencia síncrona de control, frecuentemente intencional      |
| Blocking        | Operación que puede suspender el progreso del thread               |
| Context switch  | Cambio del contexto de CPU de un thread/proceso a otro              |
| `strace`        | Herramienta para observar syscalls y señales                       |
| `errno`         | Mecanismo utilizado por APIs C para comunicar ciertos errores       |
| Fetch           | Obtener la siguiente instrucción                                   |
| Decode          | Interpretar la instrucción                                         |
| Execute         | Ejecutar la instrucción                                            |

---

# 51. Fun facts

### Fun fact 1 — La syscall no siempre tuvo la misma instrucción

En x86 de 32 bits fue muy común utilizar:

```asm
int 0x80
```

para entrar al kernel Linux.

En x86-64 normalmente se utiliza:

```asm
syscall
```

Los mecanismos evolucionaron para hacer más eficiente la transición.

---

### Fun fact 2 — `strace` parece magia

Cuando haces:

```bash
strace curl https://example.com
```

puedes observar operaciones relacionadas con:

* archivos;
* DNS;
* sockets;
* lectura;
* escritura;
* memoria.

No estás viendo las funciones internas de `curl`.

Estás observando principalmente su frontera con el kernel.

---

### Fun fact 3 — "Everything is a file"

Unix popularizó la idea de representar muchos recursos mediante interfaces similares a archivos.

Por eso encontramos file descriptors relacionados con:

* archivos,
* terminales,
* pipes,
* sockets,
* dispositivos.

La frase "everything is a file" es una simplificación, pero captura una idea central del diseño Unix.

---

# 52. Contexto histórico

Unix apareció a finales de los años 60 y principios de los 70 en Bell Labs.

Dos decisiones tuvieron consecuencias enormes:

1. una interfaz relativamente pequeña basada en operaciones simples;
2. la reescritura de grandes partes del sistema en C.

Esto facilitó la portabilidad de Unix a diferentes arquitecturas.

Décadas después, muchas de las interfaces que utilizamos en Linux todavía mantienen esa filosofía:

```c
open
read
write
close
```

La influencia llegó a:

* BSD,
* Linux,
* macOS,
* Android,
* sistemas embebidos,
* infraestructura cloud.

Cuando un estudiante aprende:

```c
read()
```

y:

```c
write()
```

no está aprendiendo solamente funciones de C.

Está aprendiendo una interfaz conceptual que lleva más de medio siglo influyendo en el diseño de sistemas.

---

# 53. Tarea

## `syscopy`

Implementar:

```bash
./syscopy origen destino
```

El programa debe copiar un archivo utilizando exclusivamente:

```c
open()
read()
write()
close()
```

para la I/O del archivo.

Puede utilizar otras funciones para manejo de errores.

### Requisitos

Debe:

1. verificar que se recibieron dos argumentos;
2. abrir el archivo de origen;
3. crear o truncar el archivo destino;
4. leer usando un buffer;
5. manejar correctamente lecturas parciales;
6. manejar correctamente escrituras parciales;
7. cerrar ambos descriptores;
8. detectar errores;
9. mostrar errores usando `perror()`.

Ejemplo:

```bash
./syscopy original.txt copia.txt
```

Verificar:

```bash
diff original.txt copia.txt
```

Si son iguales:

```text
sin salida
```

---

# 54. Parte de análisis de la tarea

El estudiante deberá ejecutar:

```bash
strace -c ./syscopy original.txt copia.txt
```

y responder:

1. ¿Cuántas llamadas `read()` se realizaron?
2. ¿Cuántas llamadas `write()`?
3. ¿Cómo cambia el número si el buffer es de 1 byte?
4. ¿Cómo cambia si el buffer es de 4096 bytes?
5. ¿Cuál versión esperaría que fuera más eficiente?
6. ¿Por qué?

Después repetir con:

```text
BUFFER_SIZE = 1
BUFFER_SIZE = 64
BUFFER_SIZE = 4096
BUFFER_SIZE = 65536
```

No se debe exigir que los tiempos exactos sean iguales entre ejecuciones.

El alumno debe analizar principalmente:

```text
cantidad de syscalls
```

y explicar el comportamiento.

---

# 55. Resumen final de la clase

La computadora ejecuta programas siguiendo aproximadamente:

```text
fetch
  ↓
decode
  ↓
execute
```

Pero los programas normales funcionan con privilegios limitados:

```text
USER MODE
```

Para solicitar operaciones privilegiadas utilizan:

```text
system calls
```

La ruta conceptual es:

```text
Programa
   │
   ▼
API POSIX
   │
   ▼
libc wrapper
   │
   ▼
system call
   │
════════════════════
      KERNEL
════════════════════
   │
   ▼
filesystem / red / dispositivos
```

El kernel también puede tomar control debido a:

```text
interrupciones
excepciones
traps
```

Las llamadas:

```c
open()
read()
write()
close()
```

nos permiten observar directamente esta frontera.

Y:

```bash
strace
```

nos permite verla en funcionamiento.

Finalmente:

> Entrar al kernel no es gratis.

Muchas operaciones pequeñas de I/O pueden provocar muchas syscalls.

Además:

```text
I/O síncrona
```

puede bloquear un thread.

Estas dos ideas explican gran parte de la motivación detrás de técnicas modernas como:

```text
buffering
batching
event-driven I/O
asynchronous I/O
epoll
io_uring
```

La idea central que los estudiantes deberían llevarse es:

> **Una aplicación no controla directamente la computadora. Ejecuta instrucciones en un entorno restringido y solicita al kernel acceso controlado a los recursos mediante llamadas al sistema.**
>
