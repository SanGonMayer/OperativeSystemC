#ifndef MEMORIA_INSTRUCCIONES_H_
#define MEMORIA_INSTRUCCIONES_H_

#include "utils/instrucciones.h"

typedef struct{
    uint32_t PID;
    uint32_t path_length;
    char* path;
}t_paqueteMemoria;

t_paqueteMemoria* inicializar_paquete_memoria();

void recibir_contexto_de_kernel(int socket, t_paqueteMemoria* paqueteMemoria, t_log*logger);

t_paquete_instruccion* recibir_instruccion(int socket, t_log* logger);

void enviar_instruccion(int socket, char* instruccion, t_log* logger);

void cargar_instrucciones(uint32_t pid, char* path_instrucciones);

char* leer_instruccion(uint32_t pid, uint32_t* pc);

void liberar_instrucciones(uint32_t pid);

void procesar_carga_instrucciones(int socket);
void procesar_pedido_instruccion(int socket);



#endif