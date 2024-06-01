
#include "config.h"
#include "global_memoria.h"
#include <commons/config.h>
#include <commons/log.h>
#include <stdlib.h>
#include <string.h>

static t_config_memoria* config_memoria = NULL;

void log_config(t_config_memoria* config){
    log_info(g_logger, "PUERTO_ESCUCHA: %s", config->puerto_escucha);
    log_info(g_logger, "TAM_MEMORIA: %d", config->tam_memoria);
    log_info(g_logger, "TAM_PAGINA: %d", config->tam_pagina);
    log_info(g_logger, "PATH_INSTRUCCIONES: %s", config->path_instrucciones);
    log_info(g_logger, "RETARDO_RESPUESTA: %d", config->retardo_respuesta);
}

t_config_memoria* config_memoria_create(char* path){

    config_memoria = malloc(sizeof(t_config_memoria));
    t_config* config = config_create(path);
    char* puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");
    config_memoria->puerto_escucha = malloc(strlen(puerto_escucha) + 1);
    strcpy(config_memoria->puerto_escucha, puerto_escucha);

    config_memoria->tam_memoria = config_get_int_value(config, "TAM_MEMORIA");
    config_memoria->tam_pagina = config_get_int_value(config, "TAM_PAGINA");
    char * path_instrucciones = config_get_string_value(config, "PATH_INSTRUCCIONES");
    config_memoria->path_instrucciones = malloc(strlen(path_instrucciones) + 1);
    strcpy(config_memoria->path_instrucciones, path_instrucciones);
    config_memoria->retardo_respuesta = config_get_int_value(config, "RETARDO_RESPUESTA");
    config_destroy(config);

    log_config(config_memoria);
    return config_memoria;
}

t_config_memoria* get_config_memoria(){
    if(config_memoria == NULL){
        config_memoria = config_memoria_create("./memoria.config");
    }

    return config_memoria;
}

void config_memoria_destroy(){
    free(config_memoria->puerto_escucha);
    free(config_memoria->path_instrucciones);
    free(config_memoria);
    config_memoria = NULL;
}