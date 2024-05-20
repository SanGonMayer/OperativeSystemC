#ifndef MEMORIA_H_
#define MEMORIA_H_
#include "utils/instrucciones.h"
#include <utils/procesos.h>
#include <utils/buffer.h>
#include <commons/log.h>

typedef struct{
    uint32_t PID;
    uint32_t path_length;
    char* path;
}t_paqueteMemoria;

t_paqueteMemoria* inicializar_paquete_memoria();

void recibir_contexto_de_kernel(int socket, t_paqueteMemoria* paqueteMemoria, t_log*logger);

void confirmar_recepcion(int socket);

t_paquete_instruccion* recibir_instruccion(int socket, t_log* logger);

void enviar_instruccion(int socket, char* instruccion, t_log* logger);

#endif