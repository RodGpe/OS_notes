#!/usr/bin/env bash
#
# demo_bloque1_apagado_brusco.sh — Bloque 1 Semana 5 (problema motivador)
#
# Muestra qué ocurre si el padre MiniServ V3 termina de golpe (SIGINT)
# mientras un hijo sigue atendiendo a un cliente en read().
#
# Usamos kill -INT al PID del padre (no Ctrl+C en la terminal del servidor):
# Ctrl+C envía SIGINT al *grupo* en foreground y a veces mata padre e hijo a la vez,
# ocultando el problema. Matar solo al padre deja visible al hijo huérfano.
#
# Requisitos: bash, nc (netcat), ps, pstree (opcional), make/gcc para V3.
#
# Uso:
#   ./demo_bloque1_apagado_brusco.sh
#   ./demo_bloque1_apagado_brusco.sh --pause   # pausa entre pasos (clase)
#
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../../.." && pwd)"
V3_DIR="$ROOT/proyecto/miniserv/v3"
LAUNCHER="$ROOT/proyecto/semana4/programas_ejemplo/demo_bloque1_launcher"
PORT="${MINISERV_PORT:-8080}"
HOST="127.0.0.1"
PAUSE=0
SIGNAL=INT

usage() {
    cat <<EOF
Uso: $0 [--pause] [--term]

  --pause  pausa con Enter entre pasos (recomendado en clase)
  --term   usa SIGTERM en lugar de SIGINT (solo si el launcher no compila)

Demo Bloque 1 — Semana 5: apagado brusco de MiniServ V3 con un hijo en read().

Nota: un job en background de bash ignora SIGINT; por eso arrancamos el servidor
vía demo_bloque1_launcher (restaura SIG_DFL). En terminal con ./miniserv en
foreground, Ctrl+C sí envía SIGINT al grupo.
EOF
}

while [[ $# -gt 0 ]]; do
    case "$1" in
    --pause) PAUSE=1 ;;
    --term)  SIGNAL=TERM ;;
    -h|--help) usage; exit 0 ;;
    *) echo "Opción desconocida: $1" >&2; usage; exit 1 ;;
    esac
    shift
done

step() {
    echo ""
    echo "========== $* =========="
    if [[ "$PAUSE" -eq 1 ]]; then
        read -r -p "[Enter para continuar] "
    else
        sleep 1
    fi
}

need_cmd() {
    if ! command -v "$1" >/dev/null 2>&1; then
        echo "Error: falta el comando '$1' en PATH." >&2
        exit 1
    fi
}

cleanup() {
    local ec=$?
    [[ -n "${CLIENT_PID:-}" ]] && kill "$CLIENT_PID" 2>/dev/null || true
    [[ -n "${WORKER_PID:-}" ]] && kill -TERM "$WORKER_PID" 2>/dev/null || true
    [[ -n "${SERVER_PID:-}" ]] && kill -TERM "$SERVER_PID" 2>/dev/null || true
    exit "$ec"
}
trap cleanup EXIT INT TERM

need_cmd nc
need_cmd ps
need_cmd pgrep

if [[ ! -x "$V3_DIR/miniserv" ]]; then
    echo "Compilando MiniServ V3..."
    make -C "$V3_DIR"
fi

SERVER_CMD=("$V3_DIR/miniserv")
if [[ "$SIGNAL" == "INT" ]]; then
    if [[ ! -x "$LAUNCHER" ]]; then
        echo "Compilando demo_bloque1_launcher (SIGINT no ignorado)..."
        gcc -Wall -Wextra "$LAUNCHER.c" -o "$LAUNCHER"
    fi
    SERVER_CMD=("$LAUNCHER" "$V3_DIR/miniserv")
fi

if ss -ltn 2>/dev/null | grep -q ":$PORT "; then
    echo "Error: el puerto $PORT ya está en uso. Prueba MINISERV_PORT=8081 $0" >&2
    exit 1
fi

step "1) Arrancar MiniServ V3 en segundo plano"
"${SERVER_CMD[@]}" 2>/tmp/miniserv_v3_demo.log &
SERVER_PID=$!
sleep 0.5
if ! kill -0 "$SERVER_PID" 2>/dev/null; then
    echo "El servidor no arrancó. Log:" >&2
    cat /tmp/miniserv_v3_demo.log >&2
    exit 1
