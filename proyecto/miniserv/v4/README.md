# MiniServ V4 — señales y cierre ordenado

Servidor TCP echo **fork-per-client** (como V3) con:

- `sigaction` para **SIGINT**, **SIGTERM** y **SIGCHLD**
- recolección de hijos en handler **SIGCHLD** (`waitpid` + `WNOHANG`)
- apagado ordenado: dejar de `accept`, cerrar `listen_fd`, **SIGKILL** a hijos vivos, `waitpid`
- **EINTR** en `accept` cuando llega señal de apagado
- **SIGPIPE** ignorado en el padre

```bash
make
./miniserv
```

Prueba concurrente (igual que V3):

```bash
nc 127.0.0.1 8080
```

Apagado (padre recibe SIGTERM/SIGINT → mata hijos con `kill -9` y espera):

```bash
kill -TERM $(pgrep -n -f 'v4/miniserv')
# o Ctrl+C si corre en foreground
```

`kill -9` **al padre** no ejecuta este apagado (SIGKILL no se captura); los hijos pueden quedar huérfanos como en V3.

Demo automatizada (cierre ordenado vs. V3):

```bash
make demo-shutdown
# con pausas para proyector:
make demo-shutdown-pause
```

Señales:

```bash
strace -e trace=signal,accept,wait4 -f ./miniserv
```

Semana 5 — evolución de `miniserv/v3/`.



Commanod para crear un archivo grande


dd if=/dev/urandom of=/tmp/big.bin bs=1M count=10000 status=progress
nc 127.0.0.1 8080 < /tmp/big.bin > /dev/null
