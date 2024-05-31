#ifndef CONFIG_MEMORIA_H_
#define CONFIG_MEMORIA_H_

typedef struct {
    char* puerto_escucha;
    int tam_memoria;
    int tam_pagina;
    char* path_instrucciones;
    int retardo_respuesta;
} t_config_memoria;

t_config_memoria* config_memoria_create(char* path);

t_config_memoria* get_config_memoria();

void config_memoria_destroy();

#endif