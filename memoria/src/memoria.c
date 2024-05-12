#include "memoria.h"

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

void enviar_posicion_de_codigo(int socket, uint32_t posicionDeCogido){
    
    void* a_enviar = malloc(sizeof(int));
    int offset = 0;

    memcpy(a_enviar + offset, &posicionDeCogido, sizeof(int));
    offset += sizeof(int);

    send(socket, a_enviar, sizeof(uint32_t), 0);

    free(a_enviar);
}

uint32_t recibir_posicin_de_codigo(int socket, t_log* logger){
    t_buffer* buffer = recibir_buffer(socket);
    uint32_t posicionDeCodigo = buffer_read_uint32(buffer);
    int result = recv(socket, &posicionDeCodigo, sizeof(uint32_t), 0);
    if(result == -1){
        log_error(logger, "Error al recibir la posicion de codigo");
    }
    return posicionDeCodigo;
}

void enviar_instruccion(int socket, char* instruccion, t_log* logger){
    uint32_t instruccion_length = strlen(instruccion) + 1;
    t_buffer* buffer = buffer_create(instruccion_length);
    buffer_add_string(buffer, instruccion_length,instruccion);
    enviar_buffer(socket, buffer, logger);
}

uint32_t abrir_archivo(char* path){
    return 1;
}

void leer_archivo(uint32_t posicionDeCodigo, char* instruccion){
  
}