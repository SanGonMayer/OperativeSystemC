#ifndef INSTRUCCIONES_H_
#define INSTRUCCIONES_H_

#include "utils/buffer.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <commons/collections/dictionary.h>
#include <commons/string.h>

typedef struct {
    uint32_t pc;
    int pid;
} t_paquete_instruccion;


t_paquete_instruccion* crear_paquete_instruccion(int pid, uint32_t pc);

t_buffer* serializar_paquete_instruccion(t_paquete_instruccion* pcb);

t_paquete_instruccion* deserializar_paquete_instruccion(t_buffer* buffer);

void cargar_instrucciones(t_dictionary* memoria_archivo, uint32_t pid, char* path_instrucciones);

char* leer_instruccion(t_dictionary* memoria_archivo, uint32_t pid, uint32_t* pc);

void liberar_instrucciones(t_dictionary* memoria_archivo, uint32_t pid);

#endif /* INSTRUCCIONES_H_ */      