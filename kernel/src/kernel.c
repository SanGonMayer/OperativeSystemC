#include "kernel.h"
#include "utils/codigo_operacion.h"
#include <commons/log.h>
#include <commons/string.h>
#include <semaphore.h>
#include <string.h>
#include <readline/readline.h>
#include <stdio.h>
#include "global_kernel.h"

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

t_registrosMem recibir_contexto_memoria(int socket){
    t_registrosMem registrosMemoria;
    recv(socket, &registrosMemoria, sizeof(t_registrosMem), MSG_WAITALL);
    return registrosMemoria;
}

void enviar_proceso_a_memoria(t_PCB* pcb, int socketMemoria, t_log* logger){
    t_paquete* paquete = crear_contexto_memoria(pcb);
    enviar_contexto_memoria(paquete, socketMemoria, logger);
    
    // Recibir OK
    int result;
    recv(socketMemoria, &result, sizeof(int), MSG_WAITALL);
    if(result == -1){
        log_error(logger, "Error al recibir el OK de memoria");
    }
}

void iniciar_proceso(char* path, t_queue* cola_new, int* contadorPID){
    (*contadorPID)++;
    t_PCB* pcb = crear_PCB();
    pcb->PID = *contadorPID;
    pcb->estado = NEW;
    pcb->path_length = strlen(path);
    pcb->path = malloc(pcb->path_length);
    strcpy(pcb->path, path);
    queue_push(cola_new, pcb);

    log_info(g_logger, "Cantidad de procesos en NEW: %d", queue_size(cola_new));
    log_info(g_logger, "Proceso %d encolado en NEW", pcb->PID);

    sem_wait(&g_mutex_multiprogramacion);

    if(g_grado_multiprogramacion > 0){
        enviar_proceso_a_ready();
    }
    sem_post(&g_mutex_multiprogramacion);

}

void enviar_proceso_a_ready(){
        t_PCB* pcb = queue_pop(g_cola_new);
        queue_push(g_cola_ready, pcb);
        enviar_proceso_a_memoria(pcb, g_socket_memoria, g_logger);
        pcb-> estado = READY;

        g_grado_multiprogramacion--;

        log_info(g_logger, "Cantidad de procesos en READY: %d", queue_size(g_cola_ready));
        log_info(g_logger, "Proceso %d encolado en READY", pcb->PID);
}

void ejecutar_cpu_FIFO(t_PCB* pcb, int conexion_cpu_dispatch, t_log* logger){
    int error;
    
    error = enviar_pcb(conexion_cpu_dispatch, pcb);
    if (error == -1){
        log_error(logger, "Error al enviar PCB a CPU");
    }else{
        log_info(logger, "PCB enviado a CPU");
    }

    sem_wait(&g_actualizacion_pcb);
    // Recibir PCB
    t_PCB* pcb_recibido = recibir_pcb(conexion_cpu_dispatch);
    if (pcb_recibido == NULL){
        log_error(logger, "Error al recibir PCB de CPU");
        free(pcb_recibido);
    }else{
        log_info(logger, "PCB recibido de CPU");
        actualizar_pcb(pcb, pcb_recibido);
        free(pcb_recibido);
    }
}

void enviar_interrupcion(int socket_interrupt, uint32_t* PID){
    t_paquete* paquete = malloc(sizeof(t_paquete));

    paquete->codigo_operacion = ENVIO_INTERRUPCION;
    paquete->buffer = buffer_create(sizeof(uint32_t));
    buffer_add_uint32(paquete->buffer, *PID);

    void* a_enviar = crear_a_enviar(paquete);
    send(socket_interrupt, a_enviar, paquete->buffer->size + sizeof(int) + sizeof(uint32_t), 0);

	free(a_enviar);
    eliminar_paquete(paquete);
}


void consola_interactiva(t_log *logger){

    char* linea_leida;
    
	linea_leida = readline(">");

    

	while (strcmp(linea_leida, "q")){

    char ** linea_leida_separada = string_split(linea_leida, " ");

    char *funcion = linea_leida_separada[0];
    string_to_upper(funcion);

    int opcion_funciones_consola;

    if (strcmp(funcion, "EJECUTAR_SCRIPT") == 0) {
        opcion_funciones_consola = EJECUTAR_SCRIPT;
    } else if (strcmp(funcion, "INICIAR_PROCESO") == 0) {
        opcion_funciones_consola = INICIAR_PROCESO;
    } else if (strcmp(funcion, "FINALIZAR_PROCESO") == 0) {
        opcion_funciones_consola = FINALIZAR_PROCESO;
    } else if (strcmp(funcion, "DETENER_PLANIFICACION") == 0) {
        opcion_funciones_consola = DETENER_PLANIFICACION;
    } else if (strcmp(funcion, "INICIAR_PLANIFICACION") == 0) {
        opcion_funciones_consola = INICIAR_PLANIFIACION;
    } else if (strcmp(funcion, "MULTIPROGRAMACION") == 0) {
        opcion_funciones_consola = MULTIPROGRAMACION;
    } else if (strcmp(funcion, "PROCESO_ESTADO") == 0) {
        opcion_funciones_consola = PROCESO_ESTADO;
    }else
        log_error(logger, "ingresaste una funcion no valida");

    switch (opcion_funciones_consola) {

        case EJECUTAR_SCRIPT:
            printf("e\n");
            
            break;
        case INICIAR_PROCESO:
            printf("Se seleccionó la opción 2\n");
            char* path = linea_leida_separada[1];
            
            string_trim(&path);

            if(!es_parametro_valido(path)){
                log_error(logger, "El path ingresado no es válido");
                break;
            }

            iniciar_proceso(path, g_cola_new, &g_contador_pid);
            break;
        case FINALIZAR_PROCESO:
            printf("Se seleccionó la opción 3\n");
            break;
        case DETENER_PLANIFICACION:
            printf("Opción no válida\n");
            break;
        case INICIAR_PLANIFIACION:
            printf("Opción no válida\n");
            break;
        case MULTIPROGRAMACION:
            printf("Opción no válida\n");
            break;
        case PROCESO_ESTADO:
            printf("Opción no válida\n");
            break;
    }


        log_info(logger, "me llego la instruccion: %s", linea_leida);
		//log_info(logger, linea_leida);

		linea_leida = readline(">");

        free(funcion);
	}

	free(linea_leida);
    
}