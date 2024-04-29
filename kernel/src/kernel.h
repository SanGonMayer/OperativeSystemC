#ifndef KERNEL_H_
#define KERNEL_H_

#include <stdlib.h>
#include <stdio.h>
#include <commons/config.h>
#include <utils/client.h>
#include <utils/server.h>
#include <unistd.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <utils/procesos.h>

typedef enum {
    FIFO= 1,
    RR= 2,
    VRR= 3,
} t_algoritmoPlanificacion;

void iniciar_proceso(char* path, t_queue* cola_new);

uint32_t enviar_path_a_memoria(char* path);

void enviar_proceso_a_ready(t_queue* cola_new,t_queue* cola_ready);

void ejecutar_cpu_FIFO(t_PCB* pcb, int conexion_cpu_dispatch, t_log* logger);

#endif