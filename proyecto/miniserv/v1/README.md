# MiniServ V1

Servidor TCP **secuencial** en puerto **8080**: echo de bytes, un cliente a la vez.

```bash
make
./miniserv
```

Otra terminal:

```bash
echo hola | nc 127.0.0.1 8080
nc 127.0.0.1 8080
```

Observación:

```bash
strace -e socket,bind,listen,accept,read,write,close ./miniserv
ss -ltnp | grep 8080
```

Tras atender un cliente, el servidor **sigue escuchando** (`for (;;)` + `accept`).
