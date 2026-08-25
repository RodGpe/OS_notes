# Historia de los Sistemas Operativos, Hardware y Software

## De computadoras que ocupaban habitaciones enteras al sistema operativo de tu celular

**Duración:** 90 minutos
**Nivel:** Universidad — Sistemas Operativos
**Conocimientos previos:** programación básica en C

---

# 1. Idea central de la clase

En lugar de enseñar la historia de los sistemas operativos como una colección de fechas y nombres, vamos a contarla como una serie de problemas:

> **¿Qué problema intentaban resolver los ingenieros en cada época?**

La evolución puede verse así:

```text
Sin Sistema Operativo
        ↓
Procesamiento por lotes
        ↓
Multiprogramación
        ↓
Tiempo compartido
        ↓
UNIX
        ↓
Computadoras personales
        ↓
Sistemas modernos
        ↓
Virtualización, contenedores y nube
```

En cada etapa aparecen nuevos problemas:

```text
Computadora demasiado cara
        ↓
Hay que aprovechar mejor el CPU

CPU esperando dispositivos
        ↓
Multiprogramación

Muchos usuarios quieren usarla
        ↓
Time-sharing

Muchos programas comparten memoria
        ↓
Protección

Muchas computadoras diferentes
        ↓
Portabilidad

Miles de servidores
        ↓
Virtualización y cloud
```

La historia de los sistemas operativos es, en buena medida, **la historia de cómo compartir recursos costosos de forma eficiente y segura**.

---

# 2. Objetivos de aprendizaje

Al finalizar la clase, el estudiante será capaz de:

1. Diferenciar **hardware, software y sistema operativo**.

2. Explicar por qué las primeras computadoras prácticamente no necesitaban sistema operativo.

3. Describir la evolución:

   * procesamiento por lotes;
   * multiprogramación;
   * tiempo compartido;
   * UNIX;
   * computadoras personales;
   * sistemas operativos modernos.

4. Explicar cómo el desarrollo del hardware permitió sistemas operativos cada vez más sofisticados.

5. Reconocer la influencia de factores económicos, militares y geopolíticos en la evolución de la computación.

6. Explicar las dos funciones fundamentales de un SO:

   * administrar recursos;
   * proporcionar abstracciones.

7. Relacionar problemas históricos con conceptos modernos como:

   * procesos;
   * scheduling;
   * memoria virtual;
   * interrupciones;
   * permisos;
   * contenedores;
   * máquinas virtuales.

---

# 3. Plan de 90 minutos

|    Tiempo | Tema                                                |
| --------: | --------------------------------------------------- |
|   0–8 min | Hook: imagina una computadora sin sistema operativo |
|  8–20 min | Hardware, software y sistema operativo              |
| 20–32 min | 1940–1950: computadoras sin SO                      |
| 32–47 min | Batch processing y multiprogramación                |
| 47–60 min | Time-sharing, Guerra Fría y redes                   |
| 60–70 min | UNIX y el nacimiento de muchas ideas modernas       |
| 70–78 min | Computadora personal → sistemas modernos            |
| 78–86 min | Demostración en Linux                               |
| 86–90 min | Discusión, errores comunes y conclusión             |

**Total: 90 minutos.**

---

# 4. Introducción — 0 a 8 minutos

Empieza la clase sin definir qué es un sistema operativo.

Pregunta:

> Imaginen que la UNAM compra una computadora de varios millones de dólares.
>
> Pero no tiene Windows.
>
> No tiene Linux.
>
> No tiene terminal.
>
> No tiene archivos.
>
> No tiene `printf()`.
>
> No existe el mouse.
>
> Ni siquiera existe el concepto moderno de "proceso".
>
> ¿Cómo ejecutarían un programa?

Probablemente los estudiantes responderán:

* conectar algo;
* cargar el programa;
* introducir instrucciones;
* usar tarjetas;
* escribir directamente en memoria.

Eso se acerca bastante a lo que ocurría.

Ahora agrega:

> Hay otras 20 personas esperando utilizar la misma computadora.
>
> ¿Quién decide cuándo puede usarla cada una?

En las primeras computadoras:

**los humanos hacían muchas de las tareas que después realizaría el sistema operativo.**

---

# 5. Hardware, software y sistema operativo — 8 a 20 minutos

## Hardware

Hardware son los componentes físicos.

```text
CPU
RAM
SSD / HDD
GPU
teclado
monitor
tarjeta de red
USB
```

### CPU

La CPU ejecuta instrucciones.

De manera simplificada:

