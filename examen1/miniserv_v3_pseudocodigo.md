# MiniServ V3 — pseudocódigo (muy alto nivel)

**Modelo:** *fork-per-client* (Semana 3).  
**Comportamiento:** servidor TCP echo en puerto 8080; un proceso **padre** acepta conexiones; cada cliente lo atiende un **proceso hijo** independiente.

---

## Vista general

```text
┌─────────────────────────────────────────────────────────┐
│  PROCESO PADRE (único, vive todo el tiempo)             │
│  listen_fd ──► accept ──► fork ──► cerrar client_fd     │
│              ▲              │                           │
│              │              └──► hijo atiende cliente   │
│              └── loop + recolectar hijos terminados     │
└─────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────┐
│  PROCESO HIJO (uno por conexión aceptada)               │
│  client_fd ──► leer bytes ──► devolverlos (echo)        │
│              ──► cerrar client_fd ──► terminar (_exit)  │
└─────────────────────────────────────────────────────────┘
```

---

## Programa principal (padre)

```
FUNCIÓN main():
    listen_fd ← crear socket TCP (IPv4)
    configurar opción reutilizar dirección (evitar “puerto ocupado” al reiniciar)
    asociar listen_fd a “cualquier IP local” + puerto 8080
    marcar listen_fd como socket en escucha (cola de conexiones pendientes)

    escribir mensaje de arranque en stderr (log del servidor)

    REPETIR para siempre:
        recolectar_hijos_terminados()     // no bloqueante; evita zombies

        client_fd ← esperar nueva conexión en listen_fd   // puede bloquear

        SI falla accept por interrupción:
            continuar al siguiente ciclo
        SI falla accept por otro error:
            abortar servidor

        pid ← crear proceso hijo (fork)

        SI fork falla:
            cerrar client_fd
            continuar al siguiente ciclo

        SI soy el HIJO (pid == 0):
            cerrar listen_fd en el hijo    // no debe escuchar ni aceptar
            atender_cliente(client_fd)
            terminar proceso hijo con código 0   // _exit, no exit de stdio

        // Soy el PADRE
        cerrar client_fd en el padre       // el hijo es quien usa ese fd
        recolectar_hijos_terminados()      // por si el hijo ya terminó muy rápido
    FIN REPETIR
```

---

## Atender un cliente (solo en el hijo)

```
FUNCIÓN atender_cliente(client_fd):
    REPETIR:
        n ← leer hasta BUF_SIZE bytes desde client_fd   // puede bloquear

        SI n > 0:
            escribir todos esos n bytes de vuelta a client_fd (echo)
            SI error al escribir:
                salir del bucle
        SI n == 0:
            // cliente cerró la conexión (fin de stream)
            salir del bucle
        SI n < 0:
            // error de lectura
            salir del bucle
    FIN REPETIR

    cerrar client_fd
FIN FUNCIÓN
```

---

## Recolectar hijos (solo en el padre)

```
FUNCIÓN recolectar_hijos_terminados():
    REPETIR:
        resultado ← waitpid(cualquier_hijo, estado, NO_BLOQUEAR)

        SI resultado indica “un hijo fue recolectado”:
            continuar el bucle interno   // puede haber varios zombies listos
        SI no hay hijo listo todavía:
            salir del bucle interno
    FIN REPETIR
FIN FUNCIÓN

// Propósito: cuando un hijo termina tras atender un cliente, el kernel lo deja
// en estado zombie hasta que el padre hace wait/waitpid. Sin recolectar, se
// acumulan procesos <defunct> y se desperdician PIDs.
// NO_BLOQUEAR permite seguir en accept() sin dormir al padre innecesariamente.
```

---

## Escritura completa (utilidad compartida)

```
FUNCIÓN escribir_todo(fd, buffer, cantidad):
    enviado ← 0
    MIENTRAS enviado < cantidad:
        w ← write(fd, siguiente trozo del buffer)
        SI interrupción:
            reintentar
        SI error:
            fallar
        enviado ← enviado + w
    retornar éxito
FIN FUNCIÓN

// Un solo write() puede transferir menos bytes de los pedidos; el echo debe
// asegurarse de enviar el bloque completo leído.
```

---

## Responsabilidades por rol

| Rol   | Conserva `listen_fd` | Usa `client_fd` | Después de conectar |
|-------|----------------------|-----------------|---------------------|
| Padre | Sí (solo `accept`)   | No (lo cierra)  | Sigue el loop       |
| Hijo  | No (lo cierra)       | Sí (read/write) | Termina al cerrar cliente |

---

## Diferencia respecto a V1

```text
V1: accept → atender cliente (bloqueante) → close → accept   // un cliente a la vez

V3: accept → fork → padre vuelve a accept de inmediato
              └── hijo atiende en paralelo con otros hijos
```

---

## Fuente de referencia en el repo

Implementación C: [`proyecto/miniserv/v3/src/server.c`](proyecto/miniserv/v3/src/server.c)
