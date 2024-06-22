#include "cola_exit.h"
#include "recursos.h"
#include "utils/procesos.h"
#include "utils/peticiones_memoria.h"
#include "utils/codigo_operacion.h"
#include "global_kernel.h"
#include <semaphore.h>

static t_queue* cola_exit;
static sem_t mutex_cola_exit;
static sem_t hay_elementos_en_exit;

void quitar_proceso_de_memoria(int pid);

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
        quitar_proceso_de_memoria(pcb->PID);
        liberar_recursos_proceso(pcb);
    }
}

void crear_hilo_cola_exit(){
    pthread_t hilo_cola_exit;
    pthread_create(&hilo_cola_exit, NULL, (void*)procesar_cola_exit, NULL);
    pthread_detach(hilo_cola_exit);
}

void quitar_proceso_de_memoria(int pid){
    t_peticion_finalizar_proceso* peticion = crear_peticion_finalizar_proceso(pid);
    t_buffer* buffer = serializar_peticion_finalizar_proceso(peticion);
    t_paquete* paquete = crear_paquete(FINALIZAR_PROCESO_MEMORIA, buffer);

    enviar_paquete(paquete, g_socket_memoria);

    eliminar_paquete(paquete);
    destruir_peticion_finalizar_proceso(peticion);

    bool resultado = recibir_ok(g_socket_memoria);

    if(resultado){
        log_info(g_logger, "Proceso %d eliminado de memoria", pid);
    } else {
        log_error(g_logger, "Error al eliminar proceso %d de memoria", pid);
    }
}