```text
Fetch
  ↓
Decode
  ↓
Execute
  ↓
Fetch
```

Una CPU entiende instrucciones máquina.

No entiende directamente conceptos como:

```text
Chrome
Spotify
Word
Docker
archivo
usuario
ventana
proceso
```

Estos son conceptos construidos mediante software.

---

## RAM

La RAM almacena temporalmente instrucciones y datos que están siendo utilizados.

```text
SSD
│
│ programa almacenado
│
▼
RAM
│
│ instrucciones
│
▼
CPU
```

Por ejemplo:

```text
SSD
─────────────────
chrome
gcc
programa.c
foto.jpg

       ↓ cargar

RAM
─────────────────
Chrome ejecutándose
GCC ejecutándose
Sistema operativo

       ↓

CPU
```

RAM normalmente es:

* rápida;
* volátil;
* limitada.

---

## Almacenamiento

SSD y HDD proporcionan almacenamiento persistente.

```text
RAM

rápida
temporal
volátil

      VS

SSD

más lento
persistente
mayor capacidad
```

Esta diferencia será fundamental cuando estudien **memoria virtual**.

---

# 6. Software

Podemos dividirlo conceptualmente en:

```text
SOFTWARE
│
├── Aplicaciones
│   ├── navegador
│   ├── videojuegos
│   ├── Spotify
│   └── VS Code
│
└── Software de sistema
    ├── sistema operativo
    ├── drivers
    ├── compiladores
    └── herramientas del sistema
```

El sistema operativo se encuentra entre las aplicaciones y el hardware.

---

# Diagrama 1 — Las capas de un sistema de cómputo

```text
+--------------------------------------+
|            Aplicaciones              |
|                                      |
| Chrome    GCC    Spotify    VS Code  |
+--------------------------------------+
                  │
                  │ APIs
                  │ system calls
                  ▼
+--------------------------------------+
|         SISTEMA OPERATIVO            |
|                                      |
| Procesos       Memoria               |
| Archivos       Networking            |
| Seguridad      Drivers               |
| Scheduling                           |
+--------------------------------------+
                  │
                  ▼
+--------------------------------------+
|              HARDWARE                |
|                                      |
| CPU    RAM    SSD    GPU    NIC      |
+--------------------------------------+
```

Pregunta al grupo:

> ¿Por qué Chrome no debería poder controlar directamente el SSD?

Posibles respuestas:

* podría borrar información;
* podría leer archivos de otros usuarios;
* dos programas podrían utilizarlo al mismo tiempo;
* cada SSD es diferente;
* sería difícil programar aplicaciones.

Exactamente.

El sistema operativo sirve como intermediario.

---

# 7. Las dos grandes funciones del sistema operativo

Una definición útil durante todo el curso es:

> **Un sistema operativo es un administrador de recursos y un proveedor de abstracciones.**

---

## Función 1: administrador de recursos

Imagina:

```text
                   CPU
                    │
          ┌─────────┼─────────┐
          ▼         ▼         ▼
       Chrome     Spotify     GCC
```

Los tres quieren utilizar el CPU.

¿Quién corre primero?

El sistema operativo.

Lo mismo ocurre con:

```text
CPU
RAM
SSD
red
GPU
impresora
USB
```

---

## Función 2: proveedor de abstracciones

Un SSD físico trabaja con bloques.

Pero una aplicación no suele decir:

> Lee el bloque físico 875923.

Dice:

```c
open("calificaciones.txt", O_RDONLY);
```

El sistema operativo proporciona una abstracción llamada:

**archivo**.

Lo mismo sucede con:

```text
Abstracción       Realidad física

proceso        →  CPU
archivo        →  bloques de almacenamiento
memoria virtual → RAM
socket         →  hardware de red
thread         →  tiempo de CPU
```

---

# 8. 1940–1950: antes del sistema operativo — 20 a 32 minutos

Las primeras computadoras electrónicas eran radicalmente diferentes.

Generalmente no existían:

* sistemas operativos;
* terminales;
* interfaces gráficas;
* discos modernos;
* lenguajes de alto nivel como los actuales.

Los programadores interactuaban mucho más directamente con la máquina.

```text
Programador
     │
     ▼
+------------+
| Hardware   |
|            |
| CPU        |
| Memoria    |
| I/O        |
+------------+
```

---

# Contexto histórico: Segunda Guerra Mundial

La Segunda Guerra Mundial aceleró enormemente el desarrollo de tecnología de cómputo.

Había problemas de enorme interés militar:

