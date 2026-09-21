| Bit       | Bandera     | Valor | Estado     | ¿Qué significa en tu programa?                                                                  |
| --------- | ----------- | ----- | ---------- | ------------------------------------------------------------------------------------------------- |
| **Bit 0** | **`CF`**    | **0** | Inactiva   | No hay acarreo ni préstamos matemáticos en operaciones sin signo.                               |
| Bit 1     | *Reservado* | *1*   | -          | (Fijo por hardware de Intel).                                                                     |
| **Bit 2** | **`PF`**    | **1** | **Activa** | El último resultado tiene paridad (número par de bits en 1).                                    |
| Bit 3     | *Reservado* | *0*   | -          | (Fijo por hardware).                                                                              |
| **Bit 4** | **`AF`**    | **0** | Inactiva   | No hay acarreo auxiliar (usado para aritmética BCD).                                             |
| Bit 5     | *Reservado* | *0*   | -          | (Fijo por hardware).                                                                              |
| **Bit 6** | **`ZF`**    | **1** | **Activa** | La última operación dio exactamente**cero**.                                                    |
| **Bit 7** | **`SF`**    | **0** | Inactiva   | El resultado anterior fue**positivo**(o cero).                                                    |
| **Bit 8** | **`TF`**    | **0** | Inactiva   | El modo de depuración por pasos de hardware está apagado.                                       |
| **Bit 9** | **`IF`**    | **1** | **Activa** | El procesador tiene permiso de responder a las interrupciones del sistema (teclado, disco, etc.). |
