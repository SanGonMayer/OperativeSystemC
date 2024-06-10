#ifndef CPU_H_
#define CPU_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <commons/config.h>
#include <utils/server.h>
#include <utils/client.h>
#include <utils/procesos.h>
#include <pthread.h>
#include <utils/codigo_operacion.h>
#include <commons/string.h>
#include <semaphore.h>
#include "global_cpu.h"
#include <utils/peticiones_memoria.h>
#include <utils/instrucciones.h>
#include <utils/buffer.h>
#include "mmu.h"
#include "tlb.h"
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>

/**
* @fn    etapa_fetch
* @brief pide la instruccion a partir de una posicion de memoria, devuelve instruccion
*/

char* etapa_fetch(int socket, t_PCB* pcb, t_log* logger, t_dictionary* diccionario);

char* recibir_instruccion(int socket);

char* pedir_instruccion(int socket, t_PCB* pcb, t_log* logger, t_dictionary* diccionario);

uint32_t recibir_interrupcion(int socket);

void ejecutar_set(char* registro, int valor, t_PCB* pcb, t_dictionary* diccionario);

void ejecutar_sum(char* registroDestino, char* registroValor, t_PCB* pcb, t_dictionary* diccionario);

void ejecutar_sub(char* registroDestino, char* registroValor, t_PCB* pcb, t_dictionary* diccionario);

void ejecutar_jnz(char* registro, int valorPC, t_PCB* pcb, t_dictionary* diccionario);

t_buffer* ejecutar_io_gen_sleep(char* dispositivo, int unidadesDeTrabajo);

void desalojar_pcb(int socket_dispatch, t_PCB* pcb, int motivo, t_log* logger, t_dictionary* diccionario);

void registros_cpu_dictionary(t_registrosCPU registros, t_dictionary* dictionary);

t_registrosCPU registros_cpu_from_dictionary(t_dictionary* dictionary);

t_buffer* ejecutar_io_stdin_read(char* dispositivo, int registro_direccion, int registro_tamanio);

t_buffer* ejecutar_io_stdout_write(char* dispositivo, int registro_direccion, int registro_tamanio);

void ejecutar_mov_in(uint32_t pid, char* registro_datos, int direccion_logica, t_dictionary* diccionario);

void ejecutar_mov_out(uint32_t pid,int direccion_logica, uint32_t valor,t_dictionary* diccionario);

void ejecutar_copy_string();

int obtener_direccion_fisica(int pid, int direccion_logica);

#endif