* criptografía;
* cálculos balísticos;
* artillería;
* navegación;
* simulaciones;
* procesamiento de grandes cantidades de información.

Computadoras como **Colossus** fueron utilizadas por los británicos para ayudar a descifrar comunicaciones alemanas.

En Estados Unidos, **ENIAC** fue desarrollado inicialmente para cálculos relacionados con tablas de tiro de artillería.

La inversión militar ayudó a convertir la computación de un experimento académico en una tecnología estratégica.

---

## Fun fact

ENIAC pesaba aproximadamente **30 toneladas** y utilizaba alrededor de **18,000 tubos de vacío**.

Hoy, un teléfono tiene millones de veces más capacidad computacional.

Puedes preguntar:

> Si tu celular apareciera mágicamente en 1946, ¿qué pensarían los ingenieros?

Probablemente sería indistinguible de tecnología alienígena.

---

# 9. Las computadoras eran extremadamente caras

El problema fundamental era económico.

```text
COMPUTADORA

$$$$$$$$$$$$$$$$$$$$
```

Si una computadora cuesta una fortuna, no quieres verla esperando a que un humano cambie tarjetas o prepare el siguiente programa.

Pero eso sucedía.

```text
CPU trabajando:

██████████


CPU esperando humanos:

██████████████████████████████
```

Los ingenieros comenzaron a pensar:

> ¿Cómo podemos mantener ocupada la máquina?

---

# 10. Procesamiento por lotes — 32 a 38 minutos

Una solución fue el **batch processing** o procesamiento por lotes.

En lugar de permitir que cada programador opere la máquina directamente:

1. los programadores entregan sus programas;
2. un operador los reúne;
3. se crea una cola de trabajos;
4. la computadora los ejecuta automáticamente.

```text
JOB A
  ↓
JOB B
  ↓
JOB C
  ↓
JOB D
```

---

## Una analogía

Piensa en una lavandería.

Sería absurdo:

```text
lavar una camiseta
esperar
lavar otra camiseta
esperar
lavar otra camiseta
```

Agrupas trabajo.

Eso es un **batch**.

---

## Pero programar era frustrante

Imagina escribir:

```c
printf("Hola mundo\n");
```

Entregas tu trabajo.

Dos horas después recibes:

```text
COMPILATION ERROR
```

Olvidaste un `;`.

Corriges.

Vuelves a entregar.

Esperas otra vez.

Puedes decir al grupo:

> Cada vez que su compilador les marca un error en 0.2 segundos, agradezcan no estar programando en 1955.

---

# 11. El siguiente problema: I/O — 38 a 47 minutos

Supón que un programa está corriendo.

De pronto necesita leer información de un dispositivo lento.

```text
CPU

ejecutando
██████████

esperando I/O
................

ejecutando
██████████
```

La CPU permanece inutilizada.

Pero hay otro programa esperando.

Entonces surge una idea:

> ¿Por qué no ejecutar otro programa mientras el primero espera?

Así nace una idea fundamental:

# Multiprogramación

En memoria pueden existir varios programas.

```text
RAM
+---------------------+
| Sistema Operativo   |
+---------------------+
| Programa A          |
+---------------------+
| Programa B          |
+---------------------+
| Programa C          |
+---------------------+
```

Supongamos:

```text
Programa A → esperando disco

Programa B → listo

Programa C → esperando teclado
```

Entonces:

```text
CPU → Programa B
```

Mucho mejor.

---

# Pero acabamos de crear muchos problemas

Pregunta al grupo:

> ¿Qué podría salir mal?

Ahora necesitamos responder:

```text
¿Quién usa el CPU?

¿Durante cuánto tiempo?

¿Quién utiliza qué memoria?

¿Puede A leer la memoria de B?

¿Qué pasa si A nunca termina?

¿Qué pasa si dos programas quieren escribir al disco?

¿Qué pasa si ambos modifican el mismo dato?
```

Y prácticamente acabamos de inventar el temario de Sistemas Operativos:

```text
Scheduling
Memory management
Processes
Protection
Synchronization
I/O
Concurrency
```

---

# 12. Hardware que hizo esto posible: interrupciones

Una tecnología fundamental son las **interrupciones**.

Sin ellas, el CPU podría verse obligado a preguntar constantemente:

```text
¿Terminaste disco?

¿Terminaste?

¿Ahora?

¿Ahora?

¿Ahora?
```

Esto se llama **polling**.

Una interrupción permite que el dispositivo diga:

> CPU, necesito tu atención.

