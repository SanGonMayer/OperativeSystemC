#include "cpu.h"
#include "utils/buffer.h"
#include "utils/client.h"
#include "utils/codigo_operacion.h"
#include "utils/instrucciones.h"
#include <stdint.h>

char* etapa_fetch(int socket, t_PCB* pcb, t_log* logger){
    char* instruccion;
    //Envio posicion de memoria + program counter

    instruccion = pedir_instruccion(socket, pcb, logger);

    //Sumar program counter
    pcb->registrosCPU.pc++;
    return instruccion;
}

int responder_ok(int socket, uint32_t posicionDeCodigo){
    t_buffer * buffer = buffer_create(sizeof(uint32_t)); 
    buffer_add_uint32(buffer, posicionDeCodigo);
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = ENVIO_PID_PC;
    paquete->buffer = buffer;

    void* a_enviar = crear_a_enviar(paquete);
    int result = send(socket, a_enviar, buffer->size + sizeof(int) + sizeof(uint32_t), 0);
    return result;
}

char* recibir_instruccion(int socket){
    t_buffer* buffer = recibir_buffer(socket);

    uint32_t* size_instruccion;
    char* instruccion = buffer_read_string(buffer, size_instruccion);
    return instruccion;
}

char* pedir_instruccion(int socket, t_PCB* pcb, t_log* logger){
    t_paquete_instruccion* paquete_instruccion = crear_paquete_instruccion(pcb->PID, pcb->registrosCPU.pc);

    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = ENVIO_PID_PC;
    paquete->buffer = serializar_paquete_instruccion(paquete_instruccion);

    void* a_enviar = crear_a_enviar(paquete);

    send(socket, a_enviar, paquete->buffer->size + sizeof(int) + sizeof(uint32_t), 0);

    return recibir_instruccion(socket);
}