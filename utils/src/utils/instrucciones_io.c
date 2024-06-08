#include "utils/instrucciones_io.h"
#include "utils/buffer.h"
#include <commons/string.h>
#include <stdint.h>

// unidades de trabajo
// registro direccion
// registro tamaño
// nombre archivo
// registro puntero archivo

t_instruccion_io* crear_instruccion_io(
    char* instruccion,
    int unidades_trabajo,
    char* direccion,
    int tamanio,
    char* nombre_archivo,
    int puntero_archivo){

    t_instruccion_io* instruccion_io = malloc(sizeof(t_instruccion_io));

    instruccion_io->unidades_trabajo = unidades_trabajo;

    instruccion_io->instruccion = string_duplicate(instruccion);

    if(direccion != NULL){
        instruccion_io->direccion = string_duplicate(direccion);
    }
    else
    {
        instruccion_io->direccion = malloc(sizeof("1"));
        instruccion_io->direccion = "1";
    }

    instruccion_io->tamanio = tamanio;

    if(nombre_archivo != NULL){
        instruccion_io->nombre_archivo = string_duplicate(nombre_archivo);
    }
    else
    {
        instruccion_io->nombre_archivo = malloc(sizeof("1"));
        instruccion_io->nombre_archivo = "1";
    }

    instruccion_io->puntero_archivo = puntero_archivo;

    return instruccion_io;
}

t_buffer* serializar_instruccion_io(t_instruccion_io* instruccion_io){
    t_buffer* buffer = buffer_create(
    sizeof(int) 
    + sizeof(uint32_t) 
    + strlen(instruccion_io->instruccion) + 1 
    + sizeof(uint32_t) 
    + strlen(instruccion_io->direccion) + 1
    + sizeof(int)
    + sizeof(uint32_t) 
    + strlen(instruccion_io->nombre_archivo) + 1
    + sizeof(int)
    );
    buffer_add_int(buffer, instruccion_io->unidades_trabajo);
    buffer_add_string(buffer, strlen(instruccion_io->instruccion) + 1, instruccion_io->instruccion);
    buffer_add_string(buffer, strlen(instruccion_io->direccion) + 1, instruccion_io->direccion);
    buffer_add_int(buffer, instruccion_io->tamanio);
    buffer_add_string(buffer, strlen(instruccion_io->nombre_archivo) + 1, instruccion_io->nombre_archivo);
    buffer_add_int(buffer, instruccion_io->puntero_archivo);

    return buffer;
}

t_instruccion_io* deserializar_instruccion_io(t_buffer* buffer){
    uint32_t unidades_trabajo = buffer_read_int(buffer);
    uint32_t length;
    char* instruccion = buffer_read_string(buffer, &length);
    char* direccion = buffer_read_string(buffer, &length);
    int tamanio = buffer_read_int(buffer);
    char* nombre_archivo = buffer_read_string(buffer, &length);
    int puntero_archivo = buffer_read_int(buffer);

    buffer_destroy(buffer);
    t_instruccion_io* ins_io = crear_instruccion_io(instruccion, unidades_trabajo, direccion, tamanio, nombre_archivo, puntero_archivo);

    free(instruccion);
    free(direccion);
    free(nombre_archivo);
    return ins_io;
}

t_buffer* serializar_interfaz(t_interfaz* interfaz){
    t_buffer* buffer = buffer_create(
        sizeof(uint32_t) + 
        sizeof(uint32_t) +
        strlen(interfaz->tipo) + 1 + 
        strlen(interfaz->nombre) + 1);
    
    buffer_add_string(buffer, strlen(interfaz->tipo) + 1, interfaz->tipo);
    buffer_add_string(buffer, strlen(interfaz->nombre) + 1, interfaz->nombre);

    return buffer;
}
// posible warning de memoria
t_interfaz* deserializar_interfaz(t_buffer* buffer){
    t_interfaz* interfaz = malloc(sizeof(t_interfaz));

    uint32_t length;
    interfaz->tipo = buffer_read_string(buffer, &length);
    interfaz->nombre = buffer_read_string(buffer, &length);

    buffer_destroy(buffer);
    return interfaz;
}


void destruir_instruccion_io(t_instruccion_io* instruccion_io){
    if(instruccion_io->instruccion != NULL){
        free(instruccion_io->instruccion);
    }
    if(instruccion_io->direccion != NULL){
        free(instruccion_io->direccion);
    }
    if(instruccion_io->nombre_archivo != NULL){
        free(instruccion_io->nombre_archivo);
    }
    free(instruccion_io);
}