```text
             INTERRUPCIÓN
                  ▲
                  │
+----------+      │
|  Disco   |──────┘
+----------+

                  ▼

              +-------+
              |  CPU  |
              +-------+
```

El CPU puede detener temporalmente lo que hace y transferir control al sistema operativo.

---

## Fun fact

Muchas ideas importantes de los sistemas operativos dependen tanto del **hardware** como del software.

El sistema operativo moderno no podría funcionar igual sin mecanismos de hardware como:

* interrupciones;
* timer;
* modos privilegiados;
* MMU;
* protección de memoria.

Por eso Sistemas Operativos no es solamente una materia de software.

---

# 13. Time-sharing — 47 a 54 minutos

La multiprogramación mejoró la utilización del CPU.

Pero los usuarios querían algo más:

**interactividad.**

Imagina 20 terminales conectadas a una misma computadora.

```text
Usuario A ─┐
Usuario B ─┤
Usuario C ─┤
Usuario D ─┼──► COMPUTADORA
Usuario E ─┤
            │
...
```

La computadora asigna pequeños intervalos de CPU a cada usuario.

```text
Tiempo →

A ███
B    ███
C       ███
A          ███
B             ███
```

Como el cambio ocurre rápidamente, cada usuario tiene la ilusión de utilizar la computadora.

Eso es **time-sharing**.

---

## Actividad rápida

Escoge tres estudiantes.

Ellos son:

```text
Process A
Process B
Process C
```

Dales el plumón.

Cada uno puede escribir durante tres segundos.

Después dices:

> ¡INTERRUPT!

Le quitas el plumón y se lo das al siguiente.

Técnicamente acabas de implementar un scheduler bastante malo.

Pero es un scheduler.

---

# 14. Contexto geopolítico: Guerra Fría — 54 a 60 minutos

Después de la Segunda Guerra Mundial comenzó una enorme competencia tecnológica entre Estados Unidos y la Unión Soviética.

La informática dejó de ser simplemente una herramienta científica.

También se volvió estratégica.

Áreas importantes incluían:

```text
misiles
radar
defensa aérea
satélites
criptografía
simulación
comunicaciones
exploración espacial
```

---

## Sputnik, 1957

Cuando la Unión Soviética lanzó **Sputnik 1**, el primer satélite artificial, causó una enorme conmoción en Estados Unidos.

La percepción era:

> "Los soviéticos nos están ganando tecnológicamente."

Estados Unidos aumentó fuertemente la inversión en ciencia, ingeniería y tecnología.

En este ambiente surgiría **ARPA**, después DARPA.

---

## Fun fact

Una de las consecuencias indirectas de esta competencia tecnológica fue el desarrollo de tecnologías relacionadas con redes de computadoras.

En **1969** comenzó a operar ARPANET, uno de los grandes antecedentes técnicos de Internet.

Ese mismo año ocurrió otra cosa interesante.

---

# 15. 1969: un año bastante importante

En 1969:

```text
Apollo 11 llega a la Luna.

ARPANET comienza a operar.

Comienza el desarrollo de UNIX.
```

Tres acontecimientos diferentes, pero todos relacionados con una época de enorme inversión tecnológica.

Puedes escribir en el pizarrón:

```text
1969

Luna

ARPANET

UNIX
```

---

# 16. UNIX — 60 a 70 minutos

UNIX comenzó a desarrollarse en **Bell Labs**.

Entre los nombres fundamentales están:

* Ken Thompson;
* Dennis Ritchie.

UNIX introdujo o popularizó muchas ideas que siguen siendo fundamentales.

Por ejemplo:

```text
procesos
archivos
pipes
shell
jerarquía de directorios
permisos
herramientas pequeñas
```

---

# La filosofía Unix

Una idea general:

> Haz programas pequeños que hagan una cosa bien y permite combinarlos.

Por ejemplo:

```bash
ls
```

lista archivos.

```bash
grep
```

busca texto.

```bash
wc
```

cuenta.

Pero podemos hacer:

```bash
ls | grep ".c" | wc -l
```

Tenemos:

```text
ls
 │
 ▼
grep
 │
 ▼
wc
```

La salida de un programa se convierte en la entrada del siguiente.

El símbolo:

```text
|
```

representa un **pipe**.

Más adelante en el curso los estudiantes podrán implementar sus propios pipes en C.

---

# Fun fact: C y UNIX crecieron juntos

UNIX inicialmente tenía partes escritas en ensamblador.

Dennis Ritchie desarrolló C en Bell Labs y una buena parte de UNIX fue reescrita en C.

Esto fue extremadamente importante.

Anteriormente:

