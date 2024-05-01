#include "procesos.h"

t_PCB* crear_PCB(){
    t_PCB* pcb = malloc(sizeof(t_PCB));
    return pcb;
}

void serializar_pcb(const t_PCB *pcb, uint8_t *buffer, size_t *size_out) {
    // Serializa todos los campos excepto el path
    size_t base_size = sizeof(t_PCB) - sizeof(char *);
    memcpy(buffer, pcb, base_size);

    // Serializa el path
    uint32_t path_len = strlen(pcb->path) + 1; // Incluye el terminador nulo
    memcpy(buffer + base_size, &path_len, sizeof(path_len)); // Longitud del path
    memcpy(buffer + base_size + sizeof(path_len), pcb->path, path_len); // Contenido del path
    
    // Establece el tamaño total del buffer
    *size_out = base_size + sizeof(path_len) + path_len;
}

int enviar_pcb(int socket, const t_PCB *pcb) {
    // Calcular el tamaño total necesario para el buffer
    size_t buffer_size;
    uint8_t *buffer = (uint8_t *)malloc(sizeof(t_PCB) + 256); // Buffer de tamaño máximo probable

    serializar_pcb(pcb, buffer, &buffer_size);

    int result = send(socket, buffer, buffer_size, 0);

    free(buffer); // Liberar la memoria del buffer

    return result;
}

void deserializar_pcb(const uint8_t *buffer, t_PCB *pcb) {
    // Deserializa todos los campos excepto el path
    size_t base_size = sizeof(t_PCB) - sizeof(char *);
    memcpy(pcb, buffer, base_size);

    // Deserializa el path
    uint32_t path_len;
    memcpy(&path_len, buffer + base_size, sizeof(path_len)); // Obtiene la longitud del path
    pcb->path = (char *)malloc(path_len);
    memcpy(pcb->path, buffer + base_size + sizeof(path_len), path_len); // Copia el contenido del path
}

int recibir_pcb(int socket, t_PCB *pcb) {
    uint8_t temp_buffer[1024]; // Buffer temporal para recibir datos
    size_t received = 0;

    int bytes = recv(socket, temp_buffer, sizeof(temp_buffer), 0);
    if (bytes > 0) {
        received += bytes;
        deserializar_pcb(temp_buffer, pcb);
    }

    return bytes;
}

void actualizar_pcb(t_PCB *pcb_viejo, const t_PCB *pcb_nuevo) {
    // Copiar datos básicos
    memcpy(&pcb_viejo->registrosCPU, &pcb_nuevo->registrosCPU, sizeof(t_registrosCPU));
    memcpy(&pcb_viejo->registrosMem, &pcb_nuevo->registrosMem, sizeof(t_registrosMem));
    pcb_viejo->PID = pcb_nuevo->PID;
    pcb_viejo->estado = pcb_nuevo->estado;
    pcb_viejo->quantum = pcb_nuevo->quantum;

    // Actualizar el path
    /*if (pcb_viejo->path != NULL) {
        free(*(pcb_viejo->path)); // Liberar el path antiguo
    
    }
    */
    strcpy(pcb_nuevo->path, pcb_viejo->path);
    /*
    uint32_t path_len = strlen(pcb_nuevo->path) + 1;
    pcb_viejo->path = (char *)malloc(path_len); // Asignar nueva memoria para el path
    if (pcb_viejo->path != NULL) {
        memcpy(pcb_viejo->path, pcb_nuevo->path, path_len); // Copiar el nuevo path
    }
    */
}