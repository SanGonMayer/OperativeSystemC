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
    sem_wait(&g_notif_largo_plazo);
    sem_post(&g_notif_largo_plazo); 

    t_PCB* pcb = crear_PCB();
    log_info(g_logger,"Creando PCB para el proceso %d", g_contador_pid);
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
    log_info(g_logger, "Encolando proceso %d en NEW, dentro de mutex", pcb->PID);
    queue_push(g_cola_new, pcb);
    sem_post(&g_mutex_cola_new);
    log_info(g_logger, "Cantidad de procesos en NEW: %d", queue_size(g_cola_new));
    log_info(g_logger, "Proceso %d encolado en NEW", pcb->PID);

    sem_wait(&g_tope_multiprogramacion);
    preparar_proceso_a_ready();
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
    sem_wait(&g_notif_corto_plazo);
    sem_post(&g_notif_corto_plazo);
    
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

void esperar_quantum(t_PCB* pcb){
    sleep(pcb->quantum);
    if(g_exec != NULL && g_exec->PID == pcb->PID){
    enviar_interrupcion(g_conexion_cpu_interrupt, pcb->PID);
    }
}

void ejecutar_cpu_RR(t_PCB* pcb){

    int error;
    sem_wait(&g_disponible_exec);
    error = enviar_pcb(g_conexion_cpu_dispatch, pcb);
    g_exec = pcb;

    pthread_t hilo_quantum;
    pthread_create(&hilo_quantum, NULL, (void*)esperar_quantum,pcb);
    pthread_detach(hilo_quantum);

    recibir_pcb_desalojado(pcb);
    pthread_cancel(hilo_quantum);

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
    g_exec = pcb;
    
    pthread_t hilo_quantum;
    timer = temporal_create();
    pthread_create(&hilo_quantum, NULL, (void*)esperar_quantum,pcb);
    pthread_detach(hilo_quantum);

    recibir_pcb_desalojado(pcb);
    temporal_stop(timer);
    g_ms_transcurridos = temporal_gettime(timer);
    sem_post(&g_tiempo_calculado);
    pthread_cancel(hilo_quantum);
    temporal_destroy(timer);
}

void recibir_pcb_desalojado(t_PCB* pcb_ejecutando){
    sem_wait(&g_notif_corto_plazo);
    sem_post(&g_notif_corto_plazo);

    t_PCB* pcb_recibido = recibir_pcb(g_conexion_cpu_dispatch);
    int motivo;
    if (pcb_recibido == NULL){
        log_error(g_logger, "Error al recibir PCB de CPU");
        free(pcb_recibido);
    }else{
        log_info(g_logger, "PCB recibido de CPU");
        actualizar_pcb(pcb_ejecutando, pcb_recibido);
        free(pcb_recibido);
        g_exec = NULL;
    }

    recv(g_conexion_cpu_dispatch, &motivo, sizeof(int), MSG_WAITALL);
    log_info(g_logger, "Motivo de finalizacion: %d", motivo);

    t_desalojo* desalojo = malloc(sizeof(t_desalojo));
    desalojo->pcb = pcb_ejecutando;
    desalojo->motivo = motivo;

    if (desalojo->motivo == SIGNAL || desalojo->motivo == WAIT){
        
        //en caso de que sea un signal o un wait va a recibir el buffer que se mando 
        //desde la funcion ciclo de ejecucion
        //al struct t_desalojo se le agrega el campo char* recurso para no romper la firma de la funcion atender_desalojo
        t_buffer* buffer = recibir_buffer(g_conexion_cpu_dispatch);
        uint32_t length;
        char *recurso_leido = buffer_read_string(buffer, &length);
        desalojo->recurso = string_duplicate(recurso_leido);

        buffer_destroy(buffer);
        free(recurso_leido);
    }

    pthread_t hilo_desalojo;
    pthread_create(&hilo_desalojo, NULL, (void*)atender_desalojo, desalojo);
    pthread_detach(hilo_desalojo);
 
}

