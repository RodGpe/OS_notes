# MiniServ V3 — fork-per-client

Servidor TCP echo: el **padre** solo `accept` + `fork` + `waitpid`; cada **hijo** atiende un cliente.

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
pstree -p $(pgrep -n -f './miniserv')
ps -o pid,ppid,stat,cmd --ppid $(pgrep -n -f './miniserv')
```

Semana 3 — sin señales ni threads. Semana 4 profundiza herencia de fds y zombies.
