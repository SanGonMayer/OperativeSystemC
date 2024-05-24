#ifndef PROCESOS_H_
#define PROCESOS_H_

#include <stdlib.h>
#include <stdio.h>
#include <commons/config.h>
#include <utils/client.h>
#include <utils/server.h>
#include <unistd.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <commons/log.h>
#include <utils/buffer.h>
#include <pthread.h>


typedef struct{
    uint32_t pc;
    uint8_t  ax;
    uint8_t  bx;
    uint8_t  cx;
    uint8_t  dx;
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t si;
    uint32_t di;

} t_registrosCPU;

typedef struct{
    uint32_t codigo;
    uint32_t datos;
    uint32_t heap;
    uint32_t posicionFinal;
} t_registrosMem;

typedef enum {
    NEW = 1,
    READY = 2,
    EXEC = 3,
    BLOCKED = 4,
    EXIT = 5
} t_estadoProceso;

typedef struct{
    uint32_t PID;
    t_estadoProceso estado;
    t_registrosCPU registrosCPU;
    t_registrosMem registrosMem;
    int quantum;
    uint32_t path_length;
    char* path;
} t_PCB;

t_PCB* crear_PCB();

t_buffer* serializar_pcb(t_PCB* pcb);

t_PCB* deserializar_pcb(t_buffer* buffer);

int enviar_pcb(int socket, t_PCB *pcb);

void responder_pcb(int socket, t_PCB *pcb, t_log* logger);

void actualizar_pcb(t_PCB *pcb_viejo, const t_PCB *pcb_nuevo);

void* crear_a_enviar(t_paquete* paquete);

t_PCB* recibir_pcb(int socket);

#endif