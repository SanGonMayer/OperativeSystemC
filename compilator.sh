#!/bin/bash

# Navegar a la carpeta 'memoria' y ejecutar el comando make memcheck en una terminal, luego abrir un bash interactivo
x-terminal-emulator -e "bash -c 'cd memoria && make memcheck; exec bash'" &

# Esperar unos segundos para dar tiempo a que se abra la primera terminal
sleep 2

# Navegar a la carpeta 'cpu' y ejecutar el comando make memcheck en una segunda terminal, luego abrir un bash interactivo
x-terminal-emulator -e "bash -c 'cd cpu && make memcheck; exec bash'" &

# Esperar unos segundos para dar tiempo a que se abra la segunda terminal
sleep 2

# Navegar a la carpeta 'kernel' y ejecutar el comando make memcheck en una tercera terminal, luego abrir un bash interactivo
x-terminal-emulator -e "bash -c 'cd kernel && make memcheck; exec bash'" &

# Esperar unos segundos para dar tiempo a que se abra la tercera terminal
sleep 2

# (Parece que hay un duplicado, eliminé la segunda repetición para la carpeta 'io')
# Navegar a la carpeta 'io' y ejecutar el comando make memcheck en una cuarta terminal, luego abrir un bash interactivo
x-terminal-emulator -e "bash -c 'cd entradasalida && ./bin/entradasalida TECLADO ../entradasalida/io_stdin.config; exec bash'" &
# Esperar unos segundos para dar tiempo a que se abra la cuarta terminal
sleep 2
# (Parece que hay un duplicado, eliminé la segunda repetición para la carpeta 'io')
# Navegar a la carpeta 'io' y ejecutar el comando make memcheck en una cuarta terminal, luego abrir un bash interactivo
x-terminal-emulator -e "bash -c 'cd entradasalida && ./bin/entradasalida MONITOR ../entradasalida/io_stdout.config; exec bash'" &
# Esperar unos segundos para dar tiempo a que se abra la cuarta terminal
sleep 2
# (Parece que hay un duplicado, eliminé la segunda repetición para la carpeta 'io')
# Navegar a la carpeta 'io' y ejecutar el comando make memcheck en una cuarta terminal, luego abrir un bash interactivo
x-terminal-emulator -e "bash -c 'cd entradasalida && ./bin/entradasalida GENERICA ../entradasalida/io_generica.config; exec bash'" &
# Esperar unos segundos para dar tiempo a que se abra la cuarta terminal
sleep 2