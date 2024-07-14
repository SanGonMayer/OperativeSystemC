#include "procesos.h"
#include "utils/buffer.h"
#include "utils/client.h"
#include "utils/codigo_operacion.h"

t_PCB* crear_PCB(){
    t_PCB* pcb = malloc(sizeof(t_PCB));

    *pcb = (t_PCB) {
        .PID = 0,
        .estado = NEW,
        .quantum = 0,
        .registrosCPU = {
            .pc = 0,
            .ax = 0,
            .bx = 0,
            .cx = 0,
            .dx = 0,
            .eax = 0,
            .ebx = 0,
            .ecx = 0,
            .edx = 0,
            .si = 0,
            .di = 0,
        },
        .registrosMem = {
            .codigo = 0,
            .datos = 0,
            .heap = 0,
            .posicionFinal = 0,
        },
        .path_length = 0,
        .path = NULL,
    };

    return pcb;
}


void responder_pcb(int socket, t_PCB *pcb, t_log* logger) {
    t_buffer* buffer = serializar_pcb(pcb);

    enviar_buffer(socket, buffer, logger);

    buffer_destroy(buffer);
}

int enviar_pcb(int socket, t_PCB *pcb) {

    t_buffer* buffer = serializar_pcb(pcb);
    t_paquete* paquete = crear_paquete(ENVIO_PCB, buffer);
    int result = enviar_paquete(paquete, socket);

    eliminar_paquete(paquete);
    
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
        sizeof(pcb->readyplus) + 
        sizeof(pcb->registrosCPU) + 
        sizeof(pcb->registrosMem) + 
        sizeof(pcb->path_length) +
        pcb->path_length
    );

    buffer_add_uint32(buffer, pcb->PID);
    buffer_add_uint32(buffer, pcb->estado);
    buffer_add_int(buffer, pcb->quantum);
    buffer_add_int(buffer, pcb->readyplus);
    buffer_add(buffer, &pcb->registrosCPU, sizeof(t_registrosCPU));
    buffer_add(buffer, &pcb->registrosMem, sizeof(t_registrosMem));
    buffer_add_string(buffer, pcb->path_length, pcb->path);

    return buffer;
}


t_PCB* recibir_pcb(int socket){
    t_buffer* buffer = recibir_buffer(socket);
    t_PCB* pcb = deserializar_pcb(buffer);
    
    buffer_destroy(buffer);

    return pcb;
}

t_PCB* deserializar_pcb(t_buffer* buffer){
    t_PCB* pcb = malloc(sizeof(t_PCB));

    pcb->PID = buffer_read_uint32(buffer);
    pcb->estado = buffer_read_uint32(buffer);
    pcb->quantum = buffer_read_int(buffer);
    pcb->readyplus = buffer_read_int(buffer);
    buffer_read(buffer, &pcb->registrosCPU, sizeof(t_registrosCPU));
    buffer_read(buffer, &pcb->registrosMem, sizeof(t_registrosMem));
    pcb->path = buffer_read_string(buffer, &pcb->path_length);

    return pcb;
}

void destroy_pcb(t_PCB* pcb){
    free(pcb->path);
    free(pcb);
}
