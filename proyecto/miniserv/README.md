# MiniServ

Servidor TCP/HTTP multiproceso en C sobre Linux. Proyecto incremental del curso de **Sistemas Operativos**.

## Estado actual (Semana 1 — V0)

Programa mínimo que imprime un mensaje de arranque. Aún no escucha conexiones de red.

## Requisitos

- Linux
- `gcc`
- `make`
- `strace` (opcional, para observar system calls)

## Compilación

```bash
make
```

Equivalente manual:

```bash
gcc -Wall -Wextra -std=c11 src/server.c -o miniserv
```

## Ejecución

```bash
./miniserv
```

Salida esperada:

```text
MiniServ iniciado
```

## Observar system calls

```bash
strace -e trace=write ./miniserv
```

## Limpiar

```bash
make clean
```

## Estructura del proyecto

```text
miniserv/
├── Makefile
├── src/
│   └── server.c
├── include/       ← headers compartidos (semanas futuras)
└── README.md
```

## Autor

- Nombre: _[completar]_
- Curso: Sistemas Operativos
