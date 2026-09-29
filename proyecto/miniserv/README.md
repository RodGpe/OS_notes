# MiniServ

Servidor TCP/HTTP multiproceso en C sobre Linux. Proyecto incremental del curso de **Sistemas Operativos**.

Las versiones viven en **carpetas** (`v0`, `v1`, …): cada una compila su propio binario `miniserv` dentro de esa carpeta.

## Versiones

| Carpeta | Semana | Descripción |
| ------- | ------ | ----------- |
| [`v0/`](v0/) | 1 | Imprime `MiniServ iniciado` y termina |
| [`v1/`](v1/) | 2 | TCP echo secuencial, loop `accept`, puerto 8080 |

## Uso (ejemplo V1)

```bash
cd v1
make
./miniserv
```

Cliente:

```bash
echo test | nc 127.0.0.1 8080
```

## Requisitos

- Linux, `gcc`, `make`
- `strace`, `nc`, `ss` (opcional, para laboratorio)

## Estructura

```text
miniserv/
├── README.md       ← este archivo
├── include/        ← headers compartidos (semanas futuras)
├── v0/
│   ├── Makefile
│   └── src/server.c
└── v1/
    ├── Makefile
    └── src/server.c
```

## Nota sobre la raíz `miniserv/`

El código activo está en `v0/`, `v1/`, etc. Compila siempre desde la carpeta de la versión que quieras probar.
