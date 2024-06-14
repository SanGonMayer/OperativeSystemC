#include "cpu.h"
#include "mmu.h"
#include "tlb.h"
#include "utils/buffer.h"
#include "utils/client.h"
#include "utils/codigo_operacion.h"
#include "utils/instrucciones.h"
#include <stdint.h>

char* etapa_fetch(int socket, t_PCB* pcb, t_log* logger, t_dictionary* diccionario){
    char* instruccion;
    //Envio posicion de memoria + program counter

    instruccion = pedir_instruccion(socket, pcb, logger, diccionario);
    
    //Sumar program counter
    uint32_t valorPCanterior = dictionary_get(diccionario, "PC");
    dictionary_put(diccionario, "PC", valorPCanterior + 1);
    return instruccion;
}



char* recibir_instruccion(int socket){
    t_buffer* buffer = recibir_buffer(socket);

    uint32_t size_instruccion;
    char* instruccion = buffer_read_string(buffer, &size_instruccion);

    buffer_destroy(buffer);
    return instruccion;
}

uint32_t recibir_interrupcion(int socket){
    t_buffer* buffer = recibir_buffer(socket);

    uint32_t interrupcion = buffer_read_uint32(buffer);
    return interrupcion;
}

char* pedir_instruccion(int socket, t_PCB* pcb, t_log* logger, t_dictionary* diccionario){
    log_info(logger, "Pidiendo instruccion a memoria");
    t_paquete_instruccion* paquete_instruccion = crear_paquete_instruccion(pcb->PID, dictionary_get(diccionario, "PC"));
    
    t_paquete* paquete = crear_paquete(ENVIO_PID_PC, serializar_paquete_instruccion(paquete_instruccion));
    int err = enviar_paquete(paquete, socket);
    
    if(err < 0){
        log_error(logger, "Error al enviar paquete");
        return NULL;
    }

    eliminar_paquete(paquete);

    return recibir_instruccion(socket);
}

void ejecutar_set(char* registro, int valor, t_PCB* pcb, t_dictionary* diccionario){
    dictionary_put(diccionario, registro, valor);
}

void ejecutar_sum(char* registroDestino, char* registroValor, t_PCB* pcb, t_dictionary* diccionario){
    //Chequear tipos de dato
    uint32_t valorASumar = dictionary_get(diccionario, registroValor);
    uint32_t valorDestino = dictionary_get(diccionario, registroDestino);
    uint32_t suma = valorASumar + valorDestino;
    dictionary_put(diccionario, registroDestino, suma);
}

void ejecutar_sub(char* registroDestino, char* registroValor, t_PCB* pcb, t_dictionary* diccionario){
    //Chequear tipos de dato
    uint32_t valorARestar = dictionary_get(diccionario, registroValor);
    uint32_t valorDestino = dictionary_get(diccionario, registroDestino);
    uint32_t resta = valorDestino - valorARestar;
    dictionary_put(diccionario, registroDestino, resta);

    // free(valorARestar);
    // free(valorDestino);
}

void ejecutar_jnz(char* registro, int valorPC, t_PCB* pcb, t_dictionary* diccionario){
    uint32_t valor = dictionary_get(diccionario, registro);
    if (valor != 0){
        dictionary_put(diccionario, "PC", valorPC);
    }
}

t_buffer* ejecutar_io_gen_sleep(char* dispositivo, int unidadesDeTrabajo){
    uint32_t length = strlen(dispositivo) + 1;
    t_buffer* buffer = buffer_create(sizeof(int) + sizeof(uint32_t) + length);
    buffer_add_int(buffer, unidadesDeTrabajo);
    buffer_add_string(buffer, length, dispositivo);
    return buffer;
}

t_buffer* ejecutar_io_stdin_read(char* dispositivo, int direccion_fisica, int registro_tamanio){
    uint32_t length = strlen(dispositivo) + 1;
    t_buffer* buffer = buffer_create(sizeof(int) + sizeof(int) + sizeof(uint32_t) + length);
    buffer_add_int(buffer, direccion_fisica); //Direccion fisica
    buffer_add_int(buffer, registro_tamanio);
    buffer_add_string(buffer, length, dispositivo);
    return buffer;
}

t_buffer* ejecutar_io_stdout_write(char* dispositivo, int direccion_fisica, int registro_tamanio){
    uint32_t length = strlen(dispositivo) + 1;
    t_buffer* buffer = buffer_create(sizeof(int) + sizeof(int) + sizeof(uint32_t) + length);
    buffer_add_int(buffer, direccion_fisica); //Direccion fisica
    buffer_add_int(buffer, registro_tamanio);
    buffer_add_string(buffer, length, dispositivo);
    return buffer;
}

void ejecutar_mov_in(direccion_fisica_datos, direccion_fisica_direccion){}

void ejecutar_mov_out(direccion_fisica_datos, direccion_fisica_direccion){}

void ejecutar_copy_string(tamanio){}


void desalojar_pcb(int socket_dispatch, t_PCB* pcb, int motivo, t_log* logger, t_dictionary* diccionario){
    pcb->registrosCPU = registros_cpu_from_dictionary(diccionario);
    responder_pcb(socket_dispatch, pcb, logger);
    send(socket_dispatch, &motivo, sizeof(int), 0);
}

void registros_cpu_dictionary(t_registrosCPU registros, t_dictionary* dictionary){
    dictionary_put(dictionary, "AX", (registros.ax));
    dictionary_put(dictionary, "BX", (registros.bx));
    dictionary_put(dictionary, "CX", (registros.cx));
    dictionary_put(dictionary, "DI", (registros.di));
    dictionary_put(dictionary, "SI", (registros.si));
    dictionary_put(dictionary, "PC", (registros.pc));
    dictionary_put(dictionary, "EAX", (registros.eax));
    dictionary_put(dictionary, "EBX", (registros.ebx));
    dictionary_put(dictionary, "ECX", (registros.ecx));
    dictionary_put(dictionary, "EDX", (registros.edx));
}

t_registrosCPU registros_cpu_from_dictionary(t_dictionary* dictionary){
    t_registrosCPU registros = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    registros.ax = dictionary_get(dictionary, "AX");
    registros.bx = dictionary_get(dictionary, "BX");
    registros.cx = dictionary_get(dictionary, "CX");
    registros.di = dictionary_get(dictionary, "DI");
    registros.si = dictionary_get(dictionary, "SI");
    registros.pc = dictionary_get(dictionary, "PC");
    registros.eax = dictionary_get(dictionary, "EAX");
    registros.ebx = dictionary_get(dictionary, "EBX");
    registros.ecx = dictionary_get(dictionary, "ECX");
    registros.edx = dictionary_get(dictionary, "EDX");

    return registros;
}


int obtener_direccion_fisica(int pid, int direccion_logica){
    bool tlb_hit = false;
    int direccion_fisica;

    if(tlb_enabled())
        tlb_hit = tlb_get_marco(pid, direccion_logica, &direccion_fisica);

    if(!tlb_hit){
        tlb_hit = traducir_a_direccion_fisica(pid, direccion_logica);

        if(tlb_enabled())
            tlb_add(pid, direccion_logica, direccion_fisica);
    }

    return direccion_fisica;
}