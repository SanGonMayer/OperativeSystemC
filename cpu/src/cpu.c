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

    int result = serializar_y_enviar_paquete(paquete, socket);
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

void ciclo_de_ejecucion (int socket, t_PCB* pcb, t_log* logger){
    char* instruccion;

    instruccion = etapa_fetch(socket, pcb, logger);

    while (instruccion != NULL) {
        //etapa decode
        char** instruccion_separada = string_split(instruccion, " ");
        switch (instruccion_separada[0])
        {
        case "SET":
            char* registro = instruccion_separada[1];
            int valor = atoi(instruccion_separada[2]);
            //etapa execute
            ejecutar_set(registro, valor, pcb);
            break;
        case "SUM":
            char* registroDestino = instruccion_separada[1];
            char* registroValor = instruccion_separada[2];
            //etapa execute
            ejecutar_sum(registroDestino, registroValor, pcb);
            break;
        case "SUB":
            char* registroDestino = instruccion_separada[1];
            char* registroValor = instruccion_separada[2];
            //etapa execute
            ejecutar_sub(registroDestino, registroValor, pcb);
            break;
        case "JNZ":
            char* registro = instruccion_separada[1];
            int valorPC = atoi(instruccion_separada[2]);
            //etapa execute
            ejecutar_jnz(registro, valorPC, pcb);
            break;
        case "IO_GEN_SLEEP":
            char* dispositivo = instruccion_separada[1];
            int unidadesDeTrabajo = atoi(instruccion_separada[2]);
            //TODO
            break;
        case "EXIT":
            //TODO
            break;
        default:
            break;
        }
        check_interrupt();
        instruccion = etapa_fetch(socket, pcb, logger);
    }
}

void ejecutar_set(char* registro, int valor, t_PCB* pcb){
    dictionary_put(pcb->registrosCPU, registro, valor);
}

void ejecutar_sum(char* registroDestino, char* registroValor, t_PCB* pcb){
    //Chequear tipos de dato
    uint32_t* valorASumar = dictionary_get(pcb->registrosCPU, registroValor);
    uint32_t* valorDestino = dictionary_get(pcb->registrosCPU, registroDestino);
    uint32_t suma = *valorASumar + *valorDestino;
    dictionary_put(pcb->registrosCPU, registroDestino, &suma);

    free(valorASumar);
    free(valorDestino);
}

void ejecutar_sub(char* registroDestino, char* registroValor, t_PCB* pcb){
    //Chequear tipos de dato
    uint32_t* valorARestar = dictionary_get(pcb->registrosCPU, registroValor);
    uint32_t* valorDestino = dictionary_get(pcb->registrosCPU, registroDestino);
    uint32_t resta = *valorDestino - *valorARestar;
    dictionary_put(pcb->registrosCPU, registroDestino, &resta);

    free(valorARestar);
    free(valorDestino);
}

void ejecutar_jnz(char* registro, int valorPC, t_PCB* pcb){
    uint32_t* valor = dictionary_get(pcb->registrosCPU, registro);
    if (*valor != 0){
        dictionary_put(pcb->registrosCPU, "PC", valorPC);
    }
}