#include "memoria.h"
#include "config.h"
#include "global_memoria.h"
#include "utils/files.h"
#include "utils/instrucciones.h"
#include <stdint.h>


t_paqueteMemoria* inicializar_paquete_memoria(){
    t_paqueteMemoria* paqueteMemoria = malloc(sizeof(t_paqueteMemoria));
    paqueteMemoria->PID = 0;
    return paqueteMemoria;
}

void recibir_contexto_de_kernel(int socket, t_paqueteMemoria* paqueteMemoria, t_log*logger){
    t_buffer* buffer = recibir_buffer(socket);

    if (buffer->stream == -1){
        log_error(logger, "Error al recibir contexto de kernel");
        return;
    }

    paqueteMemoria->PID = buffer_read_uint32(buffer);
    paqueteMemoria->path = buffer_read_string(buffer, &paqueteMemoria->path_length);

    free(buffer->stream);
    free(buffer);
}

t_paquete_instruccion* recibir_instruccion(int socket, t_log* logger){
    t_buffer* buffer = recibir_buffer(socket);
    return deserializar_paquete_instruccion(buffer);
}

void enviar_instruccion(int socket, char* instruccion, t_log* logger){
    uint32_t instruccion_length = strlen(instruccion) + 1;
    t_buffer* buffer = buffer_create(sizeof(uint32_t) + instruccion_length);
    buffer_add_string(buffer, instruccion_length,instruccion);
    enviar_buffer(socket, buffer, logger);
}

void retardo_respuesta_memoria(){
    int retardo_respuesta = get_config_memoria()->retardo_respuesta;
    sleep(retardo_respuesta / 1000);
}

void cargar_instrucciones(t_dictionary* memoria_archivo, uint32_t pid, char* path_instrucciones){
    char* instrucciones = leer_archivo_txt(path_instrucciones);
    char** instrucciones_separadas = string_split(instrucciones, "\n");

    dictionary_put(memoria_archivo, string_itoa(pid), instrucciones_separadas);
}

char* leer_instruccion(t_dictionary* memoria_archivo, uint32_t pid, uint32_t* pc){
    char* instruccion = NULL;
    char* pid_string = string_itoa(pid);
    char** instrucciones = dictionary_get(memoria_archivo, pid_string);
    
    if(instrucciones != NULL){
        instruccion = instrucciones[*pc];
    }
    
    return instruccion;
}

void liberar_instrucciones(t_dictionary* memoria_archivo, uint32_t pid){
    char* pid_string = string_itoa(pid);
    char** instrucciones = dictionary_remove(memoria_archivo, pid_string);
    if(instrucciones != NULL){
        free(instrucciones);
    }
    free(pid_string);
}


// procesos

void procesar_carga_instrucciones(int socket){

    t_paqueteMemoria* paqueteMemoria = inicializar_paquete_memoria();  
    recibir_contexto_de_kernel(socket, paqueteMemoria, g_logger);
    cargar_instrucciones(g_memoria_instrucciones, paqueteMemoria->PID, paqueteMemoria->path);
    confirmar_recepcion(socket);
    log_info(g_logger, "Instrucciones cargadas correctamente para el PID %d", paqueteMemoria->PID);
    free(paqueteMemoria);
}

void procesar_pedido_instruccion(int socket){
    t_paquete_instruccion* pedido_instruccion = recibir_instruccion(socket, g_logger);
    char* instruccion = leer_instruccion(g_memoria_instrucciones, pedido_instruccion->pid, &pedido_instruccion->pc);
    log_info(g_logger, "Instruccion leida: %s", instruccion);
    enviar_instruccion(socket, instruccion, g_logger);
    free(pedido_instruccion);
}