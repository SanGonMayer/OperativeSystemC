#include "kernel.h"
#include "utils/codigo_operacion.h"
#include <string.h>
#include <readline/readline.h>
#include <stdio.h>

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
    t_PCB* pcb = crear_PCB();
    pcb->PID = *contadorPID;
    pcb->estado = NEW;
    pcb->path_length = strlen(path)+1;
    pcb->path = malloc(pcb->path_length);
    strcpy(pcb->path, path);
    queue_push(cola_new, pcb);
}

void enviar_proceso_a_ready(t_queue* cola_new, t_queue* cola_ready, int socketMemoria, t_log* logger){
        t_PCB* pcb = queue_pop(cola_new);
        queue_push(cola_ready, pcb);
        enviar_proceso_a_memoria(pcb, socketMemoria, logger);
        pcb-> estado = READY;
}

void ejecutar_cpu_FIFO(t_PCB* pcb, int conexion_cpu_dispatch, t_log* logger){
    t_PCB* pcb_auxiliar = crear_PCB();
    int err;
    
    err = enviar_pcb(conexion_cpu_dispatch, pcb);
    //log_info(logger, "PCB ENVIADA: %d", pcb->PID);
    
    // actualizar_pcb(pcb, pcb_auxiliar);
    //log_info(logger, "PCB RECIBIDA: %d", pcb->PID);
    free(pcb_auxiliar);
}

void consola_interactiva(t_log *logger){
    
    char* linea_leida;
    
	linea_leida = readline(">");

	while (strcmp(linea_leida, "q")){

    //falta cortar el string para sacar el path
    char *funcion = strtok(linea_leida," ");

    char * parametro = strtok(NULL," "); //tendrá el valor del pid o el path en caso que corresponda, en caso de que lafuncion sea si parametro va a tener el valor null

    int opcion_funciones_consola;

    if (strcmp(linea_leida, "EJECUTAR_STRIPT") == 0) {
        opcion_funciones_consola = EJECUTAR_STRIPT;
    } else if (strcmp(linea_leida, "INICIAR_PROCESO") == 0) {
        opcion_funciones_consola = INICIAR_PROCESO;
    } else if (strcmp(linea_leida, "FINALIZAR_PROCESO") == 0) {
        opcion_funciones_consola = FINALIZAR_PROCESO;
    } else if (strcmp(linea_leida, "DETENER_PLANIFICACION") == 0) {
        opcion_funciones_consola = DETENER_PLANIFICACION;
    } else if (strcmp(linea_leida, "INICIAR_PLANIFICACION") == 0) {
        opcion_funciones_consola = INICIAR_PLANIFIACION;
    } else if (strcmp(linea_leida, "MULTIPROGRAMACION") == 0) {
        opcion_funciones_consola = MULTIPROGRAMACION;
    } else if (strcmp(linea_leida, "PROCESO_ESTADO") == 0) {
        opcion_funciones_consola = PROCESO_ESTADO;
    }else 
        log_error(logger, "ingresaste una funcion no valida");

    switch (opcion_funciones_consola) {

        case EJECUTAR_STRIPT:
            printf("e\n");
            break;
        case INICIAR_PROCESO:
            printf("Se seleccionó la opción 2\n");
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
        free(parametro);
	}

	free(linea_leida);
    
}