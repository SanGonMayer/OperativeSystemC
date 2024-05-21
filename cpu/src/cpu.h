#ifndef CPU_H_
#define CPU_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <commons/config.h>
#include <utils/server.h>
#include <utils/client.h>
#include <utils/procesos.h>
#include <pthread.h>
#include <utils/codigo_operacion.h>

/**
* @fn    etapa_fetch
* @brief pide la instruccion a partir de una posicion de memoria, devuelve instruccion
*/

char* etapa_fetch(int socket, t_PCB* pcb, t_log* logger);

int responder_ok(int socket, uint32_t posicionDeCodigo);

char* recibir_instruccion(int socket);

char* pedir_instruccion(int socket, t_PCB* pcb, t_log* logger);

#endif