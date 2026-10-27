# MiniServ V3 — fork-per-client

Servidor TCP echo: el **padre** solo `accept` + `fork` + `waitpid`; cada **hijo** atiende un cliente.

Cada hijo hace una **pausa de 20 s** (`SERVE_DELAY_SEC` en `src/server.c`) antes del echo, para observar hijos vivos en `ps`/`pstree`. Pon `0` para desactivarla.

```bash
make
./miniserv
```

Prueba concurrente:

```bash
# Terminal 2 y 3 a la vez (nc interactivo)
nc 127.0.0.1 8080
```

Observación:

```bash
pstree -p $(pgrep -n -f 'v3/miniserv')
ps -o pid,ppid,stat,cmd --ppid $(pgrep -n -f 'v3/miniserv')
```

Semana 3 — sin señales ni threads. Semana 4 profundiza herencia de fds y zombies.

---

## Experimento: hijos vivos (no zombies)

Con la pausa, un cliente conectado deja un **hijo en estado `S`** (en `sleep` o luego en `read`), no un zombie (`Z` solo aparece brevemente tras `_exit` si el padre no hace `wait`).

**Terminal 1 — servidor**

```bash
cd miniserv/v3 && make && ./miniserv
```

**Terminal 2 — cliente (dejar conectado; escribe una línea o no)**

```bash
nc 127.0.0.1 8080
```

**Terminal 3 — árbol de procesos (durante los 20 s de pausa)**

```bash
PADRE=$(pgrep -n -f 'v3/miniserv')
pstree -p "$PADRE"
ps -o pid,ppid,stat,cmd --ppid "$PADRE"
```

Deberías ver **un hijo** con `STAT` **`S`** y un mensaje en stderr del hijo: `pausa 20 s antes del echo`.

**Dos clientes a la vez** (dos hijos bajo el mismo padre):

```bash
# Terminal 2 y 4, sin cerrar nc
nc 127.0.0.1 8080
nc 127.0.0.1 8080
```

```bash
ps --ppid "$PADRE" -o pid,stat,cmd
```

**Apagado brusco (V3, sin cierre ordenado):** con un cliente conectado durante la pausa, en terminal 1 **Ctrl+C**. En terminal 3:

```bash
ps -ef | grep miniserv
```

El **hijo puede seguir vivo** (huérfano) mientras termina el `sleep`/`read`. Comparar con `miniserv/v4` y `make demo-shutdown`.

Script relacionado: `proyecto/semana4/programas_ejemplo/demo_bloque1_apagado_brusco.sh` (usa V3).