fi
echo "Padre (listener): PID=$SERVER_PID"
echo "Log stderr: /tmp/miniserv_v3_demo.log"

step "2) Cliente que permanece conectado (hijo en read bloqueante)"
# nc mantiene la conexión abierta; el hijo queda en read() esperando más datos.
( printf 'mensaje demo bloque1\n'; exec sleep 120 ) | nc -q 120 "$HOST" "$PORT" &
CLIENT_PID=$!
sleep 1

WORKER_PID=""
if command -v pgrep >/dev/null; then
    # Hijo directo del padre (worker fork-per-client)
    mapfile -t kids < <(pgrep -P "$SERVER_PID" 2>/dev/null || true)
    if [[ ${#kids[@]} -ge 1 ]]; then
        WORKER_PID="${kids[0]}"
    fi
fi

echo "Cliente nc: PID=$CLIENT_PID"
if [[ -n "$WORKER_PID" ]]; then
    echo "Hijo worker:  PID=$WORKER_PID (PPID debería ser $SERVER_PID)"
    ps -o pid,ppid,stat,cmd -p "$SERVER_PID","$WORKER_PID" 2>/dev/null || true
else
    echo "(Aún no se detectó hijo; espera un poco más o revisa pgrep -P $SERVER_PID)"
    ps -ef | grep -E '[m]iniserv|[n]c '"$HOST" || true
fi

if command -v pstree >/dev/null 2>&1; then
    echo ""
    pstree -p "$SERVER_PID" || true
fi

step "3) Apagado brusco: señal solo al padre (servidor sin handler)"
echo "Comando: kill -$SIGNAL $SERVER_PID"
kill "-$SIGNAL" "$SERVER_PID" 2>/dev/null || true
for _ in $(seq 1 30); do
    kill -0 "$SERVER_PID" 2>/dev/null || break
    sleep 0.1
done
if kill -0 "$SERVER_PID" 2>/dev/null; then
    echo "(El padre no terminó; forzando SIGKILL)"
    kill -KILL "$SERVER_PID" 2>/dev/null || true
fi
wait "$SERVER_PID" 2>/dev/null || true
SERVER_PID=""
sleep 0.5

step "4) Estado tras morir el padre — ¿murió todo el árbol?"
echo "--- ps (miniserv / nc) ---"
ps -ef | grep -E '[m]iniserv|[n]c '"$HOST" || echo "(ningún miniserv en ps)"

if [[ -n "$WORKER_PID" ]] && kill -0 "$WORKER_PID" 2>/dev/null; then
    echo ""
    echo ">>> OBSERVACIÓN CLAVE: el hijo SIGUE VIVO (huérfano; PPID suele pasar a 1):"
    ps -o pid,ppid,stat,cmd -p "$WORKER_PID"
    echo ""
    echo "Preguntas para clase:"
    echo "  • ¿El padre hizo wait() antes de morir? (no → posible zombie breve; huérfano lo adopta init)"
    echo "  • ¿Quién cierra listen_fd y deja de aceptar clientes? (nadie en el padre muerto)"
    echo "  • ¿Qué haría un servidor con SIGTERM/SIGINT + cierre ordenado? (MiniServ V4)"
else
    echo ""
    echo "El hijo ya no está (pudo recibir señal, cerrar el cliente, o terminar el echo)."
    echo "Repite con el padre en foreground y compara: Ctrl+C vs kill -INT <solo-padre>."
fi

step "5) Limpieza"
echo "Cerrando cliente y procesos restantes..."
# cleanup trap hace el resto

echo ""
echo "Fin de la demo."
echo ""
echo "Modo manual (3 terminales, como en semana5.md):"
echo "  A: cd miniserv/v3 && ./miniserv"
echo "  B: nc $HOST $PORT   # dejar conectado tras escribir una línea"
echo "  A: Ctrl+C"
echo "  C: ps -ef | grep miniserv"
echo ""
echo "En clase con este script: ./demo_bloque1_apagado_brusco.sh --pause"
