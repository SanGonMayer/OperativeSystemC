#include "memoria.h"
#include "utils/instrucciones.h"
#include <stdint.h>

t_paqueteMemoria* inicializar_paquete_memoria(){
    t_paqueteMemoria* paqueteMemoria = malloc(sizeof(t_paqueteMemoria));
    paqueteMemoria->PID = 0;
    return paqueteMemoria;
}

void recibir_contexto_de_kernel(int socket, t_paqueteMemoria* paqueteMemoria, t_log*logger){
    t_buffer* buffer = recibir_buffer(socket);

    if (buffer->stream == -1){
        log_error(logger, "Error al recibir contexto de kernel");
        return;
    }

    confirmar_recepcion(socket);

    paqueteMemoria->PID = buffer_read_uint32(buffer);
    paqueteMemoria->path_length = buffer_read_uint32(buffer);
    paqueteMemoria->path = buffer_read_string(buffer, &paqueteMemoria->path_length);

    free(buffer->stream);
    free(buffer);
}

t_paquete_instruccion* recibir_instruccion(int socket, t_log* logger){
    t_buffer* buffer = recibir_buffer(socket);
    return deserializar_paquete_instruccion(buffer);
}

void enviar_instruccion(int socket, char* instruccion, t_log* logger){
    uint32_t instruccion_length = strlen(instruccion) + 1;
    t_buffer* buffer = buffer_create(sizeof(uint32_t) + instruccion_length);
    buffer_add_string(buffer, instruccion_length,instruccion);
    enviar_buffer(socket, buffer, logger);
}