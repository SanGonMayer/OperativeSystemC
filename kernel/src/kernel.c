#include "kernel.h"
#include "cola_exit.h"
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
    // sem_wait(&g_notif_largo_plazo);
    // sem_post(&g_notif_largo_plazo); 

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
    // Preguntar hilo desalojo
    crear_hilo_test();

    pthread_t* hilo_desalojo = malloc(sizeof(pthread_t));
    log_info(g_logger, "Creando hilo desalojo");
    
    int result = pthread_create(hilo_desalojo, NULL, (void*)&atender_desalojo, desalojo);
    pthread_detach(*hilo_desalojo);

    log_info(g_logger, "Hilo creado con resultado %d y numero %lu", result, *hilo_desalojo);
 
}

void test(){
    sleep(60);
}

void crear_hilo_test(){
    pthread_t* hilo_test = malloc(sizeof(pthread_t));
    int result = pthread_create(hilo_test, NULL, (void*)&test, NULL);
    pthread_detach(*hilo_test);
}

void planificador_exit(){
    // sem_wait(&g_notif_largo_plazo);
    // sem_post(&g_notif_largo_plazo);  

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
    // sem_wait(&g_notif_corto_plazo);
    // sem_post(&g_notif_corto_plazo);
    
    pcb->estado = EXIT;
    log_info(g_logger, "Proceso %d finalizado", pcb->PID);
    
    agregar_a_cola_exit(pcb);
    
    log_info(g_logger, "Finaliza el proceso %d - Motivo: %s", pcb->PID, "SUCCESS");
    sem_post(&g_tope_multiprogramacion);
}

