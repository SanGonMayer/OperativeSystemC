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

typedef enum {
    EJECUTAR_SCRIPT= 1,
    INICIAR_PROCESO= 2,
    FINALIZAR_PROCESO= 3,
    DETENER_PLANIFICACION= 4,
    INICIAR_PLANIFIACION= 5,
    MULTIPROGRAMACION =6, 
    PROCESO_ESTADO= 7,
} t_funciones_consola;

t_paquete* crear_contexto_memoria(t_PCB* pcb);

void enviar_contexto_memoria(t_paquete* paquete, int socket, t_log*logger);

t_registrosMem recibir_contexto_memoria(int socket);

void enviar_proceso_a_memoria(t_PCB* pcb, int socketMemoria, t_log* logger);

void iniciar_proceso(char* path, t_queue* cola_new, int* contadorPID);

void enviar_proceso_a_ready();

void ejecutar_cpu_FIFO(t_PCB* pcb, int conexion_cpu_dispatch, t_log* logger);

void consola_interactiva(t_log *logger);
   
bool es_parametro_valido(char* parametro);

#endif