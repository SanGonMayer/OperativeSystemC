#include "kernel.h"

int contadorPID = 1;


void iniciar_proceso(char* path, t_queue* cola_new){
    t_PCB* PCB = crear_PCB();
    enviar_path_a_memoria(path);
    queue_push(cola_new, PCB);
}

t_PCB* crear_PCB(){
    t_PCB* pcb = malloc(sizeof(t_PCB));
    pcb -> PID = 1;
    return pcb;
}


//aca recibe manda mensaje a memoria y recibe direccion
uint32_t enviar_path_a_memoria(char* path){
    return 5;
}