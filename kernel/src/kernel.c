#include "kernel.h"

int contadorPID = 1;


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
        pcb->registrosMem.codigo = enviar_path_a_memoria(pcb->path);
        pcb-> estado = READY;
}

void ejecutar_cpu_FIFO(t_PCB* pcb, int conexion_cpu_dispatch, t_log* logger){
    t_PCB* pcb_auxiliar = crear_PCB();
    int err;
    
    err = enviar_pcb(conexion_cpu_dispatch, pcb);
    //log_info(logger, "PCB ENVIADA: %d", pcb->PID);
    
    err = recibir_pcb(conexion_cpu_dispatch, pcb_auxiliar);
    
    actualizar_pcb(pcb, pcb_auxiliar);
    //log_info(logger, "PCB RECIBIDA: %d", pcb->PID);
    free(pcb_auxiliar);
}