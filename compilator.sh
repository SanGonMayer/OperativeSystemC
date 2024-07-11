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
sleep 5
