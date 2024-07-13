#include "kernel.h"
#include "cola_exit.h"
#include "recursos.h"
#include "utils/buffer.h"
#include "utils/client.h"
#include "utils/codigo_error.h"
#include "utils/codigo_operacion.h"
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/log.h>
#include <commons/string.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <readline/readline.h>
#include <stdio.h>
#include <time.h>
#include "global_kernel.h"
#include "utils/instrucciones_io.h"
#include "utils/procesos.h"
#include "utils/server.h"
#include "utils/peticiones_memoria.h"
void manejar_recurso(int, char*, t_PCB*);
void liberar_cola_exec();
void crear_hilo_test();

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
    if(g_planificacion_pausada == 1){
        sem_wait(&g_notif_largo_plazo);
    } 

    t_PCB* pcb = crear_PCB();
    
    sem_wait(&mutex_contador_pid);
    g_contador_pid++;
    pcb->PID = g_contador_pid;
    sem_post(&mutex_contador_pid);
    log_info(g_logger,"Proceso %d creado", pcb->PID);
    
    pcb->estado = NEW;
    pcb->quantum = g_quantum;
    pcb->readyplus = 0;
    pcb->path_length = strlen(path) + 1;
    pcb->path = malloc(pcb->path_length);
    strcpy(pcb->path, path);
    
    sem_wait(&g_mutex_cola_new);
    queue_push(g_cola_new, pcb);
    sem_post(&g_mutex_cola_new);

    //lista general de procesos
    sem_wait(&g_mutex_lista_procesos_gral);
    list_add(g_lista_procesos_gral, pcb);
    sem_post(&g_mutex_lista_procesos_gral);

    //Esta linea bloquea la consola
    //sem_wait(&g_tope_multiprogramacion);
    pthread_t hilo_envios_a_ready;
    pthread_create(&hilo_envios_a_ready, NULL, (void*)preparar_proceso_a_ready, NULL);
    pthread_detach(hilo_envios_a_ready);
}

void preparar_proceso_a_ready(){
    sem_wait(&g_tope_multiprogramacion);

    sem_wait(&g_mutex_cola_new);
    t_PCB* pcb = queue_pop(g_cola_new);
    sem_post(&g_mutex_cola_new);

    sem_wait(&g_mutex_socket_memoria);
    enviar_proceso_a_memoria(pcb, g_socket_memoria, g_logger);
    sem_post(&g_mutex_socket_memoria);

    enviar_proceso_a_ready(pcb);  
}

void enviar_proceso_a_ready(t_PCB* pcb){
    // sem_wait(&g_notif_corto_plazo);
    // sem_post(&g_notif_corto_plazo);
    
    pcb->quantum = g_quantum;

    sem_wait(&g_mutex_cola_ready);
    queue_push(g_cola_ready, pcb);
    sem_post(&g_mutex_cola_ready);
    
    if(string_equals_ignore_case(algoritmo_planificacion, "VRR")){
        sem_post(&g_hay_elementos_para_ejecutar);
    } else {
        sem_post(&g_hay_elementos_en_ready);
    }

    pcb-> estado = READY;

}


void ejecutar_cpu_FIFO(t_PCB* pcb){
    int error;
    sem_wait(&g_disponible_exec);
    log_info(g_logger, "enviando PCB a CPU");
    error = enviar_pcb(g_conexion_cpu_dispatch, pcb);

    pcb->estado = EXEC;
    g_exec = pcb;

    // if (error == -1){
    //     log_error(g_logger, "Error al enviar PCB a CPU");
    // }else{
    //     log_info(g_logger, "PCB enviado a CPU");
    // }

    // Recibir PCB
    recibir_pcb_desalojado(pcb);
}

void esperar_quantum(t_PCB* pcb){
    log_info(g_logger, "Esperando quantum %d ms", pcb->quantum );
    usleep(pcb->quantum * 1000);
    if(g_exec != NULL && g_exec->PID == pcb->PID){
        enviar_interrupcion(g_conexion_cpu_interrupt, &pcb->PID, INTERRUPCION_QUANTUM);
    }
}

void ejecutar_cpu_RR(t_PCB* pcb){

    int error;
    log_info(g_logger, "Antes semaforo exec");
    sem_wait(&g_disponible_exec);
    log_info(g_logger, "Despues semaforo exec");
    error = enviar_pcb(g_conexion_cpu_dispatch, pcb);
    pcb->estado = EXEC;
    g_exec = pcb;

    pthread_t hilo_quantum;
    pthread_create(&hilo_quantum, NULL, (void*)esperar_quantum, pcb);
    
    recibir_pcb_desalojado(pcb);
    pthread_cancel(hilo_quantum);
    pthread_join(hilo_quantum, NULL);
}

void agregar_a_cola_auxiliar(t_PCB* pcb){
    sem_wait(&g_mutex_cola_auxiliar);
    queue_push(g_cola_auxiliar, pcb);
    sem_post(&g_mutex_cola_auxiliar);
    pcb->estado = READYPLUS;
}

