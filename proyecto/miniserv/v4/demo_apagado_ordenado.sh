#!/usr/bin/env bash
#
# demo_apagado_ordenado.sh — MiniServ V4: apagado ordenado (no brusco)
#
# Uso (desde miniserv/v4):
#   ./demo_apagado_ordenado.sh
#   ./demo_apagado_ordenado.sh --pause
#   make demo-shutdown
#
set -euo pipefail

DIR="$(cd "$(dirname "$0")" && pwd)"
PORT="${MINISERV_PORT:-8080}"
HOST="127.0.0.1"
LOG="/tmp/miniserv_v4_shutdown_demo.log"
PAUSE=0

[[ "${1:-}" == "--pause" ]] && PAUSE=1

step() {
    echo ""
    echo "========== $* =========="
    if [[ "$PAUSE" -eq 1 ]]; then
        read -r -p "[Enter] "
    fi
}

need() {
    command -v "$1" >/dev/null || {
        echo "Falta: $1" >&2
        exit 1
    }
}

cleanup() {
    [[ -n "${CLIENT_PID:-}" ]] && kill "$CLIENT_PID" 2>/dev/null || true
    if [[ -n "${SERVER_PID:-}" ]]; then
        kill -TERM "$SERVER_PID" 2>/dev/null || true
        wait "$SERVER_PID" 2>/dev/null || true
    fi
}
trap cleanup EXIT INT TERM

need nc
need pgrep
need make

step "1) Compilar"
make -C "$DIR"

if command -v ss >/dev/null 2>&1 && ss -ltn | grep -q ":$PORT "; then
    echo "Puerto $PORT ocupado. Prueba: MINISERV_PORT=8081 $0" >&2
    exit 1
fi

step "2) Arrancar V4 con cliente conectado (hijo en read)"
: > "$LOG"
"$DIR/miniserv" 2>"$LOG" &
SERVER_PID=$!
sleep 0.5

( printf 'demo apagado ordenado\n'; exec sleep 120 ) | nc -q 120 "$HOST" "$PORT" &
CLIENT_PID=$!
sleep 1

WORKER="$(pgrep -P "$SERVER_PID" | head -1 || true)"
echo "Padre PID=$SERVER_PID  Hijo worker PID=${WORKER:-?}"
if [[ -n "$WORKER" ]]; then
    ps -o pid,ppid,stat,cmd -p "$SERVER_PID" -p "$WORKER"
else
    ps -o pid,ppid,stat,cmd -p "$SERVER_PID"
fi

step "3) SIGTERM al padre (como systemd/docker stop)"
kill -TERM "$SERVER_PID"
sleep 0.3
# Cierra el cliente para que el hijo salga de read() y el padre pueda wait() en shutdown.
kill "$CLIENT_PID" 2>/dev/null || true
CLIENT_PID=""
if wait "$SERVER_PID"; then
    echo "wait exit code: 0 (salida ordenada)"
else
    echo "wait exit code: $? (esperado 0)" >&2
    exit 1
fi
SERVER_PID=""

step "4) Mensajes en stderr (evidencia V4)"
cat "$LOG"
echo ""
if grep -q 'apagado ordenado' "$LOG" && grep -q 'terminado' "$LOG"; then
    echo "OK: se ven mensajes de cierre ordenado."
else
    echo "FALLO: faltan mensajes de apagado ordenado en $LOG" >&2
    exit 1
fi

step "5) No deben quedar procesos miniserv de esta demo"
if pgrep -f "$DIR/miniserv" >/dev/null 2>&1; then
    echo "FALLO: aún hay miniserv en ejecución:" >&2
    pgrep -af "$DIR/miniserv" >&2 || true
    exit 1
fi
echo "OK: ningún miniserv colgado."

echo ""
echo "Comparación en clase: repetir con miniserv/v3 y ver que no imprime"
echo "'apagado ordenado' y el árbol de procesos puede quedar inconsistente."
