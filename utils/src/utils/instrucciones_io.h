#ifndef INSTRUCCIONES_IO_H_
#define INSTRUCCIONES_IO_H_

#include "utils/buffer.h"
#include <commons/collections/queue.h>
#include <semaphore.h>

typedef struct {
    int unidades_trabajo;
    char* instruccion;
    char* direccion;
    int tamanio;
    char* nombre_archivo;
    int puntero_archivo; //REVISAR hay que sacarlo para mi, siempre se pasa desde cpu de donde hasta donde escribir o leer
    t_list* peticionesMemoria;
} t_instruccion_io;

typedef struct {
    char* tipo;
    char* nombre;
} t_interfaz;

typedef struct {
    int fd;
    t_queue *cola;
    t_interfaz* interfaz;
    sem_t semaforo;
    sem_t mutex;
} t_interfaz_conectada;

typedef struct{
    t_instruccion_io* instruccion;
    t_list* peticionesMemoria;
} t_instruccion_stdin;

t_instruccion_io* crear_instruccion_io(
    char* instruccion,
    int unidades_trabajo,
    char* direccion,
    int tamanio,
    char* nombre_archivo,
    t_list* peticionesMemoria);

void destruir_instruccion_io(t_instruccion_io* instruccion_io);

t_buffer* serializar_instruccion_io(t_instruccion_io* instruccion_io);

t_instruccion_io* deserializar_instruccion_io(t_buffer* buffer);

t_buffer* serializar_interfaz(t_interfaz* interfaz);

t_interfaz* deserializar_interfaz(t_buffer* buffer);

#endif   