void ejecutar_cpu_VRR(t_PCB* pcb){
    int error;
    sem_wait(&g_disponible_exec);
    error = enviar_pcb(g_conexion_cpu_dispatch, pcb);
    pcb->estado = EXEC;
    g_exec = pcb;
    
    pthread_t hilo_quantum;
    timer = temporal_create();
    pthread_create(&hilo_quantum, NULL, (void*)esperar_quantum, pcb);

    recibir_pcb_desalojado(pcb);
    temporal_stop(timer);
    g_ms_transcurridos = temporal_gettime(timer);
    sem_post(&g_tiempo_calculado);

    pthread_cancel(hilo_quantum);
    pthread_join(hilo_quantum, NULL);
    temporal_destroy(timer);
}

void recibir_pcb_desalojado(t_PCB* pcb_ejecutando){
    // sem_wait(&g_notif_corto_plazo);
    // sem_post(&g_notif_corto_plazo);

    t_PCB* pcb_recibido = recibir_pcb(g_conexion_cpu_dispatch);
    int motivo;
    if (pcb_recibido == NULL){
        log_error(g_logger, "Error al recibir DESALOJO");
        free(pcb_recibido);
    }else{
        actualizar_pcb(pcb_ejecutando, pcb_recibido);
        free(pcb_recibido);

        // Posible condicion de carrera
        g_exec = NULL;
    }

    recv(g_conexion_cpu_dispatch, &motivo, sizeof(int), MSG_WAITALL);
    log_info(g_logger, "PID: %d - Motivo de desalojo: %d", pcb_ejecutando->PID ,motivo);

    t_desalojo* desalojo = malloc(sizeof(t_desalojo));
    desalojo->pcb = pcb_ejecutando;
    desalojo->motivo = motivo;
    
    pthread_t hilo_desalojo;
    log_info(g_logger, "Creando hilo desalojo");

    if(desalojo->motivo == SIGNAL){
        sem_wait(&g_mutex_cola_signal);
    }
    int result = pthread_create(&hilo_desalojo, NULL, (void*)atender_desalojo, desalojo);
    pthread_detach(hilo_desalojo);
}

void finalizar_proceso(t_PCB* pcb){

    pcb->estado = EXIT;
    log_info(g_logger, "Proceso %d finalizado", pcb->PID);

    agregar_a_cola_exit(pcb);

    log_info(g_logger, "Finaliza el proceso %d - Motivo: %s", pcb->PID, "SUCCESS");
    //sem_post(&g_tope_multiprogramacion);
    if(g_post_a_saltear_multiprogramacion > 0){
        g_post_a_saltear_multiprogramacion--;
        //g_grado_multiprogramacion--;
    }else{
        sem_post(&g_tope_multiprogramacion);
    }
}

