#ifndef MEMORIA_INSTRUCCIONES_H_
#define MEMORIA_INSTRUCCIONES_H_

#include "utils/instrucciones.h"

t_paquete_instruccion* recibir_instruccion(int socket, t_log* logger);

void enviar_instruccion(int socket, char* instruccion, t_log* logger);

void cargar_instrucciones(uint32_t pid, char* path_instrucciones);

char* leer_instruccion(uint32_t pid, uint32_t* pc);

void liberar_instrucciones(uint32_t pid);

void procesar_carga_instrucciones(int socket);
void procesar_pedido_instruccion(int socket);



#endif