```text
Sistema Operativo
       ↓
Hardware específico
```

Con C:

```text
               UNIX
                 │
              escrito
               en C
                 │
        ┌────────┴────────┐
        ▼                 ▼
 Hardware A           Hardware B
```

Esto ayudó a que UNIX pudiera ser transportado entre diferentes arquitecturas.

Por eso no es casualidad que en una clase moderna de Sistemas Operativos sigamos usando C.

---

# Fun fact: ¿Por qué se llama UNIX?

Antes de UNIX existía un ambicioso proyecto llamado **MULTICS**:

```text
MULTiplexed
Information and
Computing Service
```

UNIX surgió inicialmente como un sistema mucho más pequeño.

Su nombre fue un juego de palabras relacionado con MULTICS.

---

# 17. Computadoras personales — 70 a 74 minutos

Hasta aproximadamente los años 70:

```text
muchas personas
       ↓
una computadora
```

Pero el precio del hardware cayó rápidamente.

Con los microprocesadores aparece otro paradigma:

```text
una persona
     ↓
una computadora
```

Esto cambia las prioridades.

Ya no necesitas diseñar todo principalmente para 100 usuarios compartiendo una máquina.

Comienzan a hacerse importantes:

* interfaces fáciles de usar;
* gráficos;
* teclado y mouse;
* aplicaciones personales.

---

# Fun fact: la computadora dejó de ser "la computadora"

Durante décadas, una organización podía tener:

> **la computadora**

Una sola.

Hoy puedes tener fácilmente:

```text
laptop
celular
smartwatch
TV
router
automóvil
consola
Alexa
```

todos ejecutando algún tipo de sistema operativo.

---

# 18. 1980–1990: PC, interfaces gráficas y competencia comercial

Empresas como:

* Apple;
* Microsoft;
* IBM;

participaron en una enorme transformación del mercado.

El sistema operativo pasó de ser algo utilizado principalmente por:

```text
científicos
ingenieros
universidades
gobierno
grandes empresas
```

a ser utilizado por prácticamente cualquier persona.

---

## Fun fact

El mouse y muchas ideas de interfaces gráficas fueron experimentadas años antes de hacerse populares comercialmente.

El laboratorio Xerox PARC desarrolló sistemas con conceptos como:

* ventanas;
* iconos;
* mouse;
* interfaces gráficas.

Apple y posteriormente Microsoft ayudaron a llevar estas ideas al mercado masivo.

---

# 19. Linux y el mundo moderno — 74 a 78 minutos

En 1991, Linus Torvalds comenzó un kernel que se convertiría en **Linux**.

Linux está hoy presente en muchísimos lugares:

```text
servidores
cloud
supercomputadoras
routers
Android
IoT
embedded systems
```

En muchas ocasiones el usuario ni siquiera sabe que está utilizando Linux.

---

# Fun fact

Android utiliza el **kernel de Linux**.

Así que si alguien dice:

> "Yo nunca uso Linux."

pero tiene un teléfono Android...

probablemente sí usa Linux todos los días.

---

# 20. La evolución completa

# Diagrama 2 — Historia conceptual del sistema operativo

```text
1940
│
│  SIN SISTEMA OPERATIVO
│  Humanos controlan directamente la máquina
│
▼
1950
│
│  BATCH PROCESSING
│  Automatizar secuencias de trabajos
│
▼
1960
│
│  MULTIPROGRAMACIÓN
│  Mantener ocupado el CPU
│
▼
1960–1970
│
│  TIME-SHARING
│  Muchos usuarios interactivos
│
▼
1970
│
│  UNIX
│  Procesos + archivos + pipes + portabilidad
│
▼
1980
│
│  PERSONAL COMPUTERS
│  Una computadora por usuario
│
▼
1990
│
│  INTERNET + LINUX + WINDOWS
│
▼
2000
│
│  MÓVILES + VIRTUALIZACIÓN
│
▼
2010+
│
│  CLOUD + CONTAINERS
│
▼
HOY
```

Lo interesante es que las ideas anteriores **no desaparecieron**.

Se acumularon.

---

# 21. De los mainframes a AWS

Puedes sorprender a los alumnos con esta comparación.

En los años 60:

```text
COMPUTADORA CARÍSIMA

       ↓

muchos usuarios

       ↓

hay que compartirla eficientemente
```

Hoy:

```text
DATACENTER CARÍSIMO

       ↓

miles de clientes

       ↓

hay que compartirlo eficientemente
```

El problema fundamental reaparece.

Ahora utilizamos:

