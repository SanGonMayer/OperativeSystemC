#ifndef MEMORIA_H_
#define MEMORIA_H_
#include "utils/instrucciones.h"
#include <utils/procesos.h>
#include <utils/buffer.h>
#include <commons/log.h>

typedef struct{
    void* memoria;
    uint32_t tam_memoria;
    uint32_t tam_pagina;
    uint32_t cantidad_marcos;
} t_memoria;

typedef struct{
    

}t_tabla_paginas;

typedef struct{
    uint32_t PID;
    uint32_t path_length;
    char* path;
}t_paqueteMemoria;

t_paqueteMemoria* inicializar_paquete_memoria();

void recibir_contexto_de_kernel(int socket, t_paqueteMemoria* paqueteMemoria, t_log*logger);

void confirmar_recepcion(int socket);

void retardo_respuesta_memoria();

#endif