#!/bin/bash

# Navegar a la carpeta 'memoria' y ejecutar el comando make memcheck en una terminal
x-terminal-emulator -e "cd memoria && make memcheck" &

# Esperar unos segundos para dar tiempo a que se abra la primera terminal
sleep 2

# Navegar a la carpeta 'cpu' y ejecutar el comando make memcheck en una segunda terminal
x-terminal-emulator -e "cd cpu && make memcheck" &

# Esperar unos segundos para dar tiempo a que se abra la segunda terminal
sleep 2

# Navegar a la carpeta 'kernel' y ejecutar el comando make memcheck en una tercera terminal
x-terminal-emulator -e "cd kernel && make memcheck" &