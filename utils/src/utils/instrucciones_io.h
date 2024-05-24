#ifndef INSTRUCCIONES_IO_H_
#define INSTRUCCIONES_IO_H_

#include "utils/buffer.h"


typedef struct {
    uint32_t unidades_trabajo;
    char* instruccion;
} t_instruccion_io;

t_instruccion_io* crear_instruccion_io(uint32_t unidades_trabajo, char* instruccion);

t_buffer* serializar_instruccion_io(t_instruccion_io* instruccion_io);

t_instruccion_io* deserializar_instruccion_io(t_buffer* buffer);

#endif   