#include "config_io.h"
#include "global_io.h"
#include "io.h"
#include "io_generica.h"
#include "io_stdin.h"
#include "io_stdout.h"
#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <readline/readline.h>
#include <stdlib.h>


t_config* config;

void procesarParametros(int argc, char* argv[], char** param1, char** param2) {
    if (argc > 1) {
        *param1 = argv[1];
    }

    if (argc >= 2) {
        *param2 = argv[2];
    }
}

void solicitar_config_path(char** path_param) {
    char* input = readline("Ingrese el config path: ");
    if (input != NULL) {
        *path_param = malloc((strlen(input) + 1) * sizeof(char));
        strcpy(*path_param, input);
        free(input);
    }
}

void solicitar_nombre(char** nombre_param) {
    char* input = readline("Ingrese el nombre de la interfaz: ");
    if (input != NULL) {
        *nombre_param = malloc((strlen(input) + 1) * sizeof(char));
        strcpy(*nombre_param, input);
        free(input);
    }
}

int main(int argc, char* argv[]){
    char *nombre = NULL;
    char *config_path = NULL;

    procesarParametros(argc, argv, &nombre, &config_path);

    if (nombre == NULL) {
        solicitar_nombre(&nombre);
    }

    if (config_path == NULL) {
        solicitar_config_path(&config_path);
    }

    string_trim(&config_path);

	g_logger = log_create(
        "io.log", 
        "server_connection",
        1,
        LOG_LEVEL_INFO);
    
    config = config_create(config_path);

    if(config == NULL){
        log_error(g_logger, "Mal el path");
        exit(EXIT_FAILURE);
    }
    g_config_io = leer_configuracion(config);

    char* tipo = g_config_io->tipo_interfaz;

    int conexion_kernel = iniciar_conexion_kernel(g_config_io->tipo_interfaz, nombre);

    void* estrategia_procesar_instruccion = NULL;

    if(string_equals_ignore_case(tipo, "GEN")){
        estrategia_procesar_instruccion = &procesar_instruccion_generica;
    }

    if(string_equals_ignore_case(tipo, "STDIN")){
        g_socket_memoria = iniciar_conexion_memoria();
        estrategia_procesar_instruccion = &procesar_instruccion_stdin;
    }

    if(string_equals_ignore_case(tipo, "STDOUT")){
        g_socket_memoria = iniciar_conexion_memoria();
        estrategia_procesar_instruccion = &procesar_instruccion_stdout;
    }

    atender_instrucciones(conexion_kernel, estrategia_procesar_instruccion);

    log_destroy(g_logger);
    configuracionIO_destroy(g_config_io);
    config_destroy(config);
    return 0;
}


