#include "kernel.h"
#include "utils/buffer.h"
#include "utils/client.h"
#include "utils/codigo_operacion.h"
#include <commons/collections/dictionary.h>
#include <commons/log.h>
#include <commons/string.h>
#include <semaphore.h>
#include <string.h>
#include <readline/readline.h>
#include <stdio.h>
#include "global_kernel.h"
#include "utils/instrucciones_io.h"
#include "utils/server.h"

bool es_parametro_valido(char* parametro){

    if(parametro == NULL){
        return false;
    }

    char* cadena_auxiliar = string_duplicate(parametro);

    string_trim(&cadena_auxiliar);
    if(string_is_empty(cadena_auxiliar)){
        return false;
    }

    free(cadena_auxiliar);
    return true;
}

t_paquete* crear_contexto_memoria(t_PCB* pcb){
    t_paquete* paquete = malloc(sizeof(t_paquete));
    t_buffer* buffer = buffer_create(
        sizeof(uint32_t)+
        sizeof(uint32_t)+
        pcb->path_length
    );
    paquete->codigo_operacion = ENVIO_PATH_INSTRUCCIONES;
    paquete->buffer = buffer;
    buffer_add_uint32(paquete->buffer, pcb->PID);
    buffer_add_string(paquete->buffer, pcb->path_length, pcb->path);

    return paquete;
}

//aca recibe manda mensaje a memoria y recibe direccion
void enviar_contexto_memoria(t_paquete* paquete, int socket, t_log*logger){

    int result = serializar_y_enviar_paquete(paquete, socket);
    
    if (result == -1)
        log_error(logger, "Error al enviar contexto a memoria");

    // No nos olvidamos de liberar la memoria que ya no usaremos
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);

}

void enviar_proceso_a_memoria(t_PCB* pcb, int socketMemoria, t_log* logger){
    t_paquete* paquete = crear_contexto_memoria(pcb);
    enviar_contexto_memoria(paquete, socketMemoria, logger);
    
    // Recibir OK
    bool success = recibir_ok(socketMemoria);
    if(!success){
        log_error(logger, "Error al recibir el OK de memoria");
    }
}

void iniciar_proceso(char* path){
    t_PCB* pcb = crear_PCB();
    log_info(g_logger,"Creando PCB para el proceso %d", g_contador_pid);
    sem_wait(&mutex_contador_pid);
    g_contador_pid++;
    pcb->PID = g_contador_pid;
    sem_post(&mutex_contador_pid);
    log_info(g_logger,"Proceso %d creado", pcb->PID);
    pcb->estado = NEW;
    pcb->path_length = strlen(path) + 1;
    pcb->path = malloc(pcb->path_length);
    strcpy(pcb->path, path);
    sem_wait(&g_mutex_cola_new);
    log_info(g_logger, "Encolando proceso %d en NEW, dentro de mutex", pcb->PID);
    queue_push(g_cola_new, pcb);
    sem_post(&g_mutex_cola_new);
    log_info(g_logger, "Cantidad de procesos en NEW: %d", queue_size(g_cola_new));
    log_info(g_logger, "Proceso %d encolado en NEW", pcb->PID);

    sem_wait(&g_tope_multiprogramacion);
    preparar_proceso_a_ready();
    sem_post(&g_hay_elementos_en_ready);
}


void preparar_proceso_a_ready(){
    sem_wait(&g_mutex_cola_new);
    t_PCB* pcb = queue_pop(g_cola_new);
    sem_post(&g_mutex_cola_new);

    sem_wait(&g_mutex_socket_memoria);
    enviar_proceso_a_memoria(pcb, g_socket_memoria, g_logger);
    sem_post(&g_mutex_socket_memoria);

    enviar_proceso_a_ready(pcb);  
}

void enviar_proceso_a_ready(t_PCB* pcb){
    sem_wait(&g_mutex_cola_ready);
    queue_push(g_cola_ready, pcb);
    sem_post(&g_mutex_cola_ready);
        
    pcb-> estado = READY;
    log_info(g_logger, "Cantidad de procesos en READY: %d", queue_size(g_cola_ready));
    log_info(g_logger, "Proceso %d encolado en READY", pcb->PID);
}


void ejecutar_cpu_FIFO(t_PCB* pcb, int conexion_cpu_dispatch, t_log* logger){
    int error;
    sem_wait(&g_disponible_exec);
    log_info(g_logger, "enviando PCB a CPU");
    error = enviar_pcb(g_conexion_cpu_dispatch, pcb);
    g_exec = pcb;

    // if (error == -1){
    //     log_error(g_logger, "Error al enviar PCB a CPU");
    // }else{
    //     log_info(g_logger, "PCB enviado a CPU");
    // }

    // Recibir PCB
    recibir_pcb_desalojado(pcb);
}

void esperar_quantum(uint32_t PID){
    sleep(g_quantum);
    if(g_exec->PID == PID){
    enviar_interrupcion(g_conexion_cpu_interrupt, PID);
    }
}

