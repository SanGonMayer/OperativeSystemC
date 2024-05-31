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

void retardo_respuesta_memoria();

void cargar_instrucciones(t_dictionary* memoria_archivo, uint32_t pid, char* path_instrucciones);

char* leer_instruccion(t_dictionary* memoria_archivo, uint32_t pid, uint32_t* pc);

void liberar_instrucciones(t_dictionary* memoria_archivo, uint32_t pid);

void procesar_carga_instrucciones(int socket);
void procesar_pedido_instruccion(int socket);

#endif