```text
máquinas virtuales
contenedores
orquestadores
cloud computing
```

En cierta forma:

> AWS y Azure están resolviendo a escala planetaria algunos de los mismos problemas conceptuales que los mainframes intentaban resolver hace décadas.

---

# 22. Virtualización moderna

Un servidor moderno puede verse así:

```text
+---------------------------+
| Aplicaciones              |
+---------------------------+
| Containers                |
+---------------------------+
| Linux                     |
+---------------------------+
| Máquina Virtual           |
+---------------------------+
| Hypervisor                |
+---------------------------+
| Hardware                  |
+---------------------------+
```

Seguimos preguntando:

> ¿Cómo puedo compartir hardware sin que un usuario interfiera con otro?

---

# 23. Demostración Linux — 78 a 86 minutos

Ahora muestra a los estudiantes que las ideas históricas siguen vivas.

---

## Ver el CPU

```bash
lscpu
```

Busca:

```text
Architecture
CPU(s)
Core(s)
Thread(s)
Virtualization
```

Pregunta:

> ¿Cuántos CPUs lógicos tenemos?

---

## Ver memoria

```bash
free -h
```

Ejemplo:

```text
              total       used       free
Mem:           16Gi        7Gi        3Gi
```

Pregunta:

> Si tengo cientos de procesos, ¿cómo comparten 16 GB?

Introducción perfecta para memoria virtual.

---

## Ver procesos

```bash
ps -ef
```

o:

```bash
ps aux
```

Puedes contar los procesos:

```bash
ps -e --no-headers | wc -l
```

Supongamos:

```text
327
```

Ahora:

```bash
nproc
```

Resultado:

```text
8
```

Pregunta:

> Tenemos 327 procesos y solamente 8 CPUs lógicos.
>
> ¿Cómo es posible?

Respuesta:

**scheduling + procesos esperando I/O + multiprogramación.**

Estamos observando en 2026 conceptos desarrollados desde los años 50 y 60.

---

# 24. Pequeña demostración en C

Crea:

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
    pid_t pid = getpid();

    if (pid <= 0) {
        fprintf(stderr, "No fue posible obtener un PID válido\n");
        return EXIT_FAILURE;
    }

    printf("Hola. Soy el proceso %ld\n", (long)pid);

    return EXIT_SUCCESS;
}
```

Compilar:

```bash
gcc -Wall -Wextra process.c -o process
```

Ejecutar:

```bash
./process
```

Podría aparecer:

```text
Hola. Soy el proceso 18231
```

Ejecuta nuevamente:

```bash
./process
```

Ahora:

```text
Hola. Soy el proceso 18234
```

Pregunta:

> ¿De dónde salió ese PID?

El kernel lo asignó al proceso.

Ya podemos ver una de las abstracciones fundamentales del SO.

---

# 25. Una demostración todavía más interesante

En Linux:

```bash
strace ./process
```

Los estudiantes descubrirán algo interesante:

Un programa aparentemente trivial genera muchas interacciones con el sistema operativo.

Dependiendo de la distribución, arquitectura y libc, la salida de `strace` puede variar.

Pero permite introducir una idea fundamental:

```text
Programa
    │
    │ system calls
    ▼
Kernel
    │
    ▼
Hardware
```

---

# 26. Tres preguntas para discusión

## Pregunta 1

Una computadora está ejecutando un programa.

El programa solicita información al disco y debe esperar.

Hay otro programa listo.

> ¿Qué debería hacer el sistema operativo?

Respuesta esperada:

Ejecutar otro programa.

**Multiprogramación.**

---

## Pregunta 2

¿Por qué Chrome no debería poder leer directamente cualquier posición física de RAM?

Posibles respuestas:

* seguridad;
* aislamiento;
* corrupción;
* estabilidad.

Esto conduce a:

**protección de memoria.**

---

## Pregunta 3

Tu laptop tiene:

```text
8 CPUs lógicos
```

pero:

```text
350 procesos
```

¿Cómo puede ocurrir?

Porque:

* muchos procesos están esperando;
* el scheduler comparte CPU;
* existen múltiples núcleos;
* los procesos entran y salen del CPU constantemente.

---

# 27. Dos ejercicios de programación

## Ejercicio 1 — Observar multiprogramación

Programa:

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
    for (int i = 0; i < 20; i++) {

        printf("PID %ld → iteración %d\n",
               (long)getpid(), i);

        fflush(stdout);

        if (usleep(100000) != 0) {
            perror("usleep");
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
```

Compilar:

```bash
gcc -Wall -Wextra worker.c -o worker
```

