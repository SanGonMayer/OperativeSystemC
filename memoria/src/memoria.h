#ifndef MEMORIA_H_
#define MEMORIA_H_
#include <utils/procesos.h>
#include <utils/buffer.h>
#include <commons/log.h>

typedef struct{
    uint32_t PID;
    uint32_t path_length;
    char* path;
}t_paqueteMemoria;

t_paqueteMemoria* inicializar_paquete_memoria();

void recibir_contexto_de_kernel(int socket, t_paqueteMemoria* paqueteMemoria);

void enviar_posicion_de_codigo(int socket, uint32_t posicionDeCogido);

uint32_t recibir_posicin_de_codigo(int socket, t_log* logger);

void enviar_instruccion(*socket, char* instruccion, t_log* logger);

void leer_archivo(uint32_t posicionDeCodigo, char* instruccion);


#endif