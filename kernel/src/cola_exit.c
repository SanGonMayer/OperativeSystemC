#include "cola_exit.h"
#include "recursos.h"
#include "utils/procesos.h"
#include "utils/peticiones_memoria.h"
#include "utils/codigo_operacion.h"
#include "global_kernel.h"
#include <commons/collections/list.h>
#include <semaphore.h>

static t_queue* cola_exit;
static sem_t mutex_cola_exit;
static sem_t hay_elementos_en_exit;
static t_list* procesos_finalizados;


void quitar_proceso_de_memoria(int pid);

void init_cola_exit(){
    procesos_finalizados = list_create();
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
        list_add(procesos_finalizados, pcb);
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


t_list* pids_exit(){
    // retorna una lista de ints con los pids de los procesos en la cola de exit y los finalizados
    // sin hacer pop de la cola

    t_list* pids = list_create();
    sem_wait(&mutex_cola_exit);
    if(!queue_is_empty(cola_exit)){
        t_list_iterator* iterator = list_iterator_create(cola_exit->elements);
        while(list_iterator_has_next(iterator)){
            t_PCB* pcb = list_iterator_next(iterator);
            list_add(pids, (void*)pcb->PID);
        }
        list_iterator_destroy(iterator);
    }
    sem_post(&mutex_cola_exit);

    if(!list_is_empty(procesos_finalizados)){
        t_list_iterator* iterator = list_iterator_create(procesos_finalizados);
        while(list_iterator_has_next(iterator)){
            t_PCB* pcb = list_iterator_next(iterator);
            list_add(pids, (void*)pcb->PID);
        }
        list_iterator_destroy(iterator);
    }

    return pids;
}