Ejecutar tres instancias:

```bash
./worker &
./worker &
./worker &
wait
```

Los resultados se mezclarán.

El orden específico **no está garantizado**.

Pregunta:

> ¿Por qué?

Concepto:

**scheduling y concurrencia.**

---

## Ejercicio 2 — Hardware contra abstracciones

Ejecutar:

```bash
lscpu
free -h
lsblk
ps -e
```

Responder:

1. ¿Cuántos CPUs lógicos existen?
2. ¿Cuánta RAM física existe?
3. ¿Cuántos procesos existen?
4. ¿Hay más procesos que CPUs?
5. ¿Qué dispositivos de almacenamiento aparecen?
6. ¿Qué elementos corresponden a hardware?
7. ¿Qué elementos corresponden a abstracciones creadas por el SO?

---

# 28. Errores y conceptos equivocados comunes

## Error 1

> "El sistema operativo es la interfaz gráfica."

No.

Esto:

```text
GNOME
Windows Explorer
macOS Finder
```

es solamente una parte visible.

Debajo existen:

```text
scheduler
kernel
virtual memory
drivers
network stack
filesystem
```

---

## Error 2

> "Ubuntu y Linux son exactamente lo mismo."

No exactamente.

Simplificando:

```text
Linux = kernel

Ubuntu = distribución
```

Ubuntu utiliza Linux junto con muchas otras herramientas.

---

## Error 3

> "Programa y proceso son lo mismo."

No.

```text
PROGRAMA

archivo ejecutable
en almacenamiento

       ↓ ejecutar

PROCESO

instancia en ejecución
```

Un programa puede generar múltiples procesos.

---

## Error 4

> "Si tengo 8 cores solamente puedo tener 8 programas."

Puedes tener cientos de procesos.

No todos necesitan ejecutar exactamente al mismo tiempo.

---

## Error 5

> "Las computadoras siempre tuvieron sistemas operativos."

No.

Los primeros sistemas no tenían algo parecido a un SO moderno.

Muchas funciones eran realizadas manualmente por operadores.

---

## Error 6

> "Las tecnologías modernas reemplazaron completamente las ideas antiguas."

En realidad se construyeron encima de ellas.

Un contenedor moderno todavía depende de conceptos como:

```text
procesos
memoria
scheduler
filesystem
permisos
```

---

# 29. Fun facts para repartir durante la clase

Puedes insertar estos datos cuando notes que baja la atención.

### Fun fact 1

Las computadoras alguna vez fueron operadas físicamente por humanos como un recurso central de una organización.

Hoy desperdiciamos más capacidad computacional cargando anuncios en una página web que la que tuvieron muchas computadoras históricas completas.

---

### Fun fact 2

El término **bug** para describir fallos técnicos existía antes de las computadoras.

En 1947, el equipo asociado con Grace Hopper encontró literalmente una polilla atrapada en un relay de la computadora Harvard Mark II y la pegó en su bitácora.

La polilla no inventó el término.

Pero se convirtió en una de las anécdotas más famosas de la informática.

---

### Fun fact 3

UNIX comenzó en 1969.

Linux apareció en 1991.

Más de medio siglo después del nacimiento de UNIX, seguimos utilizando comandos e ideas enormemente influenciadas por ese sistema.

---

### Fun fact 4

Tu teléfono probablemente ejecuta más procesos simultáneamente que un mainframe antiguo hubiera podido imaginar.

---

### Fun fact 5

Un automóvil moderno puede contener decenas de unidades de control electrónicas, cada una ejecutando software especializado.

La computación dejó de estar solamente en "las computadoras".

---

# 30. Actividad entretenida — "Reinventemos el sistema operativo"

Presenta este escenario:

> Año: 1965.
>
> Tu computadora cuesta millones de dólares.
>
> Tiene un CPU.
>
> Diez científicos quieren utilizarla.
>
> El disco es lento.
>
> Algunos programas entran en loops infinitos.
>
> Algunos usuarios no deben poder leer la información de otros.

Pregunta:

> Diseñen el sistema.

Dales cinco minutos.

Probablemente propondrán:

```text
cola de trabajos
        ↓
Batch processing

ejecutar otro programa durante I/O
        ↓
Multiprogramación

dar algunos milisegundos a cada programa
        ↓
Scheduling / time-sharing

detener programas después de cierto tiempo
        ↓
Timer interrupt

separar la memoria
        ↓
Memory protection

identificar usuarios
        ↓
Permissions
```

Después puedes decir:

