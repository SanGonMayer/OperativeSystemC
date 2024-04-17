#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include "io.h"

ConfiguracionIO* config;
t_log* logger;

int main(int argc, char* argv[]){

	logger = log_create("io.log", "server_connection", 1, LOG_LEVEL_INFO);
    t_config* config_file = config_create("../entradasalida/io.config");
    config = leer_configuracion(logger, config_file);

    int conexion_kernel = crear_conexion(config->ip_kernel, config->puerto_kernel, "KERNEL", logger);
    handshake_cliente(conexion_kernel, logger);
    enviar_mensaje("primer mensaje enviado de cliente io a kernel", conexion_kernel);
    close(conexion_kernel);

    int conexion_memoria = crear_conexion(config->ip_memoria, config->puerto_memoria, "KERNEL", logger);
    handshake_cliente(conexion_memoria, logger);
    enviar_mensaje("primer mensaje enviado de cliente io a memoria", conexion_memoria);
    close(conexion_memoria);

    log_destroy(logger);
    config_destroy(config_file);
    configuracionIO_destroy(config);
    return 0;
}


