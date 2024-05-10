#include "cpu.h"

char* etapa_fetch(int socket, t_PCB* pcb, t_log* logger){
    char* instruccion;
    //Envio posicion de memoria + program counter
    int result = enviar_posicion_de_codigo(socket, pcb->registrosMem.codigo + pcb->registrosCPU.pc);
    if(result == -1){
        log_error(logger, "Error al enviar posicion de memoria");
        return NULL;
    }
    //Recibo instruccion
    result = recibir_instruccion(socket, instruccion);
    if(result == -1){
        log_error(logger, "Error al recibir instruccion");
        return NULL;
    }
    //Sumar program counter
    pcb->registrosCPU.pc++;
    return instruccion;
}

int enviar_posicion_de_codigo(int socket, uint32_t posicionDeCodigo){
    t_buffer * buffer = buffer_create(sizeof(uint32_t)); 
    buffer_add_uint32(buffer, posicionDeCodigo);
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = 3;
    paquete->buffer = buffer;

    void* a_enviar = crear_a_enviar(paquete);
    int result = send(socket, a_enviar, buffer->size + sizeof(int) + sizeof(uint32_t), 0);
    return result;
}

int recibir_instruccion(int socket,char* instruccion){
    t_buffer* buffer = recibir_buffer(socket);
    instruccion = buffer_read_string(buffer, &buffer->size);
    return 0;
}