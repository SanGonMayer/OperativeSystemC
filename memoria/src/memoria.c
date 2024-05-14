#include "memoria.h"
#include "utils/instrucciones.h"
#include <stdint.h>

t_paqueteMemoria* inicializar_paquete_memoria(){
    t_paqueteMemoria* paqueteMemoria = malloc(sizeof(t_paqueteMemoria));
    paqueteMemoria->PID = 0;
    return paqueteMemoria;
}

void recibir_contexto_de_kernel(int socket, t_paqueteMemoria* paqueteMemoria){
    t_buffer* buffer = recibir_buffer(socket);
    
    paqueteMemoria->PID = buffer_read_uint32(buffer);
    paqueteMemoria->path_length = buffer_read_uint32(buffer);
    paqueteMemoria->path = buffer_read_string(buffer, &paqueteMemoria->path_length);

    free(buffer->stream);
    free(buffer);
}

void responder_ok(int socket){
    uint32_t ok = 1;
    send(socket, &ok, sizeof(uint32_t), 0);
}

t_paquete_instruccion* recibir_posicion_de_codigo(int socket, t_log* logger){
    t_buffer* buffer = recibir_buffer(socket);
    return deserializar_paquete_instruccion(buffer);
}

void enviar_instruccion(int socket, char* instruccion, t_log* logger){
    uint32_t instruccion_length = strlen(instruccion) + 1;
    t_buffer* buffer = buffer_create(sizeof(uint32_t) + instruccion_length);
    buffer_add_string(buffer, instruccion_length,instruccion);
    enviar_buffer(socket, buffer, logger);
}

uint32_t abrir_archivo(char* path){
    return 1;
}

void leer_archivo(uint32_t posicionDeCodigo, char* instruccion){
  
}