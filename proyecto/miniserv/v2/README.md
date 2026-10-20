# MiniServ V2 — HTTP secuencial

Misma arquitectura que V1 (un proceso, loop `accept`), pero la conversación sigue **HTTP/1.1** sobre el socket: sigue siendo `read`/`write` en un fd.

```bash
make
./miniserv
```

Cliente:

```bash
curl -v http://127.0.0.1:8081/
curl -v http://127.0.0.1:8081/hola
```

Observación (el foco del curso sigue siendo el SO, no el protocolo):

```bash
strace -e accept,read,write,close ./miniserv
# en otra terminal: curl http://127.0.0.1:8081/
```

**Siguiente versión:** [`v3/`](../v3/) añade `fork` (concurrencia); el parseo HTTP mínimo de V2 se puede combinar con fork más adelante.
