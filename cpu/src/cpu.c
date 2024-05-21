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

uint32_t recibir_interrupcion(int socket){
    t_buffer* buffer = recibir_buffer(socket);

    uint32_t interrupcion = buffer_read_uint32(buffer);
    return interrupcion;
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

void ejecutar_set(char* registro, int valor, t_PCB* pcb, t_dictionary* diccionario){
    dictionary_put(diccionario, registro, &valor);
}

void ejecutar_sum(char* registroDestino, char* registroValor, t_PCB* pcb, t_dictionary* diccionario){
    //Chequear tipos de dato
    uint32_t* valorASumar = dictionary_get(diccionario, registroValor);
    uint32_t* valorDestino = dictionary_get(diccionario, registroDestino);
    uint32_t suma = *valorASumar + *valorDestino;
    dictionary_put(diccionario, registroDestino, &suma);

    free(valorASumar);
    free(valorDestino);
}

void ejecutar_sub(char* registroDestino, char* registroValor, t_PCB* pcb, t_dictionary* diccionario){
    //Chequear tipos de dato
    uint32_t* valorARestar = dictionary_get(diccionario, registroValor);
    uint32_t* valorDestino = dictionary_get(diccionario, registroDestino);
    uint32_t resta = *valorDestino - *valorARestar;
    dictionary_put(diccionario, registroDestino, &resta);

    free(valorARestar);
    free(valorDestino);
}

void ejecutar_jnz(char* registro, int valorPC, t_PCB* pcb, t_dictionary* diccionario){
    uint32_t* valor = dictionary_get(diccionario, registro);
    if (*valor != 0){
        dictionary_put(diccionario, "PC", &valorPC);
    }
}

t_buffer* ejecutar_io_gen_sleep(char* dispositivo, int unidadesDeTrabajo){
    uint32_t length = strlen(dispositivo) + 1;
    t_buffer* buffer = buffer_create(sizeof(int) + sizeof(uint32_t) + length);
    buffer_add_int(buffer, unidadesDeTrabajo);
    buffer_add_string(buffer, length, dispositivo);
    return buffer;
}

void desalojar_pcb(int socket_dispatch, t_PCB* pcb, int motivo, t_log* logger, t_dictionary* diccionario){
    pcb->registrosCPU = registros_cpu_from_dictionary(diccionario);
    enviar_pcb(socket_dispatch, pcb);
    send(socket_dispatch, &motivo, sizeof(int), 0);
}

void registros_cpu_dictionary(t_registrosCPU* registros, t_dictionary* dictionary){
    dictionary_put(dictionary, "AX", &registros->ax);
    dictionary_put(dictionary, "BX", &registros->bx);
    dictionary_put(dictionary, "CX", &registros->cx);
    dictionary_put(dictionary, "DI", &registros->di);
    dictionary_put(dictionary, "SI", &registros->si);
    dictionary_put(dictionary, "PC", &registros->pc);
    dictionary_put(dictionary, "EAX", &registros->eax);
    dictionary_put(dictionary, "EBX", &registros->ebx);
    dictionary_put(dictionary, "ECX", &registros->ecx);
    dictionary_put(dictionary, "EDX", &registros->edx);
}

t_registrosCPU registros_cpu_from_dictionary(t_dictionary* dictionary){
    t_registrosCPU registros;

    registros.ax = *(uint8_t*)dictionary_get(dictionary, "AX");
    registros.bx = *(uint8_t*)dictionary_get(dictionary, "BX");
    registros.cx = *(uint8_t*)dictionary_get(dictionary, "CX");
    registros.di = *(uint32_t*)dictionary_get(dictionary, "DI");
    registros.si = *(uint32_t*)dictionary_get(dictionary, "SI");
    registros.pc = *(uint32_t*)dictionary_get(dictionary, "PC");
    registros.eax = *(uint32_t*)dictionary_get(dictionary, "EAX");
    registros.ebx = *(uint32_t*)dictionary_get(dictionary, "EBX");
    registros.ecx = *(uint32_t*)dictionary_get(dictionary, "ECX");
    registros.edx = *(uint32_t*)dictionary_get(dictionary, "EDX");

    return registros;
}