void atender_desalojo(t_desalojo* desalojo){
    if (g_planificacion_pausada == 1){
        sem_wait(&g_notif_largo_plazo);
    }
    
    log_info(g_logger, "Atendiendo desalojo de proceso %d", desalojo->pcb->PID);
    desalojo->pcb->estado = BLOCKED;
    switch(desalojo->motivo){
        case FINALIZACION:
        {
            liberar_cola_exec();
            finalizar_proceso(desalojo->pcb);
            return;
        }
        case ERROR_OUT_OF_MEMORY:
        {
            liberar_cola_exec();
            finalizar_proceso(desalojo->pcb);
            return;
        }
        case INTERRUPCION_QUANTUM: //CLOCK
        {
            liberar_cola_exec();
            enviar_proceso_a_ready(desalojo->pcb);
            return;
        }
        case INTERRUPCION_KILL:
        {
            liberar_cola_exec();
            finalizar_proceso(desalojo->pcb);
            return;
        }
        case IO_GEN_SLEEP:
        {   //Abstraer a un case IO
            if(string_equals_ignore_case(algoritmo_planificacion, "VRR")){
                if(g_ms_transcurridos < desalojo->pcb->quantum){
                    sem_wait(&g_tiempo_calculado);
                    desalojo->pcb->quantum -= g_ms_transcurridos;
                    desalojo->pcb->readyplus = 1;       
                }
            }
            t_buffer* buffer = recibir_buffer(g_conexion_cpu_dispatch);
            //Parametro
            int unidadesDeTrabajo = buffer_read_int(buffer);
            uint32_t* length = malloc(sizeof(uint32_t));
            //Nombre
            char* nombreInterfaz = buffer_read_string(buffer, length);
            char instruccion[] = "IO_GEN_SLEEP";
            liberar_cola_exec();
            //instruccion que deba entender

            sem_wait(&g_mutex_acceso_interfaces);

            if(dictionary_has_key(g_interfaces, nombreInterfaz)){

                t_interfaz_conectada* interfaz = dictionary_get(g_interfaces, nombreInterfaz);

                sem_post(&g_mutex_acceso_interfaces);

                // TODO: checkear que pueda hacer la operacion

                t_parametro_cola_interfaz* item = malloc(sizeof(t_parametro_cola_interfaz));

                item->pcb = desalojo->pcb;

                t_instruccion_io* instruccion_io = crear_instruccion_io(item->pcb->PID,instruccion, unidadesDeTrabajo, NULL, NULL, NULL, NULL, NULL);

                item->instruccion = malloc(sizeof(t_instruccion_io));
                item->instruccion = instruccion_io;
                sem_wait(&interfaz->mutex);
                queue_push(interfaz->cola, item);
                sem_post(&interfaz->mutex);
                /*
                sem_wait(&g_lista_blocked_gral);
                list_add(g_lista_blocked_gral, item->pcb);
                sem_post(&g_lista_blocked_gral);
                */

                sem_post(&interfaz->semaforo);
                log_info(g_logger, "Proceso %d bloqueado", desalojo->pcb->PID);
                
                //agrega a la cola de bloquedos y ponerle estado bloqueado al pcb

            }else{
                sem_post(&g_mutex_acceso_interfaces);

                finalizar_proceso(desalojo->pcb);
            }

            buffer_destroy(buffer);
            free(length);
            free(nombreInterfaz);

            break;
        }
        case IO_STDIN_READ:
        {   
            //TODO Abstraer a un case IO 
            if(string_equals_ignore_case(algoritmo_planificacion, "VRR")){
                if(g_ms_transcurridos < desalojo->pcb->quantum){
                sem_wait(&g_tiempo_calculado);
                desalojo->pcb->quantum -= g_ms_transcurridos;
                desalojo->pcb->readyplus = 1;       
                }
            }
            t_buffer* buffer = recibir_buffer(g_conexion_cpu_dispatch);
            //Parametros
            int tamanio = buffer_read_int(buffer);
            int sizeLista = buffer_read_int(buffer);
            t_list* lista = list_create();
            buffer_read_lista(buffer, sizeLista, lista);

            uint32_t* length = malloc(sizeof(uint32_t));
            char* nombreInterfaz = buffer_read_string(buffer, length);
            liberar_cola_exec();
            char* instruccion = string_new();
            //instruccion que deba entender
            instruccion = "IO_STDIN_READ";

            //TODO abstraer a una funcion, es todo igual menos el paquete t_instruccion_io

            sem_wait(&g_mutex_acceso_interfaces);

            if(dictionary_has_key(g_interfaces, nombreInterfaz)){

                t_interfaz_conectada* interfaz = dictionary_get(g_interfaces, nombreInterfaz);

                sem_post(&g_mutex_acceso_interfaces);

                // TODO: checkear que pueda hacer la operacion

                t_parametro_cola_interfaz* item = malloc(sizeof(t_parametro_cola_interfaz));

                item->pcb = desalojo->pcb;
                
                t_instruccion_io* instruccion_io = crear_instruccion_io(item->pcb->PID,instruccion, 0, 0, tamanio, NULL, NULL,lista);

                item->instruccion = malloc(sizeof(t_instruccion_io));
                item->instruccion = instruccion_io;
                sem_wait(&interfaz->mutex);
                queue_push(interfaz->cola, item);
                sem_post(&interfaz->mutex);

                list_add(g_lista_blocked_gral, item->pcb);

                sem_post(&interfaz->semaforo);
                log_info(g_logger, "Proceso %d bloqueado", desalojo->pcb->PID);
                //agrega a la cola de bloquedos y ponerle estado bloqueado al pcb

            }else{
                sem_post(&g_mutex_acceso_interfaces);
                finalizar_proceso(desalojo->pcb);
            }

            buffer_destroy(buffer);
            free(length);
            free(nombreInterfaz);

            break;
        }
        case IO_STDOUT_WRITE:
        {   //TODO Abstraer a un case IO 
            if(string_equals_ignore_case(algoritmo_planificacion, "VRR")){
                if(g_ms_transcurridos < desalojo->pcb->quantum){
                sem_wait(&g_tiempo_calculado);
                desalojo->pcb->quantum -= g_ms_transcurridos;
                desalojo->pcb->readyplus = 1;       
                }
            }
            t_buffer* buffer = recibir_buffer(g_conexion_cpu_dispatch);
            //Parametros
            int tamanio = buffer_read_int(buffer);
            int sizeLista = buffer_read_int(buffer);
            t_list* lista = list_create();
            buffer_read_lista(buffer, sizeLista, lista);
            uint32_t* length = malloc(sizeof(uint32_t));

            char* nombreInterfaz = buffer_read_string(buffer, length);
            liberar_cola_exec();
            char instruccion[] = "IO_STDOUT_WRITE";
            //instruccion que deba entender

            //TODO abstraer a una funcion, es todo igual menos el paquete t_instruccion_io

            sem_wait(&g_mutex_acceso_interfaces);

            if(dictionary_has_key(g_interfaces, nombreInterfaz)){

                t_interfaz_conectada* interfaz = dictionary_get(g_interfaces, nombreInterfaz);

                sem_post(&g_mutex_acceso_interfaces);

                // TODO: checkear que pueda hacer la operacion

                t_parametro_cola_interfaz* item = malloc(sizeof(t_parametro_cola_interfaz));

                item->pcb = desalojo->pcb;

                t_instruccion_io* instruccion_io = crear_instruccion_io(item->pcb->PID,instruccion, 0, 0, tamanio, NULL, NULL,lista);

                item->instruccion = malloc(sizeof(t_instruccion_io));
                item->instruccion = instruccion_io;
                sem_wait(&interfaz->mutex);
                queue_push(interfaz->cola, item);
                sem_post(&interfaz->mutex);

                sem_post(&interfaz->semaforo);
                log_info(g_logger, "Proceso %d bloqueado", desalojo->pcb->PID);
                //agrega a la cola de bloquedos y ponerle estado bloqueado al pcb

            }else{
                sem_post(&g_mutex_acceso_interfaces);
                finalizar_proceso(desalojo->pcb);
            }
            break;
        }
        case SIGNAL:
        {

            t_buffer* buffer = recibir_buffer(g_conexion_cpu_dispatch);
            uint32_t *length = malloc(sizeof(uint32_t));
            char *recurso_leido = buffer_read_string(buffer, length);
            desalojo->recurso = string_duplicate(recurso_leido);
            log_info(g_logger, "SIGNAL en atender desalojo del Recurso: %s", desalojo->recurso);

            liberar_cola_exec();
            char * recurso_signal = string_duplicate(desalojo->recurso);
            
            procesar_signal(recurso_signal);

            desalojo->pcb->estado = READY;
            queue_push(g_cola_signal, desalojo->pcb);
            sem_post(&g_mutex_cola_signal);


            buffer_destroy(buffer);
            free(recurso_leido);
            break;
        }
        case WAIT:
        {

            t_buffer* buffer = recibir_buffer(g_conexion_cpu_dispatch);
            uint32_t *length = malloc(sizeof(uint32_t));
            char *recurso_leido = buffer_read_string(buffer, length);
            desalojo->recurso = recurso_leido;
            log_info(g_logger, "WAIT en atender desalojo del Recurso: %s", desalojo->recurso);
            

            liberar_cola_exec();
            char * recurso_wait = string_duplicate(desalojo->recurso);
            
            bool continuar = procesar_wait(desalojo->pcb, recurso_wait);

            if(continuar){
                enviar_proceso_a_ready(desalojo->pcb);
            }
            
            buffer_destroy(buffer);
            free(recurso_leido);
            break;
        }
        case IO_FS_CREATE:
        {
            if(string_equals_ignore_case(algoritmo_planificacion, "VRR")){
                if(g_ms_transcurridos < desalojo->pcb->quantum){
                sem_wait(&g_tiempo_calculado);
                desalojo->pcb->quantum -= g_ms_transcurridos;
                desalojo->pcb->readyplus = 1;       
                }
            }

            t_buffer* buffer = recibir_buffer(g_conexion_cpu_dispatch);
            uint32_t *length = malloc(sizeof(uint32_t));
            char* interfaz = buffer_read_string(buffer, length);
            char* nombre_archivo = buffer_read_string(buffer, length);
            liberar_cola_exec();
            procesar_io_fs_create(interfaz, nombre_archivo, desalojo->pcb);
            free(interfaz);
            free(nombre_archivo);
            break;
        }
        case IO_FS_DELETE:
        {
            if(string_equals_ignore_case(algoritmo_planificacion, "VRR")){
                if(g_ms_transcurridos < desalojo->pcb->quantum){
                sem_wait(&g_tiempo_calculado);
                desalojo->pcb->quantum -= g_ms_transcurridos;
                desalojo->pcb->readyplus = 1;       
                }
            }
            t_buffer* buffer = recibir_buffer(g_conexion_cpu_dispatch);
            uint32_t *length = malloc(sizeof(uint32_t));
            char* interfaz = buffer_read_string(buffer, length);
            char* nombre_archivo = buffer_read_string(buffer, length);
            liberar_cola_exec();
            procesar_io_fs_delete(interfaz, nombre_archivo, desalojo->pcb);
            free(interfaz);
            free(nombre_archivo);
            break;
        }
        case IO_FS_TRUNCATE:
        {
            if(string_equals_ignore_case(algoritmo_planificacion, "VRR")){
                if(g_ms_transcurridos < desalojo->pcb->quantum){
                sem_wait(&g_tiempo_calculado);
                desalojo->pcb->quantum -= g_ms_transcurridos;
                desalojo->pcb->readyplus = 1;       
                }
            }
            t_buffer* buffer = recibir_buffer(g_conexion_cpu_dispatch);
            uint32_t *length = malloc(sizeof(uint32_t));
            char* interfaz = buffer_read_string(buffer, length);
            char* nombre_archivo = buffer_read_string(buffer, length);
            int tamanio = buffer_read_int(buffer);
            liberar_cola_exec();
            procesar_io_fs_truncate(interfaz, nombre_archivo, tamanio, desalojo->pcb);
            free(interfaz);
            free(nombre_archivo);
            break;
        }
        case IO_FS_WRITE:
        {
            if(string_equals_ignore_case(algoritmo_planificacion, "VRR")){
                if(g_ms_transcurridos < desalojo->pcb->quantum){
                sem_wait(&g_tiempo_calculado);
                desalojo->pcb->quantum -= g_ms_transcurridos;
                desalojo->pcb->readyplus = 1;       
                }
            }
            t_buffer* buffer = recibir_buffer(g_conexion_cpu_dispatch);
            uint32_t *length = malloc(sizeof(uint32_t));
            char* interfaz = buffer_read_string(buffer, length);
            char* nombre_archivo = buffer_read_string(buffer, length);
            int direccion = buffer_read_int(buffer);
            int tamanio = buffer_read_int(buffer);
            int puntero_archivo = buffer_read_int(buffer);
            t_list* peticiones = list_create();
            int listaSize = buffer_read_int(buffer);
            buffer_read_lista(buffer, listaSize, peticiones);
            liberar_cola_exec();
            procesar_io_fs_write(interfaz, nombre_archivo, direccion, tamanio, puntero_archivo, desalojo->pcb, peticiones);
            free(interfaz);
            free(nombre_archivo);
            break;
        }
        case IO_FS_READ:
        {
            if(string_equals_ignore_case(algoritmo_planificacion, "VRR")){
                if(g_ms_transcurridos < desalojo->pcb->quantum){
                sem_wait(&g_tiempo_calculado);
                desalojo->pcb->quantum -= g_ms_transcurridos;
                desalojo->pcb->readyplus = 1;       
                }
            }
            t_buffer* buffer = recibir_buffer(g_conexion_cpu_dispatch);
            uint32_t *length = malloc(sizeof(uint32_t));
            char* interfaz = buffer_read_string(buffer, length);
            char* nombre_archivo = buffer_read_string(buffer, length);
            int direccion = buffer_read_int(buffer);
            int tamanio = buffer_read_int(buffer);
            int puntero_archivo = buffer_read_int(buffer);
            t_list* peticiones = list_create();
            int listaSize = buffer_read_int(buffer);
            buffer_read_lista(buffer, listaSize, peticiones);
            liberar_cola_exec();
            procesar_io_fs_read(interfaz, nombre_archivo, direccion, tamanio, puntero_archivo, desalojo->pcb, peticiones);
            free(interfaz);
            free(nombre_archivo);
            break;
        }
        default:
        {
            log_error(g_logger, "Motivo de desalojo no reconocido");
            liberar_cola_exec();
            finalizar_proceso(desalojo->pcb);
            break;
        }
    }
}

