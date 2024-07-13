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

void destroy_paquete_instruccion(t_paquete_instruccion* instruccion);

#endif /* INSTRUCCIONES_H_ */      