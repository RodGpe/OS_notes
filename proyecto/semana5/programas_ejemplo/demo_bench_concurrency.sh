#!/usr/bin/env bash
# demo_bench_concurrency.sh — compara latencia total: secuencial (V1) vs fork-per-client (V4)
#
# Uso:
#   ./demo_bench_concurrency.sh
#   N=30 ./demo_bench_concurrency.sh
#
# Requisitos: miniserv compilado en v1 y v4, nc, time (bash builtin)
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
N="${N:-20}"
HOST="${HOST:-127.0.0.1}"
PORT_V1=8082
PORT_V4=8080

V1="$ROOT/miniserv/v1/miniserv"
V4="$ROOT/miniserv/v4/miniserv"

if [[ ! -x "$V1" ]]; then
    echo "Compilando V1..." >&2
    make -C "$ROOT/miniserv/v1" -s
fi
if [[ ! -x "$V4" ]]; then
    echo "Compilando V4..." >&2
    make -C "$ROOT/miniserv/v4" -s
fi

run_clients() {
    local port=$1
    local n=$2
    seq 1 "$n" | xargs -P"$n" -I{} sh -c \
        'printf "cliente {}\n" | nc -q 2 '"$HOST"' '"$port"' >/dev/null' _
}

bench_one() {
    local label=$1
    local bin=$2
    local port=$3

    echo "=== $label (puerto $port, $N clientes en paralelo) ==="
    "$bin" &
    local srv_pid=$!
    sleep 0.3

    local t0 t1
    t0=$(date +%s.%N)
    run_clients "$port" "$N"
    t1=$(date +%s.%N)

    kill -TERM "$srv_pid" 2>/dev/null || true
    wait "$srv_pid" 2>/dev/null || true

    awk -v a="$t0" -v b="$t1" 'BEGIN { printf "Tiempo wall aprox: %.3f s\n", b - a }'
    echo
}

bench_one "MiniServ V1 secuencial" "$V1" "$PORT_V1"
bench_one "MiniServ V4 fork-per-client" "$V4" "$PORT_V4"

echo "Nota pedagógica: V1 atiende un cliente a la vez; V4 crea un proceso por conexión."
echo "Con N grande, V4 también tiene costo: fork, context switches, memoria."
