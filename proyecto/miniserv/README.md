# MiniServ

Servidor TCP/HTTP multiproceso en C sobre Linux. Proyecto incremental del curso de **Sistemas Operativos**.

Las versiones viven en **carpetas** (`v0`, `v1`, `v2`, …): cada una compila su propio binario `miniserv` dentro de esa carpeta.

## Versiones

| Carpeta | Semana | Descripción |
| ------- | ------ | ----------- |
| [`v0/`](v0/) | 1 | Imprime `MiniServ iniciado` y termina |
| [`v1/`](v1/) | 2 | TCP echo secuencial, loop `accept`, puerto 8080 |
| [`v2/`](v2/) | 2–3 | **HTTP** secuencial (respuesta mínima a `GET`) |
| [`v3/`](v3/) | 3 | TCP echo **fork-per-client** (`fork`, `waitpid`) |

La línea del curso es **V1 → V2 → V3**: bytes crudos, HTTP en un solo proceso, luego concurrencia con `fork`. Combinar HTTP + fork en una sola carpeta puede ser extensión o milestone posterior.

## Uso

**V1 (TCP echo):**

```bash
cd v1 && make && ./miniserv
echo hola | nc 127.0.0.1 8080
```

**V2 (HTTP):**

```bash
cd v2 && make && ./miniserv
curl -v http://127.0.0.1:8080/ruta
```

**V3 (fork):**

```bash
cd v3 && make && ./miniserv
```

## Requisitos

- Linux, `gcc`, `make`
- `curl`, `nc`, `strace`, `ss` (según la semana)

## Estructura

```text
miniserv/
├── README.md
├── include/
├── v0/
├── v1/
├── v2/
└── v3/
```

Compila siempre desde la carpeta de la versión que quieras probar.