void liberar_cola_exec(){
    sem_post(&g_disponible_exec);
    log_info(g_logger, "DISPONIBLE EXEC");
}

void planificador_fifo(){
    
    while(1){

        if(g_planificacion_pausada == 1){
        sem_wait(&g_notif_corto_plazo);
    }

        sem_wait(&g_mutex_cola_signal);

        if(!queue_is_empty(g_cola_signal)){
            t_PCB* pcb = queue_pop(g_cola_signal);
            sem_post(&g_mutex_cola_signal);
            ejecutar_cpu_FIFO(pcb);
            continue;
        }

        sem_post(&g_mutex_cola_signal);

        sem_wait(&g_hay_elementos_en_ready);
        log_info(g_logger, "Planificador FIFO");
        sem_wait(&g_mutex_cola_ready);
        t_PCB* pcb = queue_pop(g_cola_ready);
        sem_post(&g_mutex_cola_ready);
        ejecutar_cpu_FIFO(pcb);
    }
}

void planificador_RR(){

    while(1){
        
        if(g_planificacion_pausada == 1){
        sem_wait(&g_notif_corto_plazo);
        }

        sem_wait(&g_mutex_cola_signal);
        
        if(!queue_is_empty(g_cola_signal)){
            t_PCB* pcb = queue_pop(g_cola_signal);
            sem_post(&g_mutex_cola_signal);
            ejecutar_cpu_RR(pcb);
            continue;
        }

        sem_post(&g_mutex_cola_signal);

    sem_wait(&g_hay_elementos_en_ready);
    sem_wait(&g_mutex_cola_ready);
    log_info(g_logger, "RR - Hay procesos en ready - %d", queue_size(g_cola_ready));
    t_PCB* pcb = queue_pop(g_cola_ready);
    sem_post(&g_mutex_cola_ready);
    ejecutar_cpu_RR(pcb);
    }
}

