#include <memoria.h>
#include <commons/log.h>



void iterator(char* value, t_log* logger) {
    log_info(logger,"%s", value);
}

void inicializar_paquete_memoria(){
    t_paqueteMemoria* paqueteMemoria = malloc(sizeof(t_paqueteMemoria));
    paqueteMemoria->PID = 0;

}

int recibir_contexto_de_kernel(int socket, t_paqueteMemoria* paqueteMemoria){
    t_buffer* buffer = malloc(sizeof(t_buffer));
    recv(socket, &buffer->size, sizeof(uint32_t), MSG_WAITALL);
    buffer->stream = malloc(sizeof(buffer->size);
    int result = recv(socket, buffer->stream, buffer->size, MSG_WAITALL);
    
    paqueteMemoria->PID = buffer_read_uint32(buffer);
    paqueteMemoria->path_length = buffer_read_uint32(buffer);
    paqueteMemoria->path = buffer_read_string(buffer, &pcb->path_length);

    free(buffer->stream)
    free(buffer);

    return result;
}

void enviar_posicion_de_codigo(int socket, int posicionDeCodigo){
    
    void* a_enviar = malloc(sizeof(int));
    int offset = 0;

    memcpy(a_enviar + offset, &(posicionDeCodigo), sizeof(int));
    offset += sizeof(int);

    // Por último enviamos
    int result = send(socket, a_enviar, int, 0);

    // No nos olvidamos de liberar la memoria que ya no usaremos
    free(a_enviar);

    return result;

}


