#ifndef INSTRUCCIONES_IO_H_
#define INSTRUCCIONES_IO_H_

#include "utils/buffer.h"
#include <commons/collections/queue.h>


typedef struct {
    int unidades_trabajo;
    char* instruccion;
} t_instruccion_io;

typedef struct {
    char* tipo;
    char* nombre;
} t_interfaz;

typedef struct {
    int fd;
    t_queue *cola;
    t_interfaz* interfaz;
} t_interfaz_conectada;

t_instruccion_io* crear_instruccion_io(int unidades_trabajo, char* instruccion);

t_buffer* serializar_instruccion_io(t_instruccion_io* instruccion_io);

t_instruccion_io* deserializar_instruccion_io(t_buffer* buffer);

t_buffer* serializar_interfaz(t_interfaz* interfaz);

t_interfaz* deserializar_interfaz(t_buffer* buffer);

#endif   