#include "kernel.h"

int contadorPID = 1;

t_PCB* crear_PCB(){
    t_PCB* pcb = malloc(sizeof(t_PCB));
    pcb -> PID = 1;
    pcb -> estado = NEW;
    return pcb;
}


//aca recibe manda mensaje a memoria y recibe direccion
uint32_t enviar_path_a_memoria(char* path){
    return 5;
}

void iniciar_proceso(char* path, t_queue* cola_new){
    t_PCB* PCB = crear_PCB();
    PCB->path = path;
    queue_push(cola_new, PCB);
}

void enviar_proceso_a_ready(t_queue* cola_new, t_queue* cola_ready){
        t_PCB* pcb = queue_pop(cola_new);
        queue_push(cola_ready, pcb);
        pcb ->registrosMem->codigo = enviar_path_a_memoria(pcb->path);
        pcb -> estado = READY;
}

