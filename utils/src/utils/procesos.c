#include "procesos.h"

t_PCB* crear_PCB(){
    t_PCB* pcb = malloc(sizeof(t_PCB));
    return pcb;
}

void* crear_a_enviar(t_paquete* paquete){
    void* a_enviar = malloc(paquete->buffer->size + sizeof(int) + sizeof(uint32_t));
    int offset = 0;
    memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(int));
    offset += sizeof(int);
    memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);
    return a_enviar;
}

int enviar_pcb(int socket, const t_PCB *pcb) {

    t_buffer* buffer = serializar_pcb(pcb);

    t_paquete* paquete = malloc(sizeof(t_paquete));

    paquete->codigo_operacion = 2;
    paquete->buffer = buffer; 

    void* a_enviar = crear_a_enviar(paquete);

    // Por último enviamos
    int result = send(socket, a_enviar, buffer->size + sizeof(int) + sizeof(uint32_t), 0);

    // No nos olvidamos de liberar la memoria que ya no usaremos
    free(a_enviar);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);

    return result;
}

void actualizar_pcb(t_PCB *pcb_viejo, const t_PCB *pcb_nuevo) {
    // Copiar datos básicos
    memcpy(&pcb_viejo->registrosCPU, &pcb_nuevo->registrosCPU, sizeof(t_registrosCPU));
    memcpy(&pcb_viejo->registrosMem, &pcb_nuevo->registrosMem, sizeof(t_registrosMem));
    pcb_viejo->PID = pcb_nuevo->PID;
    pcb_viejo->estado = pcb_nuevo->estado;
    pcb_viejo->quantum = pcb_nuevo->quantum;

    strcpy(pcb_nuevo->path, pcb_viejo->path);
}

t_buffer* serializar_pcb(t_PCB* pcb){

    t_buffer* buffer = buffer_create(
        sizeof(pcb->PID) + 
        sizeof(pcb->estado) + 
        sizeof(pcb->quantum) + 
        sizeof(pcb->registrosCPU) + 
        sizeof(pcb->registrosMem) + 
        sizeof(pcb->path_length)
    );

    buffer_add_uint32(buffer, pcb->PID);
    buffer_add_uint32(buffer, pcb->estado);
    buffer_add_uint32(buffer, pcb->quantum);
    buffer_add(buffer, &pcb->registrosCPU, sizeof(t_registrosCPU));
    buffer_add(buffer, &pcb->registrosMem, sizeof(t_registrosMem));
    buffer_add_string(buffer, pcb->path_length, pcb->path);

    return buffer;
}


t_PCB* recibir_pcb(int socket){
    t_buffer* buffer = recibir_buffer(socket);
    t_PCB* pcb = deserializar_pcb(buffer);
    free(buffer);
    return pcb;
}

t_PCB* deserializar_pcb(t_buffer* buffer){
    t_PCB* pcb = malloc(sizeof(t_PCB));

    pcb->PID = buffer_read_uint32(buffer);
    pcb->estado = buffer_read_uint32(buffer);
    pcb->quantum = buffer_read_uint32(buffer);
    buffer_read(buffer, &pcb->registrosCPU, sizeof(t_registrosCPU));
    buffer_read(buffer, &pcb->registrosMem, sizeof(t_registrosMem));
    pcb->path = buffer_read_string(buffer, &pcb->path_length);

    return pcb;
}
