# Familia `exec*` — reemplazar la imagen de un proceso

## Qué problema resuelve

- `fork()` duplica el proceso (mismo código, dos PIDs).
- `exec*` **sustituye** el código y datos del proceso actual por los de un ejecutable cargado desde disco.
- Si `exec` tiene **éxito**, **no retorna** al programa que la llamó.
- Si falla, retorna **-1** y `errno` indica el error (el proceso sigue siendo el mismo).

Patrón clásico (shell, CGI, supervisores):

```text
fork()  →  hijo: exec(programa)     padre: wait/waitpid
```

MiniServ V3 **no** usa `exec` en el camino crítico: el hijo sigue siendo el mismo binario `miniserv` con otra rama de código.

---

## Idea común a todas las variantes


| Aspecto               | Comportamiento                                                                              |
| --------------------- | ------------------------------------------------------------------------------------------- |
| PID                   | No cambia                                                                                   |
| fds abiertos          | Por defecto**permanecen abiertos** (salvo flag `FD_CLOEXEC` / `O_CLOEXEC`) — tema Semana 4 |
| Memoria               | Nueva imagen (texto, datos, heap, pila del nuevo programa)                                  |
| Syscall real en Linux | Casi todas terminan en**`execve()`**                                                        |

---

## Las variantes principales (POSIX / glibc)

Convención de nombres:

```text
exec + [e] + [l | v] + [p]

  e  →  entorno (envp) explícito  (execve, execle, execvpe)
  l  →  lista de argumentos (variádica, terminada en NULL)
  v  →  vector de argumentos (char *argv[])
  p  →  PATH: busca el ejecutable en directorios del PATH
```

### Tabla resumen


| Función      | Argumentos                  | PATH               | Entorno (`envp`)   | Uso típico                                         |
| ------------- | --------------------------- | ------------------ | ------------------ | --------------------------------------------------- |
| **`execve`**  | `argv[]`                    | No (ruta completa) | Sí, explícito    | **Base del kernel**; wrappers y runtimes            |
| **`execv`**   | `argv[]`                    | No                 | Hereda del proceso | Ruta fija al binario                                |
| **`execvp`**  | `argv[]`                    | Sí                | Hereda             | Como`execv` pero `"ls"` sin `/bin/`                 |
| **`execvpe`** | `argv[]`                    | Sí                | Sí, explícito    | GNU; PATH + env controlado                          |
| **`execl`**   | lista`arg0, arg1, …, NULL` | No                 | Hereda             | Pocos args, ruta fija                               |
| **`execlp`**  | lista                       | Sí                | Hereda             | **Shell-like**; demo con `"echo"`                   |
| **`execle`**  | lista                       | No                 | Sí, explícito    | Ruta fija + env custom                              |
| **`fexecve`** | `argv[]` + **fd**           | No                 | Sí (GNU)          | Ejecutar desde fd abierto (seguridad, contenedores) |

En Linux, **`execve`** es la syscall; el resto son funciones de la **libc** (glibc) que arman `argv`/`envp` y llaman a `execve`.


```
char *argv[] = { "echo", "hola", NULL };
execv("/bin/echo", argv);

execl("/bin/echo", "echo", "hola", (char *)NULL);
```

char *argv[] = { "echo", "hola", NULL };
execv("/bin/echo", argv);

execl("/bin/echo", "echo", "hola", (char *)NULL);

---

## Detalle por función

### `int execve(const char *pathname, char *const argv[], char *const envp[])`

- **`pathname`:** ruta al ejecutable (ej. `"/bin/echo"`).
- **`argv`:** vector de cadenas; **`argv[0]`** convención = nombre del programa (no tiene que coincidir con el path).
- **`envp`:** variables de entorno (`KEY=value`, terminado en `NULL`).
- **No** busca en `PATH`.

### `int execv(const char *pathname, char *const argv[])`

- Igual que `execve` pero **hereda** el entorno actual (`environ`).

### `int execvp(const char *filename, char *const argv[])`

- Si `filename` no contiene `/`, busca en **`PATH`**.
- Hereda entorno.

### `int execl(const char *pathname, const char *arg0, …, (char *)NULL)`

- Argumentos como lista variable; el último debe ser **`NULL`**.
- **`arg0`** suele ser el mismo string que el nombre del programa (p. ej. `"echo"`).

### `int execlp(const char *file, const char *arg0, …, (char *)NULL)`

- Como `execl` + búsqueda en **`PATH`**.
- Usada en **`demo_exec_intro.c`:**

```c
execlp("echo", "echo", "hola desde exec en el hijo", (char *)NULL);
```

### `int execle(const char *pathname, const char *arg0, …, (char *)NULL, char *const envp[])`

- Lista de args + **`envp`** explícito al final.

### `int execvpe(const char *filename, char *const argv[], char *const envp[])`

- Extensión GNU: PATH + env explícito.

### `int fexecve(int fd, char *const argv[], char *const envp[])`

- Ejecuta el programa referido por un **fd** ya abierto (evita race TOCTOU en pathname).
- Menos común en intro; útil en temas de seguridad / sandbox.

---

## Errores frecuentes (SO)

1. **Llamar `exec` en el padre sin `fork`** — destruye el listener del servidor.
2. **Olvidar `NULL` final** en `execl` / `execlp` — comportamiento indefinido.
3. **Asumir que `exec` retorna si todo va bien** — solo retorna si **falla**.
4. **No comprobar retorno** — conviene `perror` + `_exit(127)` en el hijo (como en la demo).
5. **Confundir `exec` con `fork`** — `fork` duplica; `exec` reemplaza.

---

## Observación con `strace`

```bash
cd programas_ejemplo
gcc -Wall -Wextra demo_exec_intro.c -o demo_exec_intro
strace -e fork,execve,wait4 ./demo_exec_intro
```

Verás: `fork` → en el hijo **`execve("/usr/bin/echo", ...)`** (path resuelto por `execlp`) → padre `wait4`.

---

## Conexión profesional

- **Shell (`bash`):** `fork` + `execve` del comando + `wait`.
- **systemd / supervisores:** arrancar servicios = `exec` de otro binario (a veces tras `fork`).
- **Servidores:** prefork workers que **no** hacen exec (mismo binario); CGI clásico sí hacía `exec` de scripts.

---

## Referencias

- `man 3 exec`
- `man 2 execve`
