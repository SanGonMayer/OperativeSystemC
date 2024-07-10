# Navegar a la carpeta 'io' y ejecutar el comando make memcheck en una cuarta terminal, luego abrir un bash interactivo
x-terminal-emulator -e "bash -c 'cd entradasalida && make && ./bin/entradasalida MONITOR ../entradasalida/io_stdout.config; exec bash'" &

# Esperar unos segundos para dar tiempo a que se abra la cuarta terminal
sleep 2

# (Parece que hay un duplicado, eliminé la segunda repetición para la carpeta 'io')
# Navegar a la carpeta 'io' y ejecutar el comando make memcheck en una cuarta terminal, luego abrir un bash interactivo
x-terminal-emulator -e "bash -c 'cd entradasalida && make && ./bin/entradasalida TECLADO ../entradasalida/io_stdin.config; exec bash'" &

# Esperar unos segundos para dar tiempo a que se abra la cuarta terminal
sleep 2

# (Parece que hay un duplicado, eliminé la segunda repetición para la carpeta 'io')
# Navegar a la carpeta 'io' y ejecutar el comando make memcheck en una cuarta terminal, luego abrir un bash interactivo
x-terminal-emulator -e "bash -c 'cd entradasalida && make && ./bin/entradasalida GENERICA ../entradasalida/generica.config; exec bash'" &

# Esperar unos segundos para dar tiempo a que se abra la cuarta terminal
sleep 2

# (Parece que hay un duplicado, eliminé la segunda repetición para la carpeta 'io')
# Navegar a la carpeta 'io' y ejecutar el comando make memcheck en una cuarta terminal, luego abrir un bash interactivo
x-terminal-emulator -e "bash -c 'cd entradasalida && make && ./bin/entradasalida SLP1 ../entradasalida/slp1.config; exec bash'" &

# Esperar unos segundos para dar tiempo a que se abra la cuarta terminal
sleep 2

# (Parece que hay un duplicado, eliminé la segunda repetición para la carpeta 'io')
# Navegar a la carpeta 'io' y ejecutar el comando make memcheck en una cuarta terminal, luego abrir un bash interactivo
x-terminal-emulator -e "bash -c 'cd entradasalida && make && ./bin/entradasalida ESPERA ../entradasalida/espera.config; exec bash'" &