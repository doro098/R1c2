# R1c2

[![Architecture diagram](https://gitdiagram.com/diagram-badge.svg)](https://gitdiagram.com/doro098/r1c2?utm_source=readme&utm_medium=badge)

Repositorio entrega cuenta 2

Trabajo de Engler Lisandro Francisco (`lichaengler`). El sketch original se
recuperó del issue #1 del repositorio del alumno:
https://github.com/lichaengler/R1c2/issues/1

## Estructura del sketch

| Archivo | Qué hace |
|---|---|
| `engler.ino` | `setup()` y `loop()`: el flujo principal. |
| `config.h` | Pines, sensibilidad, umbrales y tiempos. |
| `Giroscopio.h/.cpp` | MPU6050 por I2C: calibración, lectura suavizada y delta del mouse. |
| `Entrada.h/.cpp` | Botones, joystick y el estado de START (toque corto / mantener). |
| `Salida.h/.cpp` | Mouse y teclado: avisa a la PC sólo cuando algo cambia. |

Es el mismo código del issue, separado para poder leerlo por partes. Placa:
**Arduino Leonardo**.
