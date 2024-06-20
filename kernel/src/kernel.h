#ifndef KERNEL_H_
#define KERNEL_H_

#include "utils/instrucciones_io.h"
#include <stdlib.h>
#include <stdio.h>
#include <commons/config.h>
#include <utils/client.h>
#include <utils/server.h>
#include <unistd.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <utils/procesos.h>
#include <utils/codigo_operacion.h>
#include <commons/temporal.h>

typedef enum {
    FIFO= 1,
    RR= 2,
    VRR= 3,
} t_algoritmoPlanificacion;

typedef struct {
    int motivo;
    t_PCB* pcb;
    char* recurso;
} t_desalojo;

typedef struct {
    t_PCB* pcb;
    t_instruccion_io* instruccion;
} t_parametro_cola_interfaz;

typedef struct {
    char* nombre;
    int instancias;
    t_queue* cola;
    sem_t semafor_cola;
} t_recurso;


t_paquete* crear_contexto_memoria(t_PCB* pcb);

void enviar_contexto_memoria(t_paquete* paquete, int socket, t_log*logger);

void enviar_proceso_a_memoria(t_PCB* pcb, int socketMemoria, t_log* logger);

void iniciar_proceso(char* path);

void preparar_proceso_a_ready();

void enviar_proceso_a_ready(t_PCB* pcb);

void ejecutar_cpu_FIFO(t_PCB* pcb, int conexion_cpu_dispatch, t_log* logger);
   
bool es_parametro_valido(char* parametro);

void planificador_fifo();

void finalizar_proceso(t_PCB* pcb);

void atender_desalojo(t_desalojo* desalojo);

void planificador_exit();

void planificador_RR();

void ejecutar_cpu_RR(t_PCB* pcb);

void esperar_quantum(t_PCB* pcb);

void recibir_pcb_desalojado(t_PCB* pcb);

void enviar_interrupcion(int socket_interrupt, uint32_t* PID);

void planificador_VRR();

void planificador_readyplus();

void listar_procesos();

void eliminar_de_lista_blocked_gral(uint32_t pid);

void iniciar_diccionario_y_listas_recursos(char** recursos, char** recursos_instancias);

t_PCB* buscar_pid_en_sistema(uint32_t pid);

t_PCB* buscar_y_eliminar_pid_en_cola(uint32_t pid,t_queue* cola);

t_PCB* buscar_en_diccionario_interfaces(uint32_t pid, t_dictionary* interfaces);

t_PCB* buscar_en_diccionario_recursos(uint32_t pid, t_dictionary* colas_recursos);

void agregar_a_cola_auxiliar(t_PCB* pcb);

#endif