void atender_desalojo(t_desalojo* desalojo){
    // sem_wait(&g_notif_corto_plazo);
    // sem_post(&g_notif_corto_plazo);
    log_info(g_logger, "Atendiendo desalojo de proceso %d", desalojo->pcb->PID);
    desalojo->pcb->estado = BLOCKED;
    switch(desalojo->motivo){
        case FINALIZACION:
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
            char* instruccion = string_new();
            liberar_cola_exec();
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

            buffer_destroy(buffer);
            free(length);
            free(nombreInterfaz);
            free(instruccion);

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
            int direccion_fisica = buffer_read_int(buffer);
            int tamanio = buffer_read_int(buffer);
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
            int direccion_fisica = buffer_read_int(buffer);
            int tamanio = buffer_read_int(buffer);
            uint32_t* length = malloc(sizeof(uint32_t));
            char* nombreInterfaz = buffer_read_string(buffer, length);
            liberar_cola_exec();
            char* instruccion = string_new();
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
        }
        case SIGNAL:
        {
            liberar_cola_exec();
            char * recurso_signal = string_duplicate(desalojo->recurso);
            manejar_recurso((int)SIGNAL, recurso_signal, desalojo->pcb);
            break;
        }
        case WAIT:
        {
            liberar_cola_exec();
            char * recurso_wait = string_duplicate(desalojo->recurso);
            manejar_recurso((int)WAIT, recurso_wait, desalojo->pcb);
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
    // sem_wait(&g_notif_corto_plazo);
    // sem_post(&g_notif_corto_plazo);
    
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
    // sem_wait(&g_notif_corto_plazo);
    // sem_post(&g_notif_corto_plazo);
    
    while(1){
    sem_wait(&g_hay_elementos_en_ready);
    sem_wait(&g_mutex_cola_ready);
    log_info(g_logger, "RR - Hay procesos en ready - %d", queue_size(g_cola_ready));
    t_PCB* pcb = queue_pop(g_cola_ready);
    sem_post(&g_mutex_cola_ready);
    ejecutar_cpu_RR(pcb);
    }
}

void planificador_VRR(){
    // sem_wait(&g_notif_corto_plazo);
    // sem_post(&g_notif_corto_plazo);

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

void enviar_interrupcion(int socket_interrupt, uint32_t* PID, uint32_t motivo){

    t_buffer* buffer = buffer_create(sizeof(uint32_t));
    buffer_add_uint32(buffer, *PID);
    buffer_add_uint32(buffer, motivo);
    
    t_paquete* paquete = crear_paquete(ENVIO_INTERRUPCION, buffer); 
    enviar_paquete(paquete, socket_interrupt);
}

void iniciar_diccionario_y_listas_recursos(char** recursos, char** recursos_instancias){
    g_diccionario_recursos = dictionary_create();
    int i = 0;
    
    while(recursos[i] != NULL){
        t_recurso *recurso = malloc(sizeof(t_recurso));
        recurso->nombre = recursos[i];
        recurso->instancias = atoi(recursos_instancias[i]);
        recurso->cola = queue_create();
        sem_init(&recurso->semafor_cola, 0, 0);

        dictionary_put(g_diccionario_recursos, recursos[i], recurso);
        
        i++;
    }
}

void manejar_recurso(int operacion, char* nombre_recurso, t_PCB* pcb){
    
    if (dictionary_has_key(g_diccionario_recursos, nombre_recurso))
    {
        t_recurso *recurso = dictionary_remove(g_diccionario_recursos, nombre_recurso);
        int instancias_int = recurso->instancias;

        switch (operacion)
        {
            case 17: //WAIT
                
                if(instancias_int > 0){
                    instancias_int--;
                    recurso->instancias = instancias_int;
                    dictionary_put(g_diccionario_recursos, nombre_recurso, recurso );   
                }
                else{
                    t_queue * cola = recurso->cola;
                    
                    sem_wait(&recurso->semafor_cola);
                    queue_push(cola, pcb);
                    sem_post(&recurso->semafor_cola);
                    
                    pcb->estado = BLOCKED;

                    sem_wait(&g_mutex_lista_blocked_gral);
                    list_add(g_lista_blocked_gral, pcb);
                    sem_post(&g_mutex_lista_blocked_gral);

                    free(cola);
                }                
                
                break;

            case 18: //SIGNAL
                instancias_int++;
                recurso->instancias = instancias_int;
                dictionary_put(g_diccionario_recursos, nombre_recurso, recurso);
                
                t_queue * cola = recurso->cola;
                
                if(!queue_is_empty(cola)){
                    sem_wait(&recurso->semafor_cola);
                    t_PCB* pcb = queue_pop(cola);
                    sem_post(&recurso->semafor_cola);

                    pcb->estado = READY;
                    enviar_proceso_a_ready(pcb);
                    eliminar_de_lista_blocked_gral(pcb->PID);
                }

                free(cola);
                
                break;
        }
    }
    else
    {
        log_error(g_logger, "El recurso: %s no existe", nombre_recurso);
    }

}

void eliminar_de_lista_blocked_gral(u_int32_t pid){
      
    sem_wait(&g_lista_blocked_gral);
    if (!list_remove_element(g_lista_blocked_gral, &pid));
        printf("No se pudo eliminar el PID %d de la lista de bloqueados generales\n", pid);
    sem_post(&g_lista_blocked_gral);
}

void listar_procesos(){
    printf("%-25s%-25s\n", "NEW", g_exec->PID);

    t_list_iterator* iterador_gral = list_iterator_create(g_lista_procesos_gral);
    
    while(list_iterator_has_next(iterador_gral)){
        t_PCB* pcb = list_iterator_next(iterador_gral);
        
        switch (pcb->estado)
        {
            case NEW:
                printf("%-25s%-25s\n", "NEW", pcb->PID);
                break;
            
            case READY:
                printf("%-25s%-25s\n", "READY", pcb->PID);
                break;
            
            case EXIT:
                printf("%-25s%-25s\n", "EXIT", pcb->PID);
                break;
            
            case READYPLUS:
                printf("%-25s%-25s\n", "READYPLUS", pcb->PID);
                break;
        }
            
    }
        

    //BLOCKED
    t_list_iterator* iterador_blocked = list_iterator_create(g_lista_blocked_gral);
    
    while(list_iterator_has_next(iterador_blocked)){
        uint32_t pid = (uint32_t)list_iterator_next(iterador_blocked);
        printf("%-25s%-25s\n", "BLOCKED", pid);
    }

    list_iterator_destroy(iterador_gral);
    list_iterator_destroy(iterador_blocked);
}

t_PCB* buscar_y_eliminar_pid_en_cola(uint32_t pid,t_queue* cola) {
    if (queue_is_empty(cola)) {
        return NULL;
    }

    // Crear una cola auxiliar para preservar el orden original
    t_queue* cola_aux = queue_create();
    t_PCB* pcb_encontrado = NULL;

    // Recorrer la cola original
    while (!queue_is_empty(cola)) {
        t_PCB* pcb_actual = queue_pop(cola);
        
        if (pcb_actual->PID == pid) {
            pcb_encontrado = pcb_actual;
            // No agregamos el PCB encontrado a la cola auxiliar para eliminarlo de la original
            continue;
        }
        
        // Agregar el PCB actual a la cola auxiliar
        queue_push(cola_aux, pcb_actual);
    }

    // Restaurar los elementos en la cola original
    cola = cola_aux;

    // Liberar la cola auxiliar
    queue_destroy(cola_aux);

    return pcb_encontrado;
}

t_PCB* buscar_en_diccionario_interfaces(uint32_t pid, t_dictionary* interfaces){
    t_PCB* pcb = NULL;

    t_list* lista_interfaces = dictionary_elements(interfaces);
    t_list_iterator* iterator = list_iterator_create(lista_interfaces);
     while(list_iterator_has_next(iterator)){
        t_interfaz_conectada* interfaz = list_iterator_next(lista_interfaces);
        //TODO agregar semaforo para acceso
        pcb = buscar_y_eliminar_pid_en_cola(pid, interfaz->cola);
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
        t_queue* cola_recurso = list_iterator_next(lista_recursos);
        //TODO agregar semaforo para acceso
        pcb = buscar_y_eliminar_pid_en_cola(pid, cola_recurso);
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
    if(pcb != NULL){
        return pcb; 
    }

    sem_wait(&g_mutex_cola_ready);
    pcb = buscar_y_eliminar_pid_en_cola(pid, g_cola_ready);
    sem_post(&g_mutex_cola_ready);
    if(pcb != NULL){
        return pcb; 
    }

    if(string_equals_ignore_case(algoritmo_planificacion, "VRR")){
        sem_wait(&g_mutex_cola_auxiliar);
        pcb = buscar_y_eliminar_pid_en_cola(pid, g_cola_auxiliar);
        sem_post(&g_mutex_cola_auxiliar);
    }


    if(g_exec != NULL && g_exec->PID == pid){
        pcb = g_exec;
    }
    
    if(pcb != NULL){
        //TODO enviar interrupcion y sacarlo de exec
        return pcb;
    }

    //las io
    // pcb = buscar_en_diccionario_interfaces(pid, g_interfaces);

    //los recursos
    //pcb = buscar_en_diccionario_recursos(pid, g_diccionario_recursos_colas_blocked);

    //TODO hacer el de cola blocked
    //wait(lista blocked general)
    //-> Cola de la interfaz IO
    //-> Semaforo propio
    //-> sem wait(semaforo de la cola)
    //Revisar si es necesario
    /*pcb = buscar_y_eliminar_pid_en_cola(pid, g_cola_exit);
    if(pcb != NULL){
        return pcb; 
    }
    */

    return pcb;
}

