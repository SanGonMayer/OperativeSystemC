#ifndef MEMORIA_H_
#define MEMORIA_H_
#include <utils/procesos.h>
#include <utils/buffer.h>
#include <commons/log.h>

typedef struct{
    uint32_t PID;
    uint32_t path_length;
    char* path;
}t_paqueteMemoria

void inicializar_paquete_memoria();

void recibir_contexto_de_kernel(int socket, t_paqueteMemoria* paqueteMemoria);

void enviar_posicion_de_codigo(int socket, int posicionDeCogido);
#endif