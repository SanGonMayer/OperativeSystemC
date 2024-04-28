#ifndef KERNEL_H_
#define KERNEL_H_

#include <stdlib.h>
#include <stdio.h>
#include <commons/config.h>
#include <utils/client.h>
#include <utils/server.h>
#include <unistd.h>
#include <commons/collections/queue.h>

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
} t_PCB;

void iniciar_proceso(char* path, t_queue* cola_new);

void eliminar_proceso();

uint32_t enviar_path_a_memoria();

t_PCB* crear_PCB();

#endif