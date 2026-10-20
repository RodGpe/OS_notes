



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


Defina formalmente qué es un programa y qué es un proceso.
Si ejecuta el mismo binario dos veces, ¿se crean uno o dos procesos? Justifique su respuesta


Explique la diferencia conceptual entre concurrencia y paralelismo

¿Por qué una configuración de arranque dual (dual boot) no se considera una forma de virtualización?

Qué tipo de estructura del kernel utiliza el sistema operativo Linux?


Si ejecutas ls | grep ".c"
 ¿cuantos procesos participan?
¿Qué proceso escribe en el pipe? 
¿Qué proceso lee del pipe?
¿que tipo de pipe se utiliza, anónimo o con nombre?



Considere el siguiente programa:

\begin{lstlisting}
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    fork();
    fork();
    execlp("echo", "echo", "hola mundo", (char *)NULL);
    fork();
    return 0;
}
\end{lstlisting}

    Dibuje el árbol de procesos generado por el programa e indique cuántos
    procesos existen al finalizar.




Describe qué ocurre durante el POST (Power-On Self-Test) en el arranque del sistema. PREGUNTA EXTRA