> Felicidades.
>
> Acaban de redescubrir aproximadamente 20 años de investigación en sistemas operativos.

---

# 31. ¿Por qué importa estudiar esta historia?

Porque permite entender **por qué existen las cosas**.

En lugar de memorizar:

```text
scheduler
```

entiendes:

```text
Muchos programas
      ↓
pocos CPUs
      ↓
¿quién corre?
      ↓
scheduler
```

En lugar de memorizar:

```text
virtual memory
```

entiendes:

```text
muchos procesos
      ↓
RAM limitada
      ↓
aislamiento
      ↓
virtual memory
```

En lugar de memorizar:

```text
permissions
```

entiendes:

```text
muchos usuarios
      ↓
recursos compartidos
      ↓
no todos deben acceder a todo
      ↓
permissions
```

---

# 32. Terminología importante

Los estudiantes deberían salir de la clase entendiendo:

### Hardware

Componentes físicos del sistema.

### Software

Instrucciones y programas ejecutados por el hardware.

### Operating System

Software que administra recursos y proporciona abstracciones.

### Kernel

Parte privilegiada y central del sistema operativo.

### Process

Programa en ejecución.

### Program

Código ejecutable almacenado.

### Batch Processing

Procesamiento automático de grupos de trabajos.

### Multiprogramming

Mantener múltiples programas disponibles para utilizar mejor el CPU.

### Time-sharing

Compartir rápidamente CPU entre múltiples procesos o usuarios.

### Interrupt

Evento que provoca que la CPU atienda al sistema operativo o a un dispositivo.

### System Call

Mecanismo controlado mediante el cual un programa solicita servicios al kernel.

### Device Driver

Software que permite al sistema operativo controlar determinado hardware.

### Virtualization

Creación de representaciones virtuales de recursos físicos.

### Scheduler

Componente que decide qué proceso o thread obtiene CPU.

---

# 33. Tarea

## ¿Por qué existe esta característica?

Cada estudiante elegirá una característica moderna:

```text
procesos
memoria virtual
filesystem
permisos
multitasking
device drivers
containers
virtual machines
system calls
```

Deberá responder en aproximadamente 1–2 páginas:

1. ¿Qué problema resuelve?
2. ¿Qué ocurriría si no existiera?
3. ¿Qué necesidad histórica ayudó a que apareciera?
4. ¿De qué soporte de hardware depende?
5. ¿Dónde puedo observarla actualmente en Linux?

Debe incluir por lo menos un comando como:

```bash
ps
free
ls -l
lsblk
lscpu
uname
```

---

# 34. Resumen final — últimos 4 minutos

Escribe en el pizarrón:

```text
                 COMPUTADORA

+-----------------------------------+
|          Aplicaciones             |
+-----------------------------------+
                 │
                 │ abstracciones
                 ▼
+-----------------------------------+
|       SISTEMA OPERATIVO           |
|                                   |
| Procesos   Memoria   Archivos     |
| Scheduling Seguridad Networking   |
+-----------------------------------+
                 │
                 │ administra
                 ▼
+-----------------------------------+
|            HARDWARE               |
|                                   |
| CPU      RAM      SSD    Devices  |
+-----------------------------------+
```

Después recorre rápidamente la historia:

```text
Sin SO
  ↓
el humano controla el hardware

Batch
  ↓
automatizar trabajos

Multiprogramación
  ↓
no desperdiciar CPU

Time-sharing
  ↓
muchos usuarios interactivos

UNIX
  ↓
procesos + archivos + pipes + C

PC
  ↓
una computadora por persona

Internet
  ↓
computadoras conectadas

Virtualización
  ↓
una computadora física simula muchas

Cloud
  ↓
datacenters compartidos a escala global

Containers
  ↓
aislamiento ligero de aplicaciones
```

---

# Mensaje final de la clase

> **Un sistema operativo no apareció porque alguien decidió que las computadoras necesitaban un menú bonito.**
>
> Apareció porque las computadoras eran caras, lentas, compartidas y cada vez más complejas.

Cada generación solucionó un problema y creó otros nuevos.

```text
Compartir CPU      → Scheduling

Compartir RAM      → Memory management

Compartir disco    → Filesystems

Compartir datos    → Synchronization

Compartir hardware → Drivers

Compartir sistemas → Virtualization

Compartir datacenter → Cloud
```

Y esa es una excelente forma de entender el resto del curso:

> **Sistemas Operativos es la historia de cómo convertir hardware limitado y complejo en abstracciones que muchos programas puedan utilizar de manera eficiente, segura y aparentemente simultánea.**
