#include "utils/instrucciones_io.h"
#include "utils/buffer.h"

t_instruccion_io* crear_instruccion_io(int unidades_trabajo, char* instruccion){
    t_instruccion_io* instruccion_io = malloc(sizeof(t_instruccion_io));
    *instruccion_io = (t_instruccion_io) {
        .unidades_trabajo = unidades_trabajo,
        .instruccion = instruccion
    };

    return instruccion_io;
}

t_buffer* serializar_instruccion_io(t_instruccion_io* instruccion_io){
    t_buffer* buffer = buffer_create(sizeof(uint32_t) + sizeof(uint32_t) + strlen(instruccion_io->instruccion));
    buffer_add_int(buffer, instruccion_io->unidades_trabajo);
    buffer_add_string(buffer, strlen(instruccion_io->instruccion), instruccion_io->instruccion);
    return buffer;
}

t_instruccion_io* deserializar_instruccion_io(t_buffer* buffer){
    uint32_t unidades_trabajo = buffer_read_int(buffer);
    uint32_t length;
    char* instruccion = buffer_read_string(buffer, &length);

    buffer_destroy(buffer);
    return crear_instruccion_io(unidades_trabajo, instruccion);
}

t_buffer* serializar_interfaz(t_interfaz* interfaz){
    t_buffer* buffer = buffer_create(
        sizeof(uint32_t) + 
        strlen(interfaz->tipo) + 
        strlen(interfaz->nombre));
    
    buffer_add_string(buffer, strlen(interfaz->tipo), interfaz->tipo);
    buffer_add_string(buffer, strlen(interfaz->nombre), interfaz->nombre);

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