void planificador_exit(){
    sem_wait(&g_notif_largo_plazo);
    sem_post(&g_notif_largo_plazo);  

    while(1){
        sem_wait(&g_hay_elementos_en_exit);
        sem_wait(&g_mutex_cola_exit);
        t_PCB* pcb = queue_pop(g_cola_exit);
        sem_post(&g_mutex_cola_exit);
        log_info(g_logger, "Proceso %d finalizado", pcb->PID);
        //TODO Liberarlo de memoria (entrega 3)
        free(pcb);
    }
}

void finalizar_proceso(t_PCB* pcb){
    sem_wait(&g_notif_corto_plazo);
    sem_post(&g_notif_corto_plazo);
    
    pcb->estado = EXIT;
    log_info(g_logger, "Proceso %d finalizado", pcb->PID);
    sem_wait(&g_mutex_cola_exit);
    queue_push(g_cola_exit, pcb);
    sem_post(&g_mutex_cola_exit);
    sem_post(&g_hay_elementos_en_exit);
    sem_post(&g_tope_multiprogramacion);
}

void atender_desalojo(t_desalojo* desalojo){
    sem_wait(&g_notif_corto_plazo);
    sem_post(&g_notif_corto_plazo);
    desalojo->pcb->estado = BLOCKED;
    switch(desalojo->motivo){
        case FINALIZACION:
            finalizar_proceso(desalojo->pcb);
            break;

        case INTERRUPCION: //CLOCK
            enviar_proceso_a_ready(desalojo->pcb);
            break;

        case IO_GEN_SLEEP:
            //Abstraer a un case IO
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

                sem_wait(&g_lista_blocked_gral);
                list_add(g_lista_blocked_gral, item->pcb);
                sem_post(&g_lista_blocked_gral);

                sem_post(&interfaz->semaforo);
                log_info(g_logger, "Proceso %d bloqueado", desalojo->pcb->PID);
                //agrega a la cola de bloquedos y ponerle estado bloqueado al pcb

            }else{
                sem_post(&g_mutex_acceso_interfaces);

                finalizar_proceso(desalojo->pcb);
            }
           
            break;
            
        case IO_STDIN_READ:
            //TODO Abstraer a un case IO 
            if(string_equals_ignore_case(algoritmo_planificacion, "VRR")){
                if(g_ms_transcurridos < desalojo->pcb->quantum){
                sem_wait(&g_tiempo_calculado);
                desalojo->pcb->quantum -= g_ms_transcurridos;
                desalojo->pcb->readyplus = 1;       
                }
            }
            t_buffer* buffer2 = recibir_buffer(g_conexion_cpu_dispatch);
            //Parametros
            int direccion_fisica = buffer_read_int(buffer);
            int tamanio = buffer_read_int(buffer);
            uint32_t* length2 = malloc(sizeof(uint32_t));
            char* nombreInterfaz2 = buffer_read_string(buffer, length);
            
            char* instruccion2 = string_new();
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

                t_instruccion_io* instruccion_io = crear_instruccion_io(instruccion, NULL, NULL, tamanio, NULL, direccion_fisica);

                item->instruccion = malloc(sizeof(t_instruccion_io));
                item->instruccion = instruccion_io;
                queue_push(interfaz->cola, item);

                list_add(g_lista_blocked_gral, item->pcb);

                sem_post(&interfaz->semaforo);
                log_info(g_logger, "Proceso %d bloqueado", desalojo->pcb->PID);
                //agrega a la cola de bloquedos y ponerle estado bloqueado al pcb

            }else{
                sem_post(&g_mutex_acceso_interfaces);
                finalizar_proceso(desalojo->pcb);
            }
            break;

        case IO_STDOUT_WRITE:
            //TODO Abstraer a un case IO 
            if(string_equals_ignore_case(algoritmo_planificacion, "VRR")){
                if(g_ms_transcurridos < desalojo->pcb->quantum){
                sem_wait(&g_tiempo_calculado);
                desalojo->pcb->quantum -= g_ms_transcurridos;
                desalojo->pcb->readyplus = 1;       
                }
            }
            t_buffer* buffer1 = recibir_buffer(g_conexion_cpu_dispatch);
            //Parametros
            int direccion_fisica1 = buffer_read_int(buffer);
            int tamanio1 = buffer_read_int(buffer);
            uint32_t* length1 = malloc(sizeof(uint32_t));
            char* nombreInterfaz1 = buffer_read_string(buffer, length);
            
            char* instruccion1 = string_new();
            //instruccion que deba entender
            instruccion = "IO_STDOUT_WRITE";

            //TODO abstraer a una funcion, es todo igual menos el paquete t_instruccion_io

            sem_wait(&g_mutex_acceso_interfaces);

            if(dictionary_has_key(g_interfaces, nombreInterfaz)){

                t_interfaz_conectada* interfaz = dictionary_get(g_interfaces, nombreInterfaz);

                sem_post(&g_mutex_acceso_interfaces);

                // TODO: checkear que pueda hacer la operacion

                t_parametro_cola_interfaz* item = malloc(sizeof(t_parametro_cola_interfaz));

                item->pcb = desalojo->pcb;

                t_instruccion_io* instruccion_io = crear_instruccion_io(instruccion, NULL, NULL, tamanio, NULL, direccion_fisica);

                item->instruccion = malloc(sizeof(t_instruccion_io));
                item->instruccion = instruccion_io;
                queue_push(interfaz->cola, item);

                sem_wait(&g_lista_blocked_gral);
                list_add(g_lista_blocked_gral, item->pcb);
                sem_post(&g_lista_blocked_gral);

                sem_post(&interfaz->semaforo);
                log_info(g_logger, "Proceso %d bloqueado", desalojo->pcb->PID);
                //agrega a la cola de bloquedos y ponerle estado bloqueado al pcb

            }else{
                sem_post(&g_mutex_acceso_interfaces);
                finalizar_proceso(desalojo->pcb);
            }
            break;

        case SIGNAL:
            char * recurso_signal = string_duplicate(desalojo->recurso);
            manejar_recurso((int)SIGNAL, recurso_signal, desalojo->pcb);
            break;

        case WAIT:
            char * recurso_wait = string_duplicate(desalojo->recurso);
            manejar_recurso((int)WAIT, recurso_wait, desalojo->pcb);
            break;

    free(desalojo);
    }


    sem_post(&g_disponible_exec);
}
void planificador_fifo(){
    sem_wait(&g_notif_corto_plazo);
    sem_post(&g_notif_corto_plazo);
    
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
    sem_wait(&g_notif_corto_plazo);
    sem_post(&g_notif_corto_plazo);
    
    while(1){  
    sem_wait(&g_hay_elementos_en_ready);
    log_info(g_logger, "Planificador RR");
    sem_wait(&g_mutex_cola_ready);
    t_PCB* pcb = queue_pop(g_cola_ready);
    sem_post(&g_mutex_cola_ready);
    ejecutar_cpu_RR(pcb);
    }
}

