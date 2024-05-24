#include "utils/instrucciones_io.h"
#include "utils/buffer.h"

t_instruccion_io* crear_instruccion_io(uint32_t unidades_trabajo, char* instruccion){
    t_instruccion_io* instruccion_io = malloc(sizeof(t_instruccion_io));
    instruccion_io->unidades_trabajo = unidades_trabajo;
    instruccion_io->instruccion = instruccion;
    return instruccion_io;
}

t_buffer* serializar_instruccion_io(t_instruccion_io* instruccion_io){
    t_buffer* buffer = buffer_create(sizeof(uint32_t) + sizeof(uint32_t) + strlen(instruccion_io->instruccion));
    buffer_add_uint32(buffer, instruccion_io->unidades_trabajo);
    buffer_add_string(buffer, strlen(instruccion_io->instruccion), instruccion_io->instruccion);
    return buffer;
}

t_instruccion_io* deserializar_instruccion_io(t_buffer* buffer){
    uint32_t unidades_trabajo = buffer_read_uint32(buffer);
    uint32_t length;
    char* instruccion = buffer_read_string(buffer, &length);

    buffer_destroy(buffer);
    return crear_instruccion_io(unidades_trabajo, instruccion);
}