void planificador_VRR(){

    while(1){

        if(g_planificacion_pausada == 1){
        sem_wait(&g_notif_corto_plazo);
        }

        sem_wait(&g_mutex_cola_signal);
        
        if(!queue_is_empty(g_cola_signal)){
            t_PCB* pcb = queue_pop(g_cola_signal);
            sem_post(&g_mutex_cola_signal);
            ejecutar_cpu_VRR(pcb);
            continue;
        }

        sem_post(&g_mutex_cola_signal);

        sem_wait(&g_hay_elementos_para_ejecutar);
        log_info(g_logger, "Planificador VRR");
        
        t_PCB* pcb; 
        if(!queue_is_empty(g_cola_auxiliar)){
            sem_wait(&g_mutex_cola_auxiliar);
            pcb = queue_pop(g_cola_auxiliar);
            sem_post(&g_mutex_cola_auxiliar);
        } else{
            sem_wait(&g_mutex_cola_ready);
            pcb = queue_pop(g_cola_ready);
            sem_post(&g_mutex_cola_ready);
        }
        ejecutar_cpu_VRR(pcb);
    }
}

void enviar_interrupcion(int socket_interrupt, uint32_t* PID, uint32_t motivo){
    log_info(g_logger, "Enviando interrupcion ");
    t_buffer* buffer = buffer_create(sizeof(uint32_t) + sizeof(uint32_t));
    buffer_add_uint32(buffer, *PID);
    buffer_add_uint32(buffer, motivo);
    
    t_paquete* paquete = crear_paquete(ENVIO_INTERRUPCION, buffer); 
    enviar_paquete(paquete, socket_interrupt);
}



void eliminar_de_lista_blocked_gral(uint32_t pid){
      
    sem_wait(&g_lista_blocked_gral);
    if (!list_remove_element(g_lista_blocked_gral, &pid));
        printf("No se pudo eliminar el PID %d de la lista de bloqueados generales\n", pid);
    sem_post(&g_lista_blocked_gral);
}

void listar_procesos(){

    // estados posibles NEW, READY, READY-PLUS, EXEC, BLOCKED, EXIT

    // listo Procesos en NEW
    sem_wait(&g_mutex_cola_new);
    t_list* procesos_new = g_cola_new->elements;

    if(list_is_empty(procesos_new)){
        log_info(g_logger, "No hay procesos en NEW");
    }else{
        log_info(g_logger, "Procesos en NEW");
        t_list_iterator* new_iterator = list_iterator_create(procesos_new);
        while(list_iterator_has_next(new_iterator)){
            t_PCB* pcb = list_iterator_next(new_iterator);
            log_info(g_logger, "PID: %d - Estado: NEW", pcb->PID);
        }

        list_iterator_destroy(new_iterator);
    }
    sem_post(&g_mutex_cola_new);

    // listo Procesos en READY
    sem_wait(&g_mutex_cola_ready);
    t_list* procesos_ready = g_cola_ready->elements;

    if(list_is_empty(procesos_ready)){
        log_info(g_logger, "No hay procesos en READY");

    }else{
        log_info(g_logger, "Procesos en READY");
        t_list_iterator* ready_iterator = list_iterator_create(procesos_ready);
        while(list_iterator_has_next(ready_iterator)){
            t_PCB* pcb = list_iterator_next(ready_iterator);
            log_info(g_logger, "PID: %d - Estado: READY", pcb->PID);
        }

        list_iterator_destroy(ready_iterator);
    }
    sem_post(&g_mutex_cola_ready);

    // listo Procesos en READY-PLUS

    sem_wait(&g_mutex_cola_auxiliar);
    t_list* procesos_auxiliar = g_cola_auxiliar->elements;

    if(list_is_empty(procesos_auxiliar)){
        log_info(g_logger, "No hay procesos en READY-PLUS");
    }else{
        log_info(g_logger, "Procesos en READY-PLUS");
        t_list_iterator* ready_plus_iterator = list_iterator_create(procesos_auxiliar);
        while(list_iterator_has_next(ready_plus_iterator)){
            t_PCB* pcb = list_iterator_next(ready_plus_iterator);
            log_info(g_logger, "PID: %d - Estado: READY-PLUS", pcb->PID);
        }
    }
    sem_post(&g_mutex_cola_auxiliar);

    // listo Procesos en EXEC
    if(g_exec != NULL){
        log_info(g_logger, "Proceso en EXEC");
        log_info(g_logger, "PID: %d - Estado: EXEC", g_exec->PID);
    }else{
        log_info(g_logger, "No hay procesos en EXEC");
    }

    // listo Procesos en BLOCKED

    // bloqueados por Recursos

    t_list* procesos_bloqueados_recursos = get_procesos_bloqueados_recursos();
    if(!list_is_empty(procesos_bloqueados_recursos)){
        log_info(g_logger, "Procesos bloqueados por Recursos");
        t_list_iterator* blocked_iterator = list_iterator_create(procesos_bloqueados_recursos);
        while(list_iterator_has_next(blocked_iterator)){
            t_PCB* pcb = list_iterator_next(blocked_iterator);
            log_info(g_logger, "PID: %d - Estado: BLOCKED", pcb->PID);
        }
        list_iterator_destroy(blocked_iterator);
    }

    // bloqueados por IO

    sem_wait(&g_mutex_acceso_interfaces);

    if(!dictionary_is_empty(g_interfaces)){
        t_list* lista_interfaces = dictionary_elements(g_interfaces);
        t_list_iterator* iterator = list_iterator_create(lista_interfaces);
        while(list_iterator_has_next(iterator)){
            t_interfaz_conectada* interfaz = list_iterator_next(iterator);
            sem_wait(&interfaz->mutex);
            if(!queue_is_empty(interfaz->cola)){
                log_info(g_logger, "Procesos bloqueados en interfaz %s", interfaz->interfaz->nombre);
                t_list_iterator* blocked_iterator = list_iterator_create(interfaz->cola->elements);
                while(list_iterator_has_next(blocked_iterator)){
                    t_parametro_cola_interfaz* parametro = list_iterator_next(blocked_iterator);
                    log_info(g_logger, "PID: %d - Estado: BLOCKED", parametro->pcb->PID);
                }
                list_iterator_destroy(blocked_iterator);
            }
            sem_post(&interfaz->mutex);
        }
        list_iterator_destroy(iterator);
    }

    sem_post(&g_mutex_acceso_interfaces);


    // listo Procesos en EXIT

    t_list* procesos_exit = pids_exit(); // lista de ints

    if(list_is_empty(procesos_exit)){
        log_info(g_logger, "No hay procesos en EXIT");
    }else{
        log_info(g_logger, "Procesos en EXIT");
        t_list_iterator* exit_iterator = list_iterator_create(procesos_exit);
        while(list_iterator_has_next(exit_iterator)){
            uint32_t pid = (uint32_t)list_iterator_next(exit_iterator);
            log_info(g_logger, "PID: %d - Estado: EXIT", pid);
        }
        list_iterator_destroy(exit_iterator);
    }
    list_destroy(procesos_exit);

    }
     
t_PCB* buscar_y_eliminar_pid_en_cola(uint32_t pid,t_queue* cola) {

    if (queue_is_empty(cola)) {
        return NULL;
    }
    t_list* procesos = cola->elements;

    bool existePID(void* pcb) {
        return ((t_PCB*)pcb)->PID == pid;
    }

    t_PCB* pcb = (t_PCB*)list_find(procesos, &existePID);
    if(pcb != NULL){
        list_remove_by_condition(procesos, &existePID);
    }

    return pcb;
}

