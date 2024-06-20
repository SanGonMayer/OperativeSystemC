#include "cola_exit.h"
#include "utils/procesos.h"
#include <semaphore.h>

static t_queue* cola_exit;
static sem_t mutex_cola_exit;
static sem_t hay_elementos_en_exit;

void init_cola_exit(){
    cola_exit = queue_create();
    sem_init(&mutex_cola_exit, 0, 1);
    sem_init(&hay_elementos_en_exit, 0, 0);
}

void agregar_a_cola_exit(t_PCB* pcb){
    sem_wait(&mutex_cola_exit);
    queue_push(cola_exit, pcb);
    sem_post(&mutex_cola_exit);
    sem_post(&hay_elementos_en_exit);
}

t_PCB* obtener_de_cola_exit(int pid){
    sem_wait(&mutex_cola_exit);
    t_PCB* pcb = NULL;
    for(int i = 0; i < queue_size(cola_exit); i++){
        t_PCB* pcb_aux = queue_pop(cola_exit);
        if(pcb_aux->PID == pid){
            pcb = pcb_aux;
        }
        queue_push(cola_exit, pcb_aux);
    }
    sem_post(&mutex_cola_exit);
    return pcb;
}

bool esta_en_cola_exit(int pid){
    sem_wait(&mutex_cola_exit);
    bool esta = false;
    for(int i = 0; i < queue_size(cola_exit); i++){
        t_PCB* pcb = queue_pop(cola_exit);
        if(pcb->PID == pid){
            esta = true;
        }
        queue_push(cola_exit, pcb);
    }
    sem_post(&mutex_cola_exit);
    return esta;
}

void procesar_cola_exit(){

    while(1){
        sem_wait(&hay_elementos_en_exit);
        sem_wait(&mutex_cola_exit);
        t_PCB* pcb = queue_pop(cola_exit);
        sem_post(&mutex_cola_exit);

    }
}

void crear_hilo_cola_exit(){
    pthread_t hilo_cola_exit;
    pthread_create(&hilo_cola_exit, NULL, (void*)procesar_cola_exit, NULL);
    pthread_detach(hilo_cola_exit);
}