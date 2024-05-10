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

/**
* @fn    etapa_fetch
* @brief pide la instruccion a partir de una posicion de memoria, devuelve instruccion
*/

char* etapa_fetch(int socket, t_PCB* pcb, t_log* logger);

int enviar_posicion_de_codigo(int socket, uint32_t posicionDeCodigo);

int recibir_instruccion(int socket,char* instruccion);

#endif