t_PCB* buscar_y_eliminar_pid_en_cola_io(uint32_t pid,t_queue* cola) {

    if (queue_is_empty(cola)) {
        return NULL;
    }
    t_list* procesos = cola->elements;

    bool existePID(t_parametro_cola_interfaz* parametro) {
        return parametro->pcb->PID == pid;
    }
    
    t_parametro_cola_interfaz* parametro = (t_PCB*)list_find(procesos, &existePID);
    if(parametro != NULL){
        list_remove_by_condition(procesos, &existePID);
    }

    return parametro->pcb;
}

t_PCB* buscar_en_diccionario_interfaces(uint32_t pid, t_dictionary* interfaces){
    t_PCB* pcb = NULL;

    if(dictionary_is_empty(interfaces)){
        return NULL;
    }

    t_list* lista_interfaces = dictionary_elements(interfaces);
    t_list_iterator* iterator = list_iterator_create(lista_interfaces);
     while(list_iterator_has_next(iterator)){
        t_interfaz_conectada* interfaz = list_iterator_next(iterator);
        //TODO agregar semaforo para acceso
        sem_wait(&interfaz->mutex);
        pcb = buscar_y_eliminar_pid_en_cola(pid, interfaz->cola);
        sem_post(&interfaz->mutex);
        if(pcb != NULL){
            return pcb;
        }
    }
    

    return pcb;
}

t_PCB* buscar_en_diccionario_recursos(uint32_t pid, t_dictionary* colas_recursos){
    t_PCB* pcb = NULL;

    t_list* lista_recursos = dictionary_elements(colas_recursos);
    t_list_iterator* iterator = list_iterator_create(lista_recursos);
     while(list_iterator_has_next(iterator)){
        t_recurso* recurso = list_iterator_next(iterator);
        //TODO agregar semaforo para acceso
        pcb = buscar_y_eliminar_pid_en_cola(pid, recurso->cola);
        if(pcb != NULL){
            return pcb;
        }
    }

    return pcb;
}

t_PCB* buscar_pid_en_sistema(uint32_t pid){
    t_PCB* pcb = NULL;

    sem_wait(&g_mutex_cola_new);
    pcb = buscar_y_eliminar_pid_en_cola(pid, g_cola_new);
    sem_post(&g_mutex_cola_new);

    if(pcb == NULL){
        sem_wait(&g_mutex_cola_ready);
        pcb = buscar_y_eliminar_pid_en_cola(pid, g_cola_ready);
        sem_post(&g_mutex_cola_ready);
    }


    if(pcb == NULL && string_equals_ignore_case(algoritmo_planificacion, "VRR")){
        sem_wait(&g_mutex_cola_auxiliar);
        pcb = buscar_y_eliminar_pid_en_cola(pid, g_cola_auxiliar);
        sem_post(&g_mutex_cola_auxiliar);
    }


    if(pcb == NULL && g_exec != NULL && g_exec->PID == pid){
        pcb = g_exec;
        
        if(pcb != NULL){
        log_info(g_logger, "PID ENCONTRANDO EN COLA EXEC");
        log_info(g_logger, "PID: %d ESTADO: %d", pcb->PID, pcb->estado );
        pcb->estado = EXEC;
        }
    }
    
    if(pcb == NULL){
        pcb = buscar_en_diccionario_interfaces(pid, g_interfaces);
    }

    if(pcb == NULL){
        pcb = quitar_proceso_bloqueado(pid);
    }

    return pcb;
}

void procesar_io_fs_create(char* interfaz, char* nombre_archivo, t_PCB* pcb) {
    sem_wait(&g_mutex_acceso_interfaces);

    if (dictionary_has_key(g_interfaces, interfaz)) {
        t_interfaz_conectada* interfaz_conectada = dictionary_get(g_interfaces, interfaz);
        sem_post(&g_mutex_acceso_interfaces);

        t_parametro_cola_interfaz* item = malloc(sizeof(t_parametro_cola_interfaz));
        item->pcb = pcb;
        t_instruccion_io* instruccion_io = crear_instruccion_io(item->pcb->PID,"IO_FS_CREATE", 0, 0, 0, nombre_archivo, 0,NULL);
        
        item->instruccion = malloc(sizeof(t_instruccion_io));
        item->instruccion = instruccion_io;

        sem_wait(&interfaz_conectada->mutex);
        queue_push(interfaz_conectada->cola, item);
        sem_post(&interfaz_conectada->mutex);

        sem_post(&interfaz_conectada->semaforo);
        log_info(g_logger, "Proceso %d bloqueado en IO_FS_CREATE", pcb->PID);
    } else {
        sem_post(&g_mutex_acceso_interfaces);
        finalizar_proceso(pcb);
    }
}

