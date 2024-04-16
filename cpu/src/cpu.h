#ifndef CPU_H_
#define CPU_H_

#include <stdlib.h>
#include <stdio.h>
#include <commons/config.h>
#include <utils/server.h>
#include <utils/client.h>

t_log* logger;
t_config* config;

char* ip_memoria;
char* puerto_memoria;
char* puerto_escucha_dispatch;
char* puerto_escucha_interrupt;
uint32_t cantidad_entradas_tlb;
char* algoritmo_tlb;

/**
* @fn    iniciar_cpu
* @brief esta funcion nos devuelve un fd de escucha.
*/

#endif