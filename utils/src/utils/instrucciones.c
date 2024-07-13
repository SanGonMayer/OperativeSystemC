#include "instrucciones.h"
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

    t_paquete_instruccion* instruccion = malloc(sizeof(t_paquete_instruccion));
    instruccion->pc = buffer_read_uint32(buffer);
    buffer_read(buffer, &instruccion->pid,sizeof(int));

    return instruccion;
}

void destroy_paquete_instruccion(t_paquete_instruccion* instruccion){
    free(instruccion);
}
