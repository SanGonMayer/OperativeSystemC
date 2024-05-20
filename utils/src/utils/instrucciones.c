#include "instrucciones.h"
#include "files.h"
#include "utils/buffer.h"

t_paquete_instruccion* crear_paquete_instruccion(int pid, uint32_t pc){
    t_paquete_instruccion* instruccion = malloc(sizeof(t_paquete_instruccion));
    instruccion->pc = pc;
    instruccion->pid = pid;

    return instruccion;
}

t_buffer* serializar_paquete_instruccion(t_paquete_instruccion* instruccion){
    
    t_buffer* buffer = buffer_create(sizeof(uint32_t) + sizeof(int));
    buffer_add_uint32(buffer, instruccion->pc);
    buffer_add(buffer, &instruccion->pid, sizeof(int));

    return buffer;
}

t_paquete_instruccion* deserializar_paquete_instruccion(t_buffer* buffer){
    // TODO revisar si hay que liberar el buffer

    t_paquete_instruccion* instruccion = malloc(sizeof(t_paquete_instruccion));
    instruccion->pc = buffer_read_uint32(buffer);
    buffer_read(buffer, &instruccion->pid,sizeof(int));

    return instruccion;
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
    /*
    if(instrucciones != NULL){
        instruccion = instrucciones[*pc];
        *pc = *pc + 1;
    }
    */
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

