#include "config_io.h"
#include "global_io.h"
#include "io.h"
#include "io_generica.h"
#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <stdlib.h>


t_config* config;

int main(int argc, char* argv[]){

	g_logger = log_create(
        "io.log", 
        "server_connection",
        1,
        LOG_LEVEL_INFO);
    
    config = config_create("../entradasalida/io.config");

    if(config == NULL){
        log_error(g_logger, "Mal el path");
        exit(EXIT_FAILURE);
    }
    g_config_io = leer_configuracion(config);

    char* tipo = g_config_io->tipo_interfaz;

    int conexion_kernel = iniciar_conexion_kernel(tipo);

    void* estrategia_procesar_instruccion = NULL;

    if(string_equals_ignore_case(tipo, "GEN")){
        estrategia_procesar_instruccion = &procesar_instruccion_generica;
    }

    atender_instrucciones(conexion_kernel, estrategia_procesar_instruccion);

    log_destroy(g_logger);
    configuracionIO_destroy(g_config_io);
    config_destroy(config);
    return 0;
}