void planificador_VRR(){
    sem_wait(&g_notif_corto_plazo);
    sem_post(&g_notif_corto_plazo);

    while(1){
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

void enviar_interrupcion(int socket_interrupt, uint32_t* PID){

    t_buffer* buffer = buffer_create(sizeof(uint32_t));
    buffer_add_uint32(buffer, *PID);
    
    t_paquete* paquete = crear_paquete(ENVIO_INTERRUPCION, buffer); 
    enviar_paquete(paquete, socket_interrupt);
}

void iniciar_diccionario_y_listas_recursos(char** recursos, char** recursos_instancias){
    g_diccionario_recursos = dictionary_create();
    g_diccionario_recursos_colas_blocked = dictionary_create();
    int i = 0;
    
    while(recursos[i] != NULL){
        dictionary_put(g_diccionario_recursos, recursos[i], recursos_instancias[i]);
        
        t_queue* cola_bloqueados = queue_create();
        dictionary_put(g_diccionario_recursos_colas_blocked, recursos[i], cola_bloqueados);
        i++;
    }
}

void manejar_recurso(int operacion, char* recurso, t_PCB* pcb){
    
    if (dictionary_has_key(g_diccionario_recursos, recurso))
    {
        char* instancias = dictionary_get(g_diccionario_recursos, recurso);
        int instancias_int = atoi(instancias);

        switch (operacion)
        {
            case 17: //WAIT
                
                
                if(instancias_int > 0){
                    instancias_int--;
                    char* instancias_nuevas = string_itoa(instancias_int);
                    dictionary_put(g_diccionario_recursos, recurso, instancias_nuevas);   
                }
                else{
                    t_queue * cola = (g_diccionario_recursos_colas_blocked, recurso);
                    queue_push(cola, pcb);
                    pcb->estado = BLOCKED;

                    sem_wait(&g_mutex_lista_blocked_gral);
                    list_add(g_lista_blocked_gral, pcb);
                    sem_post(&g_mutex_lista_blocked_gral);

                    free(cola);
                }                
                
                break;

            case 18: //SIGNAL
                instancias_int++;
                char* instancias_nuevas = string_itoa(instancias_int);
                dictionary_put(g_diccionario_recursos, recurso, instancias_nuevas);
                
                t_queue * cola = (g_diccionario_recursos_colas_blocked, recurso);
                
                if(!queue_is_empty(cola)){
                    t_PCB* pcb = queue_pop(cola);
                    pcb->estado = READY;
                    enviar_proceso_a_ready(pcb);
                    eliminar_de_lista_blocked_gral(pcb->PID);
                }

                free(cola);
                free(instancias_nuevas);
                
                break;
        }
    }
    else
    {
        log_error(g_logger, "El recurso: %s no existe", recurso);
    }

}

void eliminar_de_lista_blocked_gral(t_PCB* pcb){
      
    sem_wait(g_lista_blocked_gral);
    if (!list_remove_element(g_lista_blocked_gral, pcb));
        printf("No se pudo eliminar el PID %d de la lista de bloqueados generales\n", pcb->PID);
    sem_post(g_lista_blocked_gral);
}

void listar_procesos(){
    //NEW
    t_list_iterator* iterador_new = list_iterator_create(g_cola_new);
    
    while(list_iterator_has_next(iterador_new)){
        t_PCB* pcb = list_iterator_next(iterador_new);
        printf("%-25s%-25s\n", "NEW", pcb->PID);
    }

    //READY
    t_list_iterator* iterador_ready = list_iterator_create(g_cola_ready);
    
    while(list_iterator_has_next(iterador_ready)){
        t_PCB* pcb = list_iterator_next(iterador_ready);
        printf("%-25s%-25s\n", "READY", pcb->PID);
    }

    //BLOCKED
    t_list_iterator* iterador_blocked = list_iterator_create(g_lista_blocked_gral);
    
    while(list_iterator_has_next(iterador_blocked)){
        t_PCB* pcb = list_iterator_next(iterador_blocked);
        printf("%-25s%-25s\n", "BLOCKED", pcb->PID);
    }

    //EXIT
    t_list_iterator* iterador_exit = list_iterator_create(g_cola_exit);
     while(list_iterator_has_next(iterador_exit)){
        t_PCB* pcb = list_iterator_next(iterador_exit);
        printf("%-25s%-25s\n", "READY", pcb->PID);
    }


    list_iterator_destroy(iterador_new);
    list_iterator_destroy(iterador_ready);
    list_iterator_destroy(iterador_blocked);
    list_iterator_destroy(iterador_exit);
}