void ejecutar_cpu_RR(t_PCB* pcb){

    int error;
    sem_wait(&g_disponible_exec);
    error = enviar_pcb(g_conexion_cpu_dispatch, pcb);
    g_exec = pcb;

    pthread_t hilo_quantum;
    pthread_create(&hilo_quantum, NULL, (void*)esperar_quantum,pcb->PID);
    pthread_detach(hilo_quantum);

    recibir_pcb_desalojado(pcb);
    pthread_cancel(hilo_quantum);

}

void recibir_pcb_desalojado(t_PCB* pcb){

    t_PCB* pcb_recibido = recibir_pcb(g_conexion_cpu_dispatch);
    int motivo;
    if (pcb_recibido == NULL){
        log_error(g_logger, "Error al recibir PCB de CPU");
        free(pcb_recibido);
    }else{
        log_info(g_logger, "PCB recibido de CPU");
        actualizar_pcb(pcb, pcb_recibido);
        free(pcb_recibido);
    }

    recv(g_conexion_cpu_dispatch, &motivo, sizeof(int), MSG_WAITALL);
    log_info(g_logger, "Motivo de finalizacion: %d", motivo);

    t_desalojo* desalojo = malloc(sizeof(t_desalojo));
    desalojo->pcb = pcb;
    desalojo->motivo = motivo;

    pthread_t hilo_desalojo;
    pthread_create(&hilo_desalojo, NULL, (void*)atender_desalojo, desalojo);
    pthread_detach(hilo_desalojo);
 
    sem_post(&g_disponible_exec);
}

void planificador_exit(){
    while(1){
        sem_wait(&g_hay_elementos_en_exit);
        sem_wait(&g_mutex_cola_exit);
        t_PCB* pcb = queue_pop(g_cola_exit);
        sem_post(&g_mutex_cola_exit);
        log_info(g_logger, "Proceso %d finalizado", pcb->PID);
        //Liberarlo de memoria (entrega 3)
        free(pcb);
    }
}

void finalizar_proceso(t_PCB* pcb){
    pcb->estado = EXIT;
    log_info(g_logger, "Proceso %d finalizado", pcb->PID);
    sem_wait(&g_mutex_cola_exit);
    queue_push(g_cola_exit, pcb);
    sem_post(&g_mutex_cola_exit);
    sem_post(&g_hay_elementos_en_exit);
    sem_post(&g_tope_multiprogramacion);
}

void atender_desalojo(t_desalojo* desalojo){
    switch(desalojo->motivo){
        case FINALIZACION:
            finalizar_proceso(desalojo->pcb);
            break;
        case INTERRUPCION:
            enviar_proceso_a_ready(desalojo->pcb);
            break;
        case IO_GEN_SLEEP:
            t_buffer* buffer = recibir_buffer(g_conexion_cpu_dispatch);
            //Parametro
            int unidadesDeTrabajo = buffer_read_int(buffer);
            uint32_t* length = malloc(sizeof(uint32_t));
            //Nombre
            char* nombreInterfaz = buffer_read_string(buffer, length);
            char* instruccion = string_new();
            //instruccion que deba entender
            instruccion = "IO_GEN_SLEEP";

            sem_wait(&g_mutex_acceso_interfaces);

            if(dictionary_has_key(g_interfaces, nombreInterfaz)){

                t_interfaz_conectada* interfaz = dictionary_get(g_interfaces, nombreInterfaz);

                sem_post(&g_mutex_acceso_interfaces);

                // TODO: checkear que pueda hacer la operacion

                t_parametro_cola_interfaz* item = malloc(sizeof(t_parametro_cola_interfaz));

                item->pcb = desalojo->pcb;

                t_instruccion_io* instruccion_io = crear_instruccion_io(instruccion, unidadesDeTrabajo, NULL, NULL, NULL, NULL);

                item->instruccion = malloc(sizeof(t_instruccion_io));
                item->instruccion = instruccion_io;
                queue_push(interfaz->cola, item);
                sem_post(&interfaz->semaforo);
                log_info(g_logger, "Proceso %d bloqueado", desalojo->pcb->PID);
            }else{
                sem_post(&g_mutex_acceso_interfaces);

                finalizar_proceso(desalojo->pcb);
            }
           
            break;
    }
    free(desalojo);
}

void planificador_fifo(){
    while(1){
        sem_wait(&g_hay_elementos_en_ready);
        log_info(g_logger, "Planificador FIFO");
        sem_wait(&g_mutex_cola_ready);
        t_PCB* pcb = queue_pop(g_cola_ready);
        sem_post(&g_mutex_cola_ready);
        ejecutar_cpu_FIFO(pcb, g_conexion_cpu_dispatch, g_logger);
    }
}

void planificador_RR(){
    while(1){
    sem_wait(&g_hay_elementos_en_ready);
    log_info(g_logger, "Planificador RR");
    sem_wait(&g_mutex_cola_ready);
    t_PCB* pcb = queue_pop(g_cola_ready);
    sem_post(&g_mutex_cola_ready);
    ejecutar_cpu_RR(pcb);
    }
}

void enviar_interrupcion(int socket_interrupt, uint32_t* PID){

    t_buffer* buffer = buffer_create(sizeof(uint32_t));
    buffer_add_uint32(buffer, *PID);
    
    t_paquete* paquete = crear_paquete(ENVIO_INTERRUPCION, buffer); 
    enviar_paquete(paquete, socket_interrupt);
}
