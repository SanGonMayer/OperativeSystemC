#include "utils/instrucciones_io.h"
#include "utils/buffer.h"
#include <commons/string.h>
#include <stdint.h>
#include <utils/peticiones_memoria.h>
// unidades de trabajo
// registro direccion
// registro tamaño
// nombre archivo
// registro puntero archivo

t_instruccion_io* crear_instruccion_io(
    int pid, 
    char* instruccion,
    int unidades_trabajo,
    int direccion,
    int tamanio,
    char* nombre_archivo,
    int puntero_archivo,
    t_list* peticionesMemoria){

    t_instruccion_io* instruccion_io = malloc(sizeof(t_instruccion_io));

    instruccion_io->pid = pid;

    instruccion_io->unidades_trabajo = unidades_trabajo;

    instruccion_io->instruccion = string_duplicate(instruccion);

    instruccion_io->direccion = direccion;

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

    instruccion_io->peticionesMemoria = peticionesMemoria;

    return instruccion_io;
}

int sizeTotalDeLista(t_list *peticiones){
    int sizeLista = list_size(peticiones);

    int size_por_peticion = sizeof(uint32_t) + sizeof(int) + sizeof(t_tipo_acceso) + sizeof(uint32_t);

    int sizeTotal =  sizeof(int) 
        + sizeLista * size_por_peticion;

    for (int i = 0; i < sizeLista; i++) {
        t_peticion_acceso_usuario* peticion = list_get(peticiones, i);
        sizeTotal += string_length(peticion->string);
    }

    return sizeTotal;
}

t_buffer* serializar_instruccion_io(t_instruccion_io* instruccion_io){
    int listaSize;
    int sizeTotalLista;
    if(instruccion_io->peticionesMemoria == NULL){
        listaSize = 0;
        sizeTotalLista = sizeof(int);
    }else {
        listaSize = list_size(instruccion_io->peticionesMemoria);
        sizeTotalLista = sizeTotalDeLista(instruccion_io->peticionesMemoria);
    }
    t_buffer* buffer = buffer_create(
    sizeof(int) 
    + sizeof(int) 
    + sizeof(uint32_t) 
    + strlen(instruccion_io->instruccion) + 1 
    + sizeof(int)
    + sizeof(int)
    + sizeof(uint32_t) 
    + strlen(instruccion_io->nombre_archivo) + 1
    + sizeof(int)
    + sizeTotalLista
    );
    buffer_add_int(buffer, instruccion_io->pid);
    buffer_add_int(buffer, instruccion_io->unidades_trabajo);
    buffer_add_string(buffer, strlen(instruccion_io->instruccion) + 1, instruccion_io->instruccion);
    buffer_add_int(buffer, instruccion_io->direccion);
    buffer_add_int(buffer, instruccion_io->tamanio);
    buffer_add_string(buffer, strlen(instruccion_io->nombre_archivo) + 1, instruccion_io->nombre_archivo);
    buffer_add_int(buffer, instruccion_io->puntero_archivo);

    buffer_add_lista(buffer, listaSize, instruccion_io->peticionesMemoria);

    return buffer;
}

t_instruccion_io* deserializar_instruccion_io(t_buffer* buffer){
    int pid = buffer_read_int(buffer);
    uint32_t unidades_trabajo = buffer_read_int(buffer);
    uint32_t length;

    char* instruccion = buffer_read_string(buffer, &length);
    int direccion = buffer_read_int(buffer);
    int tamanio = buffer_read_int(buffer);
    char* nombre_archivo = buffer_read_string(buffer, &length);
    int puntero_archivo = buffer_read_int(buffer);

    int listaSize = buffer_read_int(buffer);
    t_list* peticionesMemoria = list_create();

    buffer_read_lista(buffer, listaSize, peticionesMemoria);

    buffer_destroy(buffer);
    t_instruccion_io* ins_io = crear_instruccion_io(pid,instruccion, unidades_trabajo, direccion, tamanio, nombre_archivo, puntero_archivo,peticionesMemoria);

    free(instruccion);
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
    if(instruccion_io->nombre_archivo != NULL){
        free(instruccion_io->nombre_archivo);
    }
    free(instruccion_io);
}
