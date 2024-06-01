#include "memoria_instrucciones.h"
#include "memoria.h"
#include "global_memoria.h"
#include "utils/files.h"
#include <commons/collections/dictionary.h>
#include <time.h>

static t_dictionary* memoria_instrucciones = NULL;

t_dictionary* get_memoria_instrucciones(){
    if(memoria_instrucciones == NULL){
        memoria_instrucciones = dictionary_create();
    }
    return memoria_instrucciones;
}

void cargar_instrucciones(uint32_t pid, char* path_instrucciones){

    t_dictionary* memoria_archivo = get_memoria_instrucciones();
    char* instrucciones = leer_archivo_txt(path_instrucciones);
    char** instrucciones_separadas = string_split(instrucciones, "\n");

    dictionary_put(memoria_archivo, string_itoa(pid), instrucciones_separadas);
}

char* leer_instruccion(uint32_t pid, uint32_t* pc){

    t_dictionary* memoria_archivo = get_memoria_instrucciones();

    char* instruccion = NULL;
    char* pid_string = string_itoa(pid);
    char** instrucciones = dictionary_get(memoria_archivo, pid_string);
    
    if(instrucciones != NULL){
        instruccion = instrucciones[*pc];
    }
    
    return instruccion;
}

void liberar_instrucciones(uint32_t pid){
    t_dictionary* memoria_archivo = get_memoria_instrucciones();
    char* pid_string = string_itoa(pid);
    char** instrucciones = dictionary_remove(memoria_archivo, pid_string);
    if(instrucciones != NULL){
        free(instrucciones);
    }
    free(pid_string);
}

void procesar_carga_instrucciones(int socket){

    t_paqueteMemoria* paqueteMemoria = inicializar_paquete_memoria();  
    recibir_contexto_de_kernel(socket, paqueteMemoria, g_logger);
    cargar_instrucciones(paqueteMemoria->PID, paqueteMemoria->path);
    confirmar_recepcion(socket);
    log_info(g_logger, "Instrucciones cargadas correctamente para el PID %d", paqueteMemoria->PID);
    free(paqueteMemoria);
}

void procesar_pedido_instruccion(int socket){
    t_paquete_instruccion* pedido_instruccion = recibir_instruccion(socket, g_logger);
    char* instruccion = leer_instruccion(pedido_instruccion->pid, &pedido_instruccion->pc);
    log_info(g_logger, "Instruccion leida: %s", instruccion);
    enviar_instruccion(socket, instruccion, g_logger);
    free(pedido_instruccion);
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