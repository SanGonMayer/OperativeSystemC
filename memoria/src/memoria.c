#include "memoria.h"

void inicializar_paquete_memoria(){
    t_paqueteMemoria* paqueteMemoria = malloc(sizeof(t_paqueteMemoria));
    paqueteMemoria->PID = 0;
}

void recibir_contexto_de_kernel(int socket, t_paqueteMemoria* paqueteMemoria){
    t_buffer* buffer = recibir_buffer(socket);
    
    paqueteMemoria->PID = buffer_read_uint32(buffer);
    paqueteMemoria->path_length = buffer_read_uint32(buffer);
    paqueteMemoria->path = buffer_read_string(buffer, &pcb->path_length);

    free(buffer->stream);
    free(buffer);
}

void enviar_posicion_de_codigo(int socket, int posicionDeCogido){
    
    void* a_enviar = malloc(sizeof(int));
    int offset = 0;

    memcpy(a_enviar + offset, &posicionDeCogido, sizeof(int));
    offset += sizeof(int);

    send(socket, a_enviar, int, 0);

    free(a_enviar);
}


