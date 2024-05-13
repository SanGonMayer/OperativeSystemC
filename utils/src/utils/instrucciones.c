#include "instrucciones.h"
#include "files.h"


void cargar_instrucciones(t_dictionary* memoria_archivo, uint32_t pid, char* path_instrucciones){
    char* instrucciones = leer_archivo_txt(path_instrucciones);
    char** instrucciones_separadas = string_split(instrucciones, "\n");
    int i = 0;
    dictionary_put(memoria_archivo, string_itoa(pid), instrucciones_separadas);
}

char* leer_instruccion(t_dictionary* memoria_archivo, uint32_t pid, int* pc){
    char* instruccion = NULL;
    char* pid_string = string_itoa(pid);
    char** instrucciones = dictionary_get(memoria_archivo, pid_string);
    if(instrucciones != NULL){
        instruccion = instrucciones[*pc];
        *pc = *pc + 1;
    }
    free(pid_string);
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