
## Programa principal (padre)

```
FUNCIÓN main():
    listen_fd ← crear socket TCP (IPv4)
    asociar listen_fd a “cualquier IP local” + puerto 8080
    marcar listen_fd como socket en escucha (cola de conexiones pendientes)

    REPETIR para siempre:
        recolectar_hijos_terminados()     // no bloqueante; evita zombies

        client_fd ← esperar nueva conexión en listen_fd   // puede bloquear

        pid ← crear proceso hijo (fork)

        SI soy el HIJO (pid == 0):
            cerrar listen_fd en el hijo    // no debe escuchar ni aceptar
            atender_cliente(client_fd)
            terminar proceso hijo con código 0   // _exit, no exit de stdio

        ELSE
        cerrar client_fd en el padre       // el hijo es quien usa ese fd
        recolectar_hijos_terminados()      // por si el hijo ya terminó muy rápido
    FIN REPETIR
```

---

## Programa principal (padre) — versión que acumula zombies

Misma lógica que arriba, pero **sin** `recolectar_hijos_terminados()` y con **sleep** en el padre para que, cuando el hijo ya terminó (`_exit`), el kernel conserve su entrada en estado **Z** hasta que el padre haga `wait`/`waitpid`.

**Cómo observarlo:** conectar con `nc 127.0.0.1 8080`, cerrar la sesión (el hijo termina), mientras el padre duerme ejecutar `ps -o pid,ppid,stat,cmd --ppid <pid_padre>` → `STAT` = `Z` o `Z+`, `<defunct>`.

``` pseudocode
FUNCIÓN main():
    listen_fd ← crear socket TCP (IPv4)
    asociar listen_fd a “IP local” + puerto 8080
    marcar listen_fd como socket en escucha (cola de conexiones pendientes)

    REPETIR para siempre:
        client_fd ← esperar nueva conexión en listen_fd   // puede bloquear

        pid ← crear proceso hijo (fork)

        SI soy el HIJO (pid == 0):
            cerrar listen_fd en el hijo
            atender_cliente(client_fd)   // al cerrar el cliente, _exit(0)
            //dormir(3 segundos)
            terminar proceso hijo con código 0
        cerrar client_fd 
        //dormir(20 segundos)  
    FIN REPETIR
```



Preguntas
cuantos procesos del programa del pseudo código de arriba habrá despues de ejecutar `for i in 1 2 3; do nc localhost 8080 & done; wait`. wait hace que el commando `nc` se ejecuté de manera concurrente

diagrama el arbol genealogico de los procesos creados con el comando de la pregunta anterior

supongamos que las lineas tal y tal están descomentadas, cómo puedo afectar esto en la generación de un zombie.

supongamos que la linea tal está comentada, los procesos pueden seguir ejecutandose? 