void procesar_io_fs_delete(char* interfaz, char* nombre_archivo, t_PCB* pcb) {
    sem_wait(&g_mutex_acceso_interfaces);

    if (dictionary_has_key(g_interfaces, interfaz)) {
        t_interfaz_conectada* interfaz_conectada = dictionary_get(g_interfaces, interfaz);
        sem_post(&g_mutex_acceso_interfaces);

        t_parametro_cola_interfaz* item = malloc(sizeof(t_parametro_cola_interfaz));
        item->pcb = pcb;
        t_instruccion_io* instruccion_io = crear_instruccion_io(item->pcb->PID,"IO_FS_DELETE", 0, 0, 0, nombre_archivo, 0,NULL);
        item->instruccion = instruccion_io;

        sem_wait(&interfaz_conectada->mutex);
        queue_push(interfaz_conectada->cola, item);
        sem_post(&interfaz_conectada->mutex);

        sem_post(&interfaz_conectada->semaforo);
        log_info(g_logger, "Proceso %d bloqueado en IO_FS_DELETE", pcb->PID);
    } else {
        sem_post(&g_mutex_acceso_interfaces);
        finalizar_proceso(pcb);
    }
}

void procesar_io_fs_truncate(char* interfaz, char* nombre_archivo, int tamanio, t_PCB* pcb) {
    sem_wait(&g_mutex_acceso_interfaces);

    if (dictionary_has_key(g_interfaces, interfaz)) {
        t_interfaz_conectada* interfaz_conectada = dictionary_get(g_interfaces, interfaz);
        sem_post(&g_mutex_acceso_interfaces);

        t_parametro_cola_interfaz* item = malloc(sizeof(t_parametro_cola_interfaz));
        item->pcb = pcb;
        t_instruccion_io* instruccion_io = crear_instruccion_io(item->pcb->PID,"IO_FS_TRUNCATE", 0, 0 , tamanio, nombre_archivo, 0,NULL);
        item->instruccion = instruccion_io;

        sem_wait(&interfaz_conectada->mutex);
        queue_push(interfaz_conectada->cola, item);
        sem_post(&interfaz_conectada->mutex);

        sem_post(&interfaz_conectada->semaforo);
        log_info(g_logger, "Proceso %d bloqueado en IO_FS_TRUNCATE", pcb->PID);
    } else {
        sem_post(&g_mutex_acceso_interfaces);
        finalizar_proceso(pcb);
    }
}

void procesar_io_fs_write(char* interfaz, char* nombre_archivo, int direccion, int tamanio, int puntero_archivo, t_PCB* pcb, t_list* peticiones) {
    sem_wait(&g_mutex_acceso_interfaces);

    if (dictionary_has_key(g_interfaces, interfaz)) {
        t_interfaz_conectada* interfaz_conectada = dictionary_get(g_interfaces, interfaz);
        sem_post(&g_mutex_acceso_interfaces);

        t_parametro_cola_interfaz* item = malloc(sizeof(t_parametro_cola_interfaz));
        item->pcb = pcb;
        t_instruccion_io* instruccion_io = crear_instruccion_io(item->pcb->PID,"IO_FS_WRITE", 0,direccion, tamanio, nombre_archivo, puntero_archivo, peticiones);
        item->instruccion = instruccion_io;

        sem_wait(&interfaz_conectada->mutex);
        queue_push(interfaz_conectada->cola, item);
        sem_post(&interfaz_conectada->mutex);

        sem_post(&interfaz_conectada->semaforo);
        log_info(g_logger, "Proceso %d bloqueado en IO_FS_WRITE", pcb->PID);
    } else {
        sem_post(&g_mutex_acceso_interfaces);
        finalizar_proceso(pcb);
    }
}

void procesar_io_fs_read(char* interfaz, char* nombre_archivo, int direccion, int tamanio, int puntero_archivo, t_PCB* pcb, t_list* peticiones) {
    sem_wait(&g_mutex_acceso_interfaces);

    if (dictionary_has_key(g_interfaces, interfaz)) {
        t_interfaz_conectada* interfaz_conectada = dictionary_get(g_interfaces, interfaz);
        sem_post(&g_mutex_acceso_interfaces);

        t_parametro_cola_interfaz* item = malloc(sizeof(t_parametro_cola_interfaz));
        item->pcb = pcb;
        t_instruccion_io* instruccion_io = crear_instruccion_io(item->pcb->PID,"IO_FS_READ", 0,direccion, tamanio, nombre_archivo, puntero_archivo,peticiones);
        item->instruccion = instruccion_io;

        sem_wait(&interfaz_conectada->mutex);
        queue_push(interfaz_conectada->cola, item);
        sem_post(&interfaz_conectada->mutex);

        sem_post(&interfaz_conectada->semaforo);
        log_info(g_logger, "Proceso %d bloqueado en IO_FS_READ", pcb->PID);
    } else {
        sem_post(&g_mutex_acceso_interfaces);
        finalizar_proceso(pcb);
    }
}

void modificar_grado_multiprogramacion(int nuevo_grado){

    if(g_grado_multiprogramacion > nuevo_grado){
        //Bajar grado, no hacer los post
        g_post_a_saltear_multiprogramacion = g_grado_multiprogramacion - nuevo_grado;
    } else if(g_grado_multiprogramacion < nuevo_grado){
        //Subir grado
        int diferencia = nuevo_grado - g_grado_multiprogramacion;
        for(int i = 0; i < diferencia; i++){
            sem_post(&g_tope_multiprogramacion);
        }
    } else {
        log_info(g_logger, "El grado de multiprogramacion ya es el deseado");
        return;
    }
    g_grado_multiprogramacion